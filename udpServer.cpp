#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <time.h>
#include <ctime>
#include <sys/time.h>
#include <csignal>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close() */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <iostream>
#include <getopt.h>
#define MAX_MSG 1500
#define MAX_PDU	100000

#include "udpgen.h"
#include "sample.h"

int LOCAL_SERVER_PORT= 1500;
int SERVER_TIMEOUT=10;
void alarmHandler(int signo);
int timeout;
int timeout_value;
SAMPLE sample_iptr,sample_ipgr,sample_ipts,sample_ipgs;



/*
struct pduinfo{
  char* exp_id;
  u_int32_t seq_no;
  u_int64_t send_start;
  u_int64_t send_stop;
  u_int64_t recv_start;
  u_int64_t recv_stop;
  u_int32_t length; //size of pdu!
  u_int64_t ipts;
  u_int64_t ipgs;
  //u_int64_t iptr;
  //u_int64_t ipgr;
};
*/

struct pdudata{
  u_int32_t seq_no;
  u_int64_t send_start;
  u_int64_t send_stop;
  u_int64_t recv_start;
  u_int64_t recv_stop;
  timeval send_dept_time;
  timeval recv_arrival_time;
//  u_int32_t length;
};

void Sample(int sig);
void output_file(u_int32_t, u_int32_t,struct pdudata[],int, double);
//struct pduinfo recvpdu[MAX_PDU];
struct pdudata logdat[MAX_PDU];
int pducount=0;

double byteCount,pktCount;
double dT;

itimerval diftime;

timeval tidA,tidB,theTime;
itimerval sampletime,sampletimeOff;

timeval tid1,tid2,tidf,tid3,tidOff;
int  counter;
u_int32_t msgcounter;

static inline u_int64_t realcc(void){
  u_int64_t cc;
  asm volatile("rdtsc":"=&A"(cc));
  return cc;
}

double estimateCPU(int samples, int sleeptime, char* fname);
double samplefreq;
int runAsDaemon;
char *logpath;

u_int64_t sumipts=0,sumipgs=0,sumiptr=0,sumipgr=0;  /*to keep the previous pkt recv cpu counter*/
int pcounter,m,js=0,jr=0; /*previou pkt sequence number*/
int cond;
int main(int argc, char *argv[])
{
  int option_index, op,reqFlag=0;
  u_int32_t exp_id,run_id,key_id;
  exp_id=run_id=key_id=0;
  runAsDaemon=0;
  logpath=".";

  double CPU_before, CPU_after;
  struct timeval GTOD_before, GTOD_after, PktArr;
  u_int64_t TSC_before,TSC_after;
  double samplefreq=0;
  byteCount=pktCount=0;
  int hflag=0;
  int wildcard=1;
  int loglevel=0;

  static struct option long_options[]={
    {"experiment_id", required_argument , 0,'e'},
    {"run_id", required_argument , 0,'r'},
    {"key_id", required_argument , 0,'k'},
    {"port_no", required_argument, 0, 'p'},
    {"freq", required_argument, 0, 'f'},
    {"log", required_argument, 0, 'L'},
    {"logpath", required_argument, 0, 'l'},
    {"timeout", required_argument, 0, 't'},
    {"wildcard", no_argument, 0, 'w'},    
    {"daemon", no_argument, 0, 'd'},    
    {0,0,0,0}
    
  };
  if (argc<2)    {
    printf("Please enter the experiment id (-e exp_id string)");
    exit(EXIT_FAILURE);
    // LOCAL_SERVER_PORT=atoi(argv[1]);
  }
  
  for(int i=0;i<MAX_PDU;i++){
    logdat[i].send_start=0;
    logdat[i].send_stop=0;
    logdat[i].recv_start=0;
    logdat[i].recv_stop=0;
  }
  dT=0;
  while ( (op =getopt_long(argc, argv, "t:f:k:e:r:p:hdL:l:w",long_options, &option_index))!=EOF) {
    switch (op){
    case 'e': /* experiment_id */
      exp_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      wildcard=0;
      break;
    case 'r': /* experiment_id */
      run_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      wildcard=0;
      break;
    case 'k': /* experiment_id */
      key_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      wildcard=0;
      break;
    case 'p':/*port*/
      LOCAL_SERVER_PORT=atoi(optarg);
      break;
    case 'f':/*sampleFreq*/
      samplefreq=atof(optarg);
      dT=1/(double)samplefreq;
      break;
    case 'w': 
      wildcard=1;
      break;
    case 'd':
      runAsDaemon=1;
      wildcard=1;
      break;
    case 't':
      SERVER_TIMEOUT=atoi(optarg);
      break;
      
   case 'L':/*Log Level*/
      loglevel=atoi(optarg);
      if(loglevel<0){
	loglevel=0;
      }
      if(loglevel>3){
	loglevel=3;
      }
      break;
      
    case 'l': /* Log Path */
      logpath=strdup(optarg);
      break;

    case 'h': /* Help */
      printf("UDP server. v6\n");
      printf("-d || --daemon                    If enabled; the server will continue to serve new experiments. \n");
      printf("                                  Timeout is %d s, and -w is set.\n",SERVER_TIMEOUT);
      printf("-t || --timeout                   Timeout value [optional]\n");
      printf("-w || --wildcard                  If used, it will NOT accept the first packet recieve \n");
      printf("                                  and use the data found in it as expid,runid, keyid. \n");
      printf("-p || --port_no 	     <port>	Port to listen on, default %d\n", LOCAL_SERVER_PORT);

      printf("-e || --experiment_id  <id>       Experiment id [optional]\n");
      printf("-r || --run_id	     <runid>	Run id [optional]\n");
      printf("-k || --key_id	     <keyid>	Key id [optional]\n");

      printf("-f || --freq    	     <freq>	Sample frequency, default 0 -- No sampling.\n");
      printf("                                  Print status messages periodically.\n");

      printf("-h   help \n");
      printf("-L  --log              <level>    Logs recived data and CPU info to file\n");
      printf("                                  Logs the data into a directory <experiment_id> that must\n");
      printf("                                  exist, save the data as \n");
      printf("                                  <run_id>_server.txt\n");
      printf("                                  <run_id>_recv_cpueval.txt\n");
      printf("                                  The sender <run_id>_send_cpueva.txt must be collected \n");
      printf("                                  from the sender after experiments completed.\n");
      printf("                                  1-- , 2-- , 3-- complete .\n");
      printf("-l || --logpath <path>            Complete path where to store log, otherwise its logged where executed. \n");
      printf("\n");
      printf("MAX PDUs; %d MAX MSG length %d bytes .\n",MAX_PDU,MAX_MSG);
      hflag=1;
      break;
    default:
      hflag=1;
      printf("Error....\n");
      break;
    }
  }


  if (hflag) exit(EXIT_SUCCESS);
  if( (wildcard==0) && (reqFlag<3) ) {
    printf("You are missing required info. %d \n", reqFlag);
    exit(EXIT_FAILURE);
  }
  char fname_cpu[200];
  char fpath[200];
  int mkdir_status;
  bzero(&fname_cpu,200);
  /*
  strcat(fname_cpu, exp_id);
  strcat(fname_cpu, "/");
  strcat(fname_cpu, run_id);
  strcat(fname_cpu, "_recv_cpueval.txt");
  */

  printf("MAX PDUs; %d MAX MSG length %d bytes .\n",MAX_PDU,MAX_MSG);
  if(runAsDaemon){
    printf("[Daemon] ");
  }
  printf("[%s]  ",logpath);

  if(wildcard){
    printf("[Wildcard] ");
  } else {
    printf("[Expid=%d, Runid=%d, Keyid=%d] ",exp_id,run_id,key_id);
  }

  printf("[level= %d] \n",loglevel);
  
  sprintf(fname_cpu,"%s/%d/recv_cpueval-%d-%d-%d.txt",logpath,exp_id,exp_id,run_id,key_id);
  sprintf(fpath,"%s/%d",logpath,exp_id);
  if(loglevel>1){
    printf("Writes cpu data to %s.\n", fname_cpu);
  }
  if(wildcard==0 && loglevel>1){ // This is the ONLY case when its applicable, we know the exp,run and key id.
    mkdir_status=mkdir(fpath,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    printf("Result of mkdir %d, for mkdir(%s).\n",mkdir_status,fpath);
    CPU_before=estimateCPU(40,10000,fname_cpu);
  } else {
    CPU_before=estimateCPU(40,10000,0);
  }
  TSC_before=realcc();
  gettimeofday(&GTOD_before,NULL);
  
  printf("Estimated cpu to %f Hz.\n",CPU_before);
  printf("Sampling interval %f s.\n",dT);
  u_int64_t rstart,rstop;/*rstart mean reciving start time*/
  int sd, rc, n, cliLen;
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];
  /* socket creation */
  sd=socket(AF_INET, SOCK_DGRAM, 0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }
  
  /* bind local server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  servAddr.sin_port = htons(LOCAL_SERVER_PORT);
  timeout=0;
  timeout_value=2;
  rc = bind (sd, (struct sockaddr *) &servAddr,sizeof(servAddr));
  if(rc<0) {
    printf("%s: cannot bind port number %d \n",argv[0], LOCAL_SERVER_PORT);
    exit(1);
  }
  
  printf("%s: waiting for data on port UDP %u\n",argv[0],LOCAL_SERVER_PORT);
  
  rstart=0;
  rstop=0;
  /* server infinite loop */
  transfer_data *message; 
  counter=0;
  tid1.tv_sec=30;
  diftime.it_interval=tid1;
  diftime.it_value=tid1;
  counter=-1;
  
  //    setitimer(ITIMER_REAL,&diftime,NULL);
  
  struct tm *file_stime;
  gettimeofday(&tidf,NULL);
  char file_name[20];
  file_name[19]='\0';
  file_stime=localtime(&tidf.tv_sec);
  strftime(file_name,20,"%Y-%m-%d %H.%M.%S",file_stime);
  printf("File Started at %s\n", file_name);


  m=0; /*for pduinfo array*/
  int cond=1;
  struct timeval accept_timeout;
  fd_set rset;

  int selectReturn=0;
  
  /*
  char expidrunid[16];
  strcpy(expidrunid,exp_id);
  strcat(expidrunid,run_id);
  */
  int Acounter=0;
  int charErr=0;
  
  if(dT>=1){
    tidA.tv_sec=int(dT);
    tidB.tv_sec=int(dT);
    tidA.tv_usec=int((dT-int(dT))*1000000);
    tidB.tv_usec=tidA.tv_usec;
  } else {
    tidA.tv_sec=0;
    tidB.tv_sec=0;
    tidA.tv_usec=int(dT*1000000);
    tidB.tv_usec=tidA.tv_usec;
  }
  
  tidOff.tv_sec=0;
  tidOff.tv_usec=0;

  sampletime.it_interval=tid1;
  sampletime.it_value=tid2;

  sampletimeOff.it_interval=tidOff;
  sampletimeOff.it_value=tidOff;
  
  pktCount=0;
  byteCount=0;



  
  while(cond==1){
    /* init buffer */
    memset(msg,0x0,MAX_MSG);
    
    /* receive message */
    cliLen = sizeof(cliAddr);
    
    FD_ZERO(&rset);
    FD_SET(sd, &rset);
    accept_timeout.tv_sec=SERVER_TIMEOUT;
    accept_timeout.tv_usec=0;
    
    rstart=realcc();
    selectReturn=select(sd+1,&rset,NULL,NULL,&accept_timeout);
    if(selectReturn==-1){
      perror("Select Error:\n");
    } else if(selectReturn==0){
      printf("!TIMEOUT!\nSocket was idle for %d seconds.\n", SERVER_TIMEOUT);//(int)accept_timeout.tv_sec);
      FD_SET(sd,&rset);
      accept_timeout.tv_sec=SERVER_TIMEOUT;
      accept_timeout.tv_usec=0;
      if(loglevel>0 && pktCount>0){
	printf("Writing to file\n");
	sprintf(fpath,"%s/%d",logpath,exp_id);
	mkdir_status=mkdir(fpath,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	printf("Result of mkdir %d, for mkdir(%s).\n",mkdir_status,fpath);
	output_file(exp_id,run_id, logdat,pducount, CPU_before);
      }
      if(runAsDaemon){
	printf("Running as Daemon.\n");
	/*Clean up */
	 pktCount=0;
	 byteCount=0;
	 pducount=0;
	 for(int i=0;i<MAX_PDU;i++){
	   logdat[i].send_start=0;
	   logdat[i].send_stop=0;
	   logdat[i].recv_start=0;
	   logdat[i].recv_stop=0;
	 }
	 counter=-1;
	 
	 if(dT!=0){
	   gettimeofday(&theTime, NULL);
	   printf("Initializing the sampling every %g second.\n",dT);
	   signal(SIGALRM, Sample);
	   setitimer(ITIMER_REAL,&sampletimeOff,NULL); //used for termination with SIGALRM
	    } else {
	   printf("Sampling disabled.\n");
	 }
	 
	 

	cond=1;
      }else {
	printf("One hit wonder.\n");
	cond=0;
      }
    } else {
      n = recvfrom(sd, msg, MAX_MSG, 0,(struct sockaddr *) &cliAddr,(socklen_t*) &cliLen);
      rstop=realcc();
      gettimeofday(&PktArr,NULL);
      if(n<0){
	/*  printf("%s: cannot receive data \n",argv[0]); */
	continue;
      } else {
	if(n<40){
	  byteCount+=n;
	  pktCount++;
	  printf("[%d]: Got small packet, %d \n", (int)pktCount,(int)byteCount);
	} else {
	  /* print received message */
	  message=(transfer_data*)msg;
	  byteCount+=n;
	  pktCount++;
	  
	  if(loglevel>2){
	  printf("NORD:%d:%d:%d\n",message->exp_id,message->run_id,message->key_id);
	  printf("HORD:%d:%d:%d\n",ntohl(message->exp_id),ntohl(message->run_id),ntohl(message->key_id));
	  }
	  charErr=0;
	  //	printf("Payload is %d bytes.\n", n);
	  for(Acounter=0;Acounter<(n-(sizeof(transfer_data)-1500));Acounter++){
	    if(message->junk[Acounter]!='x'){
	      printf("Err: %c (%d) ", (message->junk[Acounter]),Acounter);
	      charErr++;
	    } else {
	      //				printf("Received: %c ", (message->junk[Acounter]));
	    }
	  }	
	  if(charErr>0){
	    printf("CharError is %d\n",charErr);
	  }
	  if( (ntohl(message->exp_id)!=exp_id) || (ntohl(message->run_id)!=run_id) || (ntohl(message->key_id)!=key_id) ){ 
	    printf("Missmatch of exp/run/key_id %u:%u:%u expected %u:%u:%u .\n", ntohl(message->exp_id),ntohl(message->run_id),ntohl(message->key_id), exp_id,run_id,key_id);
	    
	  }
	  
	  if( (counter==-1) ){
	    if(dT!=0){
	      gettimeofday(&theTime, NULL);
	      printf("Initializing the sampling every %g second.\n",dT);
	      signal(SIGALRM, Sample);
	      setitimer(ITIMER_REAL,&sampletime,NULL); //used for termination with SIGALRM
	    } else {
	      printf("Sampling disabled.\n");
	    }
	    msgcounter=ntohl(message->counter); /* Init the counter */
	    if(wildcard==1){
	      exp_id=ntohl(message->exp_id);
	      run_id=ntohl(message->run_id);
	      key_id=ntohl(message->key_id);
	    }
	    printf("Initial message;%u:%u:%u;%u:%u:%u;(Got;expected)\n", ntohl(message->exp_id),ntohl(message->run_id),ntohl(message->key_id), exp_id,run_id,key_id);
	    if(msgcounter!=0) {
	      printf("First packet did not hold 0 as it should, it contained the value %d.\n", msgcounter);
	    }
	  } else {
	    msgcounter++;
	    if(msgcounter!=ntohl(message->counter)){
	      if(ntohl(message->counter)==0) {
		/* Probably a new client. Make no fuss about it.*/
		msgcounter=ntohl(message->counter);	
	      } else {
		//	      printf("Packet missmatch, expected %d got %d, a loss of %d packets.\n",msgcounter, message->counter,message->counter-msgcounter);
		msgcounter=ntohl(message->counter);	
	      }
	    }
	  }
	  
	  counter++;
	  int arrpos=ntohl(message->counter);
	  //Store the SEQnr of this PDU
	  logdat[arrpos].seq_no=arrpos;
	  // Store the sending time of the previous PDU
	  if((arrpos-1)>=0){
	    logdat[arrpos-1].send_start=message->starttime;
	    logdat[arrpos-1].send_stop=message->stoptime;
	    logdat[arrpos-1].send_dept_time=message->depttime;
	  }
	  // Store the receive time of this PDU. 
	  logdat[arrpos].recv_start=rstart;
	  logdat[arrpos].recv_stop=rstop;
	  logdat[arrpos].recv_arrival_time=PktArr;
	  
	  
	  pducount++;  	    	
	  
	  //	printf("%d\t %llu\t %llu\n", message->counter, rstart, rstop);
	  rstart=0;
	  rstop=0;
	  if(counter%10000==0 && counter!=0) {
	    //	  printf("%s: from %s:UDP:%u  COUNTER= %d \n",argv[0],inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port), message->counter);
	  }
	}
      }
    }
  }/* end of server infinite loop */
  
  if(loglevel>1){  
    gettimeofday(&GTOD_after,NULL);
    TSC_after=realcc();
    CPU_after=estimateCPU(40,100000,fname_cpu);
    
    printf("Start:%d.%06ld - %llu\n", (int)GTOD_before.tv_sec, GTOD_before.tv_usec, TSC_before);
    printf("Stop:%d.%06ld - %llu\n", (int)GTOD_after.tv_sec, GTOD_after.tv_usec, TSC_after);
    printf("CPU before: %f \n", CPU_before);
    printf("CPU after: %f \n", CPU_after);
  }


  return 0;
  
}

void
close_con(int sig)
{
    setitimer(ITIMER_REAL,&diftime,NULL);
    gettimeofday(&tid2,NULL);
    if((tid3.tv_sec+10)<=tid2.tv_sec){
    printf("Closing the Socket. \n");
    cond=0;
    }else{
    cond=1;
    }
	
    }
//    counter=0;




void output_file(u_int32_t eid,u_int32_t rid, pdudata rpdu[],int sz, double freq){
  printf(">output_file(%d,%d, %p, %d)\n",eid, rid, &rpdu, sz);
  int n,rc=0;
  u_int64_t iptr=0,ipts=0, ipgr=0,ipgs=0;
  //  double av_iptr=0,av_ipts=0,av_ipgr=0,av_ipgs=0;
  //double var_iptr=0,var_ipts=0,var_ipgr=0,var_ipgs=0;
  if(sz==0){
    printf("No data to save.\n");
    return;
  }

  char fname[200];
  bzero(&fname,200);
  /*
  strcat(fname, eid);
  strcat(fname, "/");
  strcat(fname, filename);
  strcat(fname, "_server.txt");
  */
  sprintf(fname,"%d/udpserver-%d-%d.txt",eid,eid,rid);
  

  FILE *pFile;
  printf("LOGGING DATA to %s.\n", fname);
  pFile=fopen(fname,"a+");
  if(pFile==NULL){
    printf("Problems with file.\n");
    return;
  }

  for(n=0;n<sz;n++){
    /*to find iptr,ipts,ipgr,ipgs using sample(start)*/
    if(n>0){
      if( (rpdu[n].send_stop!=0) & (rpdu[n-1].send_stop!=0) ){
	ipts=(rpdu[n].send_stop-rpdu[n-1].send_stop);
	sample_ipts.PutSample((double)ipts);
      }
      if( (rpdu[n].send_start!=0) & (rpdu[n-1].send_start!=0) ){
	ipgs=(rpdu[n].send_start-rpdu[n-1].send_stop);
	sample_ipgs.PutSample((double)ipgs);
      }
      if( (rpdu[n].recv_stop!=0) & (rpdu[n-1].recv_stop!=0) ){
	iptr=(rpdu[n].recv_stop-rpdu[n-1].recv_stop);
	sample_iptr.PutSample((double)iptr);
	rc++;
      }
      if( (rpdu[n].recv_start!=0) & (rpdu[n-1].recv_start!=0) ){
	ipgr=(rpdu[n].recv_start-rpdu[n-1].recv_stop);
	sample_ipgr.PutSample((double)ipgr);
      }
    }
  }

  /*  var_ipts=sample_ipts.GetSampleVar();
  av_ipts=sample_ipts.GetSampleMean();
  var_iptr=sample_iptr.GetSampleVar();
  av_iptr=sample_iptr.GetSampleMean();
  var_ipgs=sample_ipgs.GetSampleVar();
  av_ipgs=sample_ipgs.GetSampleMean();
  var_ipgr=sample_ipgr.GetSampleVar();
  av_ipgr=sample_ipgr.GetSampleMean();
  */

  fprintf(pFile, "Statitics Section\n");
  fprintf(pFile, "Total number of PDU recieved %d\n",sz);
  fprintf(pFile,"param:mean:var;min;max\n");
  fprintf(pFile,"IPTS:%f:%f:%f:%f\n",sample_ipts.GetSampleMean(),sample_ipts.GetSampleVar(),sample_ipts.GetSampleMin(),sample_ipts.GetSampleMax());
  fprintf(pFile,"IPGS:%f:%f:%f:%f\n",sample_ipgs.GetSampleMean(),sample_ipgs.GetSampleVar(),sample_ipgs.GetSampleMin(),sample_ipgs.GetSampleMax());
  fprintf(pFile,"IPTR:%f:%f:%f:%f\n",sample_iptr.GetSampleMean(),sample_iptr.GetSampleVar(),sample_iptr.GetSampleMin(),sample_iptr.GetSampleMax());
  fprintf(pFile,"IPGR:%f:%f:%f:%f\n",sample_ipgr.GetSampleMean(),sample_ipgr.GetSampleVar(),sample_ipgr.GetSampleMin(),sample_ipgr.GetSampleMax());
  fprintf(pFile, "\n*************************************************************************************************\
\n");


  fprintf(pFile, "Index\tSeqNo\tS.start\tS.stop\tR.start\tR.stop\tDeptTime\tArrTime\n");
  for(n=0;n<sz;n++){
    fprintf(pFile, "%d\t%d\t%llu\t%llu\t%llu\t%llu\t%ld.%06ld\t%ld.%06ld\n", n, rpdu[n].seq_no, rpdu[n].send_start, rpdu[n].send_stop, rpdu[n].recv_start,rpdu[n].recv_stop,rpdu[n].send_dept_time.tv_sec,rpdu[n].send_dept_time.tv_usec,rpdu[n].recv_arrival_time.tv_sec,rpdu[n].recv_arrival_time.tv_usec);
    //    printf("%d\t %llu\t %llu\t %llu\t %llu\n", rpdu[n].seq_no, rpdu[n].send_start, rpdu[n].send_stop, rpdu[n].recv_start, rpdu[n].recv_stop);
    //    rpdu[n].seq_no=0; 
    //rpdu[n].send_start=0;
    //rpdu[n].send_stop=0;
    //rpdu[n].recv_start=0; 
    //rpdu[n].recv_stop=0; 
  } 
  fclose(pFile);
  printf("<output_file()\n");
  return;
}

double estimateCPU(int samples, int sleeptime, char *filename){

  struct tm *s;
  char tms[40];
  u_int64_t difft;
  float freq;
  double freq_avg=0;
  unsigned long microseconds,seconds;
  struct timeval data;
  struct timeval st[100];
  u_int64_t cputime[100];

  double dseconds;
  if(samples>100){
    samples=100;
  }
  FILE *pFile=0;
  if(filename!=NULL) {
    pFile=fopen(filename,"a+");
    fprintf(pFile, "Time\tCpu Cycles\tFrequency\n");
    printf("Opened file.\n");
  }

  for(int i=0;i<samples;i++){
    gettimeofday(&data,NULL);
    cputime[i]=realcc();
    st[i]=data;
    usleep(sleeptime);

  }


  for(int n=0;n<samples;n++){
    s=localtime(&st[n].tv_sec);
    strftime (tms, sizeof (tms), "%Y-%m-%d %H:%M:%S", s);
    if(n-1>=0){
      difft=cputime[n]-cputime[n-1];
      if(st[n-1].tv_usec>st[n].tv_usec){
        microseconds=(st[n].tv_usec+1000000)-st[n-1].tv_usec;
        seconds=(st[n].tv_sec-1)-st[n-1].tv_sec;
      }else{
        microseconds=st[n].tv_usec-st[n-1].tv_usec;
        seconds=st[n].tv_sec-st[n-1].tv_sec;
      }
      dseconds=seconds+(microseconds/1e6);
      freq=difft/dseconds;
      freq_avg+=freq;
    }else{
      freq=0;
      freq_avg=0;
    }
    if(pFile!=NULL){
      fprintf(pFile, "%s.%06ld\t%llu\t%f\n",tms, st[n].tv_usec,cputime[n],freq);
    }
  }
  if(pFile!=NULL){
    fprintf(pFile,"Average CPU = %f\n", (freq_avg)/(double)(samples));
    fprintf(pFile,"***\n");
    fclose(pFile);
  }
  return (freq_avg)/(double)(samples);
}


void Sample(int sig){
	gettimeofday(&theTime, NULL);
	printf("%ld.%06ld:%g:%g:%g:%g\n",theTime.tv_sec,theTime.tv_usec,8*byteCount,pktCount,(8*byteCount)/dT,pktCount/dT);
	byteCount=0;
	pktCount=0;
  return;
}

