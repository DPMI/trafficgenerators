

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <string.h> /* memset() */
#include <getopt.h>
#include <sys/time.h> /* select() */
#include <stdlib.h>
#include <iostream>


#define SUCCESS 0
#define ERROR   1

#define END_LINE 0x00
#define SERVER_PORT 1500
#define MAX_MSG 65536
#define MAX_PDU	100000

#include "udpgen.h"
#include "sample.h"
using namespace std;

double estimateCPU(int samples, int sleeptime, char* fname);

SAMPLE sample_iptr,sample_ipgr,sample_ipts,sample_ipgs;
void output_file(u_int32_t, u_int32_t,struct pdudata[],int, double);



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

struct pdudata logdat[MAX_PDU];
int pducount=0;

static inline u_int64_t realcc(void){
  u_int64_t cc;
  asm volatile("rdtsc":"=&A"(cc));
  return cc;
}


int main (int argc, char *argv[]) {
  
  int sd, newSd, cliLen, recvBytes, recvPkts;
  int segsize=MAX_MSG;
int option_index, op,reqFlag=0;	
  u_int32_t exp_id,run_id,key_id;
  exp_id=run_id=key_id=0;
int hflag=0;
int myPort=SERVER_PORT;
double CPU_before, CPU_after;
  struct timeval GTOD_before, GTOD_after, PktArr, tidf;
  u_int64_t TSC_before,TSC_after;
u_int64_t rstart,rstop;/*rstart mean reciving start time*/
rstart=0;
  rstop=0;

static struct option long_options[]={
    {"experiment_id", required_argument , 0,'e'},
    {"run_id", required_argument , 0,'r'},
    {"key_id", required_argument , 0,'k'},
    {"port_no", required_argument, 0, 'p'},
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

  while ( (op =getopt_long(argc, argv, "k:e:r:p:h",long_options, &option_index))!=EOF) {
    switch (op){
    case 'e': /* experiment_id */
      exp_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      break;
    case 'r': /* experiment_id */
      run_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      break;
    case 'k': /* experiment_id */
      key_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      break;
    case 'p':/*port*/
      myPort=atoi(optarg);
      break;
   
   

		case 'h': /* Help */
			printf("TCP server. v6\n");
			printf("-e --experiment_id	Experiment id [required]\n");
			printf("-r --run_id	 				Run id [required]\n");
			printf("-k --key_id	 				Key id [required]\n");
  		printf("-p --port_no 				Port to listen on, default %d\n", SERVER_PORT);
			printf("-h   help \n\n");
			printf("Logs the data into a directory <experiment_id> that must exist, save the data as \n");
			printf("<run_id>_server.txt\n");
			printf("<run_id>_recv_cpueval.txt\n");
			printf("The sender <run_id>_send_cpueva.txt must be collected from the sender after experiments completed.\n");
			printf("\n");
			hflag=1;
			break;
    default:
			hflag=1;

      printf("Error....\n");
      break;
    }
  }

	
	if (hflag) exit(EXIT_SUCCESS);
	if(reqFlag<3) {
			printf("You are missing required info. %d \n", reqFlag);
			exit(EXIT_FAILURE);
	}	

char fname_cpu[200];
  bzero(&fname_cpu,200);
  /*
  strcat(fname_cpu, exp_id);
  strcat(fname_cpu, "/");
  strcat(fname_cpu, run_id);
  strcat(fname_cpu, "_recv_cpueval.txt");
  */

  sprintf(fname_cpu,"%d_%d_recvtcp_cpueval.txt",exp_id,run_id);

  printf("Writes cpu data to %s.\n", fname_cpu);
 CPU_before=estimateCPU(40,100000,fname_cpu);
  TSC_before=realcc();
  gettimeofday(&GTOD_before,NULL);
  printf("Estimated cpu to %f Hz.\n",CPU_before);
  
  struct sockaddr_in cliAddr, servAddr;
  char msg[MAX_MSG];


 // myPort=atoi(argv[1]);
  segsize=1500;
printf("Hi");


  /* create socket */
  sd = socket(AF_INET, SOCK_STREAM, 0);
   if(sd<0) {
    perror("cannot open socket ");
    return ERROR;
  }
  
  /* bind server port */
  servAddr.sin_family = AF_INET;
  servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servAddr.sin_port = htons(myPort);
  
  if(bind(sd, (struct sockaddr *) &servAddr, sizeof(servAddr))<0) {
    perror("cannot bind port ");
    return ERROR;
  }

  listen(sd,5);// 5 connections

  
    struct tm *file_stime;
    char file_name[20];
    file_name[19]='\0';
    gettimeofday(&tidf,NULL);
    file_stime=localtime(&tidf.tv_sec);
    strftime(file_name,20,"%Y-%m-%d %H.%M.%S",file_stime);
    int cond=1;
    struct timeval accept_timeout;
    fd_set rset;
    
    char filename[200];
    bzero(&filename,200);
    sprintf(filename,"tcpserver.log");
      
    FILE *pFile;
    pFile=fopen(filename,"a+");
    fprintf(pFile, "\n%s **Server Starting**\n",file_name);
    fclose(pFile);
    

  
     while(sd!=0) {
    printf("%s: waiting for data on port TCP %u, will read %d segments \n",argv[0],myPort, segsize);
    cliLen = sizeof(cliAddr);
    newSd = accept(sd, (struct sockaddr *) &cliAddr, (socklen_t*)&cliLen);
    if(newSd<0) {
      perror("cannot accept connection ");
      return ERROR;
    }
    printf("Connected to : %s : %d \n", inet_ntoa(cliAddr.sin_addr) ,ntohs(cliAddr.sin_port) );

    pFile=fopen(filename,"a+");
    if (pFile == NULL)
   printf("Helloworld");
    gettimeofday(&tidf,NULL);
    file_stime=localtime(&tidf.tv_sec);
    strftime(file_name,20,"%Y-%m-%d %H.%M.%S",file_stime);
    fprintf(pFile, "%s: ",file_name);
    fprintf(pFile, "%s %d Connected \n",inet_ntoa(cliAddr.sin_addr),ntohs(cliAddr.sin_port));
    fclose(pFile);

    recvBytes=0;
    recvPkts=0;
    /* init line */
    memset(msg,0x0,MAX_MSG);
    /* receive segments */
    
    int n,n2;
    double byteCount,pktCount;
    transfer_data *message; 
    unsigned int Acounter=0;
    int charErr=0;
    int counter=-1;
    
    unsigned int msgcounter=0;
    segsize=1500;
     double x;//mycheckpoint
    while(newSd!=0) {
      n2=0;
      n=0;
//memset(msg,0x0,MAX_MSG);
      
int iii;// packet counter.
 //---------------------------------------------------------------------------------------------------------------     
      while(n<segsize && n2!=-1){
        memset(msg,0x0,MAX_MSG);
	rstart = realcc();
        
	n2 = read(newSd, msg, (segsize));//&msg[n2]
        rstop=realcc();
        cout<<rstop<<"\n";
        gettimeofday(&PktArr,NULL);
        cout<<n2<<"\n";
        x+=n2;
        cout<<x<<"net bytes\n";
       // printf("Read %d of %d bytes\n",n2,segsize);
	if(n2<0){
	  //	  printf("n2<0, %d: newSd = %d, msg = %p, msg[n2] = %p, msg+MAX = %p, segsize=%d, segsize-n2=%d", n2,newSd,&msg, &msg[n2], &msg+MAX_MSG, segsize, segsize-n2);
	  perror("problem");
	  n2=-1;
	  n=-1;
	  close(newSd);
	  close(sd);
	  newSd=0;
	  sd=0;
	  exit(0);
	} 
	if (n2==0) {
  gettimeofday(&GTOD_after,NULL);
  TSC_after=realcc();
  CPU_after=estimateCPU(40,100000,fname_cpu);

  printf("Start:%d.%06ld - %llu\n", (int)GTOD_before.tv_sec, GTOD_before.tv_usec, TSC_before);
  printf("Stop:%d.%06ld - %llu\n", (int)GTOD_after.tv_sec, GTOD_after.tv_usec, TSC_after);
  printf("CPU before: %f \n", CPU_before);
  printf("CPU after: %f \n", CPU_after); 
output_file(exp_id,run_id, logdat,pducount, CPU_before);
	  perror("EoF");
	  n2=-1;
	  n=-1;
	  close(newSd);
	  newSd=0;
	  //	  goto newconn;
	  close(sd);
	  sd=0;
	  exit(0);
	  
	} 
	
		
	n=n2;//	
   //  printf("Read %d of %d bytes\n",n2,n);
	if(counter==-1){
	  n2=-1;
	}
if(n<40){
	  //	  Printf("1.\n");
	  byteCount+=n;
	  pktCount++;
	  printf("[%g]: Got small packet, %g \n", pktCount,byteCount);
	} 
else {
	/* print received message */
	message=(transfer_data*)msg;
	byteCount+=n;
	pktCount++;
         iii++;
        printf("recieved packet no. %d \n",ntohl(message->counter));
	/*
	printf("NORD:%d:%d:%d\n",message->exp_id,message->run_id,message->key_id);
	printf("HORD:%d:%d:%d\n",ntohl(message->exp_id),ntohl(message->run_id),ntohl(message->key_id));
	*/
/*	charErr=0;
		printf("Payload is %d bytes.\n", n);
	for(Acounter=0;Acounter<(n-(sizeof(transfer_data)-1500));Acounter++){
		if(message->junk[Acounter]!='x'){
				printf("Err: %c (%d) ", (message->junk[Acounter]),Acounter);
				charErr++;
			} else {
				//printf("Received: %c ", (message->junk[Acounter]));
			}
		}
if(charErr>0){
		printf("CharError is %d\n",charErr);
	}
	if( (ntohl(message->exp_id)!=exp_id) || (ntohl(message->run_id)!=run_id) || (ntohl(message->key_id)!=key_id) ){ 
	  printf("Missmatch of exp/run/key_id %lu:%lu:%lu expected %u:%u:%u .\n", ntohl(message->exp_id),ntohl(message->run_id),ntohl(message->key_id), exp_id,run_id,key_id);
	}
if( (counter==-1) )
{
msgcounter=ntohl(message->counter); // Init the counter /
	  printf("Initial message;%lu:%lu:%lu;%u:%u:%u;(Got;expected)\n", ntohl(message->exp_id),ntohl(message->run_id),ntohl(message->key_id), exp_id,run_id,key_id);
	  if(msgcounter!=0) {
	    printf("First packet did not hold 0 as it should, it contained the value %d.\n", msgcounter);
	  }
	} 
else {  //
	  msgcounter++;
	  if(msgcounter!=ntohl(message->counter)){
	    if(ntohl(message->counter)==0) {
	      //* Probably a new client. Make no fuss about it./
	      msgcounter=ntohl(message->counter);	
	    } else {
	      //	      printf("Packet missmatch, expected %d got %d, a loss of %d packets.\n",msgcounter, message->counter,message->counter-msgcounter);
	      msgcounter=ntohl(message->counter);	
	    }
	  }
	} //
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
*/
	
}// here i am 17 59 else above print received
printf("heyyyyyyyy end of while loop\n");
      }// end of while loop


  

 
      }
     }
 
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
  FILE *pFile;
  pFile=fopen(filename,"a+");
  fprintf(pFile, "Time\tCpu Cycles\tFrequency\n");
  printf("Opened file.\n");

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
    fprintf(pFile, "%s.%06ld\t%llu\t%f\n",tms, st[n].tv_usec,cputime[n],freq);
  }
  fprintf(pFile,"Average CPU = %f\n", (freq_avg)/(double)(samples));
  fprintf(pFile,"***\n");
  fclose(pFile);
  return (freq_avg)/(double)(samples);

}


void output_file(u_int32_t eid,u_int32_t rid, pdudata rpdu[],int sz, double freq){
  printf(">output_file(%d,%d, %p, %d)\n",eid, rid, &rpdu, sz);
  int n,rc=0;
  u_int64_t iptr=0,ipts=0, ipgr=0,ipgs=0;
  double av_iptr=0,av_ipts=0,av_ipgr=0,av_ipgs=0;
  double var_iptr=0,var_ipts=0,var_ipgr=0,var_ipgs=0;
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
  sprintf(fname,"%d_x_%d_tcppserver.txt",eid,rid);
  

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
      if(rpdu[n].send_stop!=0 & rpdu[n-1].send_stop!=0){
	ipts=(rpdu[n].send_stop-rpdu[n-1].send_stop);
	sample_ipts.PutSample((double)ipts);
      }
      if(rpdu[n].send_start!=0 & rpdu[n-1].send_start!=0 ){
	ipgs=(rpdu[n].send_start-rpdu[n-1].send_stop);
	sample_ipgs.PutSample((double)ipgs);
      }
      if(rpdu[n].recv_stop!=0 & rpdu[n-1].recv_stop!=0){
	iptr=(rpdu[n].recv_stop-rpdu[n-1].recv_stop);
	sample_iptr.PutSample((double)iptr);
	rc++;
      }
      if(rpdu[n].recv_start!=0 & rpdu[n-1].recv_start!=0){
	ipgr=(rpdu[n].recv_start-rpdu[n-1].recv_stop);
	sample_ipgr.PutSample((double)ipgr);
      }
    }
  }

  var_ipts=sample_ipts.GetSampleVar();
  av_ipts=sample_ipts.GetSampleMean();
  var_iptr=sample_iptr.GetSampleVar();
  av_iptr=sample_iptr.GetSampleMean();
  var_ipgs=sample_ipgs.GetSampleVar();
  av_ipgs=sample_ipgs.GetSampleMean();
  var_ipgr=sample_ipgr.GetSampleVar();
  av_ipgr=sample_ipgr.GetSampleMean();

  fprintf(pFile, "TCPStatitics Section\n");
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



