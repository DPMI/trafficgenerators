#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h> /* select() */
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <strings.h> //hhhhhhhhhhhhhhhhhhhh

#include <iomanip>
#include <getopt.h> /* Variable calling of main */
#include <iostream>
#include <math.h>
#include <cstring>/// cggggggggg
#include "udpgen.h"
#define lent 8 // sizeof double
#include "rndexp.h"
#include "rndunif.h"
#include "rnd.h"
#include "rnddet.h"
#include "rndunid.h"




using namespace std;

void uPause(double noUsec);
void closePrg(int sig);
void killPrg(int sig);
void randexpo(double a[], int xxx);

double estimateCPU(int samples, int sleeptime, char* fname);

static inline u_int64_t realcc(void){
 u_int64_t cc;
 asm volatile("rdtsc":"=&A"(cc));
 return cc;
}
//provide sample length no. of packets 32 x x x x x .. 32+ sample length..... 
struct timeval *s;
struct timeval start,data,stop;
double runPkts,runPkts_1;
int size,noBreak,size1;
int difference_size,sample_length;

int main(int argc, char *argv[]) {
//  double zzz[10000];// added today
//  int vvv;//added today
///   vvv = (sizeof(zzz)/lent);//***today
//randexpo(zzz,vvv);//***
  int option_index,op,sd, rc, hflag, REMOTE_SERVER_PORT,reqFlag;
  double waittime,waittime1,sleepTime,sleepTime1;
  //  double linkCapacity;
  struct sockaddr_in cliAddr, remoteServAddr;
  struct sockaddr_in myAddr;
  struct hostent *h;
  char *serverName=0;
  u_int32_t exp_id,run_id,key_id;
  exp_id=run_id=key_id=0;
  sample_length = -1; 
  runPkts_1 = 0;
  double CPU_before, CPU_after;
  struct timeval GTOD_before, GTOD_after, PktDept;
  u_int64_t TSC_before,TSC_after;
  int loglevel=1;
 
  int runType; /* 0= default, forever, 1= nopkts, 2=time */
  noBreak=1;
  //linkCapacity=9600;
  waittime1= 1000000;
  char psd, wtd;  
  psd='z';
  wtd='z';

  static struct option long_options[] =  {
	{"expid ",required_argument,0,'e'},
	{"keyid ",required_argument,0,'r'},
	{"runid ",required_argument,0,'k'},
	{"server",required_argument, 0, 's'},
	{"port", required_argument,0,'p'},
	{"pkts", required_argument,0,'n'},
	{"pktdist", required_argument,0,'m'},	
	{"pktLenmin", required_argument, 0, 'l'},
        {"pktLenMax", required_argument, 0, 'L'},
	{"pktLen",required_argument,0,'a'},
	{"waitdist", required_argument, 0, 'v'},
	{"waittime", required_argument, 0, 'i'},
	{"waittimemin", required_argument, 0, 'w'},
        {"waittimemax", required_argument, 0, 'W'},
	{"samplelength", required_argument, 0, 'z'},
	{"help", required_argument, 0, 'h'},
	{0, 0, 0, 0}
        };
  REMOTE_SERVER_PORT=1500;

  /* check command line args, so that we are atleast in the correct "area" */
  if(argc<1){
    printf("use %s -h for help\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  hflag=0;
  runType=0;
  reqFlag=0;
  size=1224;
  sleepTime=-1;
  size1=1224;
  waittime=0;  
  while ( (op =getopt_long(argc, argv, "k:e:r:s:p:n:m:l:L:v:i:w:W:z:h",long_options, &option_index))!=EOF) {
    switch (op){
    case 'e':/*exp_id*/
      exp_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      break;
    case 'r':/*exp_id*/
      run_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      break;
    case 'k':/*key_id*/
      key_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      break;
    case 's': /* Server */
      serverName=optarg;
      reqFlag++;
      break;
    case 'p': /* Port number */
      REMOTE_SERVER_PORT=atoi(optarg);
      break;
    case 'n': /* number of pkts per samplesize*/
      runPkts=atof(optarg);
      runType=1;
      break;
      break;
    case 'm': /*pkt size distribution*/
      psd=*optarg;
      /*      cout <<" Packet Size Distribution: ";
      switch (psd){
      case'e':
	cout << "Exponential.";
	break;
      case'u':
	cout << "Uniform.";
	break;
      case'd':
	cout << "Uniform Discrete.";
	break;
      default:
	cout << "Discrete.";
	break;
      }
      cout << "(" << psd <<")\n" ;
      */
      break;
    case 'l': /*pkt length min*/
      size=atoi(optarg);
      cout<< "Min packet Size is "<<size <<"\n";
      break;
    case 'L': /*pkt length maximal*/
      size1=atoi(optarg);
      cout<< "Max packet Size is "<<size1 <<"\n";
      break;
      
    case 'a' : /* packet length */
      size=atoi(optarg);
      size1=atoi(optarg);
      cout << "Packet size is " << size  << "\n";
      break;
    case 'v': /* distribution*/
      wtd=*optarg;
      break;
    case 'i': /* wait time*/
      sleepTime=atoi(optarg);
      waittime=sleepTime;
      waittime1=sleepTime;
      break;

    case 'w': /*pkt length*/
      sleepTime=atoi(optarg);
      waittime=sleepTime;
      reqFlag=4;
      break;
   case 'W': /*wait time maxima*/
      sleepTime1=atoi(optarg);
      cout<<"sleeptime Max is "<<sleepTime<<"\n";
      waittime1=sleepTime1;
      reqFlag=4;
      break;   
    case 'z': /*sample_length*/
      sample_length=atoi(optarg);
      cout<<"sample_length is "<<sample_length<<"\n";
      break;   
      
    case 'h': /*Help*/
      hflag=1;
      
      printf("%s\n",argv[0]);
      printf(" -h help (this text)\n");
      printf(" -e(--expid) Experiment id [required]\n");
      printf(" -r(--runid) Run id [required]\n");
      printf(" -k(--keyid) Key id [required]\n");
      printf(" -s (--server) Destination Server [required] \n");
      printf(" -p (--port) <Destination Port> [optional default = 1500] \n");
      printf(" -n (--pkts) <Number of packets to send> [optional default = forever]\n");
      printf(" -l (--pktLenmin) <Packet Length> [bytes] [optional default = 1224]\n");
      printf(" -L (--pktLenmax) <Packet Length> [bytes] [optional default = 1224]\n");
      printf("    --pktLen <Packet Length> [bytes] [ Optional, sets min=max=<value>\n");
      printf(" -m (--pktdist) e- exponential u- uniform d- discrete uniform default- deterministic\n");
      printf(" \n");
      printf(" -i (--waittime) <Inter frame gap, in usec.> [optional, but if set, voids desired]\n");
      printf(" -w (--waittimemin) <inter frame gap, in usec.> [optional ] .\n");
      printf(" -W (--waittimemax) <inter frame gap, in usec.> [optional ] .\n");
      printf(" -v (--waitdist) e- exponential u- uniform d- discrete uniform default- deterministic\n");
      printf(" \n");
      printf(" -z  Enter the sample length (integer) (optional) ");
      printf("        If used then it will be the number of samples per packet size in the distribution.)\n");      
      printf(" The -t and -n options are exclusive, if both are defined unknown behaviour might occur.\n");
      printf(" If neither is defined the software will run forever, or atleast until terminated. \n\n");
      break;
    default:
			hflag=1;
      printf("Use -h for instructions.\n");
      break;
    }
  }
  if (hflag) exit(EXIT_SUCCESS);
  if (reqFlag<4) {
    printf("Missing required arguments.\nRun %s -h for arguments.\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  /* get server IP address (no check if input is IP address or DNS name */
  h = gethostbyname(serverName);
  if(h==NULL) {
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    exit(1);
  }


  RND* myRND1;// packet size distribution
  RND* myRND2; // wait time distribution
  printf("Packet size distribution: ");
  switch(psd){
  case 'e':
    printf("Expontial, rndexp(%d).\n",size1);
    myRND1=new RNDEXP(size1);
    //	RNDEXP myRND1(size1);
    break;
  case 'u':
    printf("Uniform, rndunif(%d,%d)\n",size,size1);
    myRND1=new RNDUNIF(size,size1);
    //RNDUNIF myRND1(size,size1);		
    break;
    
  case 'd':
    printf ("Uniform Discrete, rndunid(%d,%d).\n",size,size1);
    myRND1 = new RNDUNID(size,size1);
    //RNDUNID myRND1(size,size1);
    break;
  default:
    printf("Default is to deterministic, rnddet(%d).\n ",size1);
    myRND1=new RNDDET(size1);
    //RNDDET myRND1(size1);
    break;
  }
  
  printf("Wait time distribution: ");
  switch(wtd){
  case 'e': 
    printf("Expontial, rndexp(%g)\n",waittime1);
    myRND2=new RNDEXP(waittime1);
    break;
  case 'u':
    printf("Uniform, rndunif(%g,%g)\n",waittime,waittime1);
    myRND2=new RNDUNIF(waittime,waittime1);
    break;
    
  case 'd':
    printf("Uniform Discrete, rndunid(%g,%g).\n",waittime,waittime1);
    myRND2=new RNDUNID(waittime,waittime1);
    break;
  default:
    printf("Defaults to deterministic, rnddet(%g)\n",waittime1);
    myRND2=new RNDDET(waittime1);
    break;
  }
  
	 printf("Seeds:\n");
	 printf("\tPktsize: ");
(*myRND1).printseed();
	 printf("\tWaittime: ");
(*myRND2).printseed();
	 
  char fname_cpu[200];
  bzero(&fname_cpu,200);
  /*
  strcat(fname_cpu, exp_id);
  strcat(fname_cpu, "/");
  strcat(fname_cpu, run_id);
  strcat(fname_cpu, "_send_cpueval.txt");
  */
  sprintf(fname_cpu,"%d_%d_send_cpueval.txt", exp_id,run_id);
  if(loglevel>1){
    printf("Writes cpu data to %s.\n", fname_cpu);
    //CPU_before=estimateCPU(40,100000,fname_cpu);
    TSC_before=realcc();
    gettimeofday(&GTOD_before,NULL);
    printf("Estimated cpu to %f Hz.\n",CPU_before); 
  }

  printf("%s: Sending data to %s (%s:%d)\n", argv[0], h->h_name,inet_ntoa(*(struct in_addr *)h->h_addr_list[0]),REMOTE_SERVER_PORT);
  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  remoteServAddr.sin_port = htons(REMOTE_SERVER_PORT);

  /* socket creation */
  sd = socket(AF_INET,SOCK_DGRAM,0);
  if(sd<0) {
    printf("%s: cannot open socket \n",argv[0]);
    exit(1);
  }

  /* bind any port */
  cliAddr.sin_family = AF_INET;
  cliAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  cliAddr.sin_port = 0;

  rc = bind(sd, (struct sockaddr *) &cliAddr, sizeof(cliAddr));
  if(rc<0) {
    printf("%s: cannot bind port\n", argv[0]);
    exit(1);
  }
  printf("Src port : %d \n", ntohs(cliAddr.sin_port));
  
  socklen_t len;
  len=128;
  

  rc=getsockname(sd, (struct sockaddr* ) &myAddr, &len);

  transfer_data sender;
  //transfer_data *psender;
  
  string test(1499,'x');
  strcpy(sender.junk, test.c_str());
  //    psender=&sender;
  
  u_int64_t istart,istop;
  //    u_int64_t istart0,istop0;/*Var used for send start-stop time*/
  istart=0;
  istop=0;
  //istart0=0;
  //istop0=0;
  
  /* send data */
  s=&data;
  gettimeofday(s,NULL);
  
  //    cout<<s->tv_sec<<","<<s->tv_usec<<endl;
  start=*s;

  // ((L-l)*num of samples )/sample_length
  //sample_length = 2;
  if (sample_length > 0){
    printf("Adapting number of packets, as to reach the desired sample size.\n"); 
    printf("Size1 %d, size = %d \n", size1,size);
    difference_size = size1 -size;
    printf("difference_size = %d , runPkts = %g , sample_length = %d , runPkts = %g \n", difference_size,runPkts,sample_length, runPkts_1);
    runPkts_1 = floor (((difference_size)*runPkts)/sample_length) + runPkts;
  } else {
    printf("Fixed no pkts. \n");
    runPkts_1 = runPkts;
  }
  /*
    printf("runType = %d runPkts = %g ", runType,runPkts);
    printf("runPkts_1 = %g \n", runPkts_1);
  */

  if(runType==1) {
    printf("will run %g pkts for each size.\n",runPkts);
    printf("Experiment will run an overall of %g samples.\n",runPkts_1);
    if (sample_length>0){
      printf("(%d *%d) / %g ==> %g \n",difference_size, runPkts, sample_length, floor (((difference_size)*runPkts)/sample_length));
    }
    double di=0;
    
    sender.exp_id=htonl(exp_id);
    sender.run_id=htonl(run_id);
    sender.key_id=htonl(key_id);
    
    printf("Sending:\n");
    printf("Experiment id=%d, run id=%d and key id = %d\n", exp_id,run_id,key_id);
    /*
    printf("HORD:%d:%d:%d\n", exp_id,run_id,key_id);
    printf("NORD%d:%d:%d\n", sender.exp_id,sender.run_id,sender.key_id);
    */
    PktDept.tv_sec=0;
    PktDept.tv_usec=0;

    if(runPkts_1==-1) {
      printf("Will run forever....\n");
      printf("Sending %d bytes.\n", size1);
      while(true){
	sender.counter=htonl((int)di);
	sender.starttime=istart;
	sender.stoptime=istop;
	sender.depttime=PktDept;
	istart=realcc();
	rc =sendto(sd, &sender,size1, 0,(struct sockaddr *) &remoteServAddr,sizeof(remoteServAddr));//size> app head
	istop=realcc();
	gettimeofday(&PktDept,NULL);
	if(rc<0)          {
	  printf("%s: cannot send data %d, error was %d and size was %d \n",argv[0],(int)(di-1),rc,size1);
	  close(sd);
	  exit(1);
	}
	if(loglevel>1){
	  printf("Sent ; %d < %d \n", (int)di, (int)runPkts_1);
	}
	di++;

	if(int(di)%1000==0) {
	  cout << di << " pkts." <<endl;
	}
	waittime=myRND2->Rnd();
	//       cout<< waittime<<"wait time\n" << "packet num is "<< di<<"\n";
	uPause(waittime);
      }      
      
      
    } else {
      printf("This is a finite run.\n");
      while(di<runPkts_1){
	sender.counter=htonl((int)di);
	sender.starttime=istart;
	sender.stoptime=istop;
	sender.depttime=PktDept;
	istart=realcc();
	rc =sendto(sd, &sender,size1, 0,(struct sockaddr *) &remoteServAddr,sizeof(remoteServAddr));//size> app head
	istop=realcc();
	gettimeofday(&PktDept,NULL); 
	if(rc<0)          {
	  printf("%s: cannot send data %d \n",argv[0],(int)(di-1));
	  close(sd);
	  exit(1);
	}
	if(loglevel>1){
	  printf("Sent ; %d < %d \n", (int)di, (int)runPkts_1);
	}
	di++;
	if (int (di) %(int)runPkts == 0)
	  {
	    size1 = size1 - sample_length;
	    
	  }
	
	if(int(di)%1000==0) {
	  cout << di << " pkts." <<endl;
	}
	waittime=myRND2->Rnd();
	//       cout<< waittime<<"wait time\n" << "packet num is "<< di<<"\n";
	uPause(waittime);
      }

    }
    
    printf("Sent %d pkts.\n",(int)di); 
    gettimeofday(s,NULL);
    stop=*s;
  }

if(loglevel>1){
  gettimeofday(&GTOD_after,NULL);
  TSC_after=realcc();
  //  CPU_after=estimateCPU(40,100000,fname_cpu);
  
  printf("Start:%d.%06ld - %llu\n", (int)GTOD_before.tv_sec, GTOD_before.tv_usec, TSC_before);
  printf("Stop:%d.%06ld - %llu\n", (int)GTOD_after.tv_sec, GTOD_after.tv_usec, TSC_after);
  printf("CPU before: %f \n", CPU_before);
  printf("CPU after: %f \n", CPU_after);
 }


  return 1;
}

void uPause(double noUsec){
//  printf(">uPause(%g)\t", noUsec);
  struct timeval s;
  struct timeval e;
  int secJump=0;
  int loops=0;
  int secs=(int)floor((double)noUsec/1000000.0);
  double usecs=noUsec-secs*1000000;
  gettimeofday(&s,NULL);
  e.tv_sec=s.tv_sec+secs; 
 
  if(secs>0) {
  	secJump=1;
	if(secs>1){
	  printf("Sec %d s!!",secs); 
	}

  }
  if(s.tv_usec+usecs>1000000) {
    e.tv_sec=e.tv_sec+1;
    e.tv_usec=s.tv_usec+(long int)usecs-1000000;
    secJump=1;
  } else {
    e.tv_usec=s.tv_usec+(long int)usecs;
  }
  
  if(secJump==1) {
    while(s.tv_sec<e.tv_sec && loops<1000000 ){
       gettimeofday(&s,NULL);
       loops++;
   }
    if(loops>=1000000){
      //printf("sec loops 100000.\n ");
    }
  }

  loops=0;
  while(s.tv_sec<=e.tv_sec && s.tv_usec<e.tv_usec && loops<1000000 ){
      gettimeofday(&s,NULL);
      loops++;
  }
  if(loops>=1000000){
    //    printf("usec loops 100000.\n ");
    //printf("Current %d s target %d s \t ",(int)s.tv_sec,(int)e.tv_sec   );
    //printf("Current %06ld target us  too %06ld  us\n",s.tv_usec,e.tv_usec   );
  }
  if(s.tv_sec>e.tv_sec){
    printf("s sec > e sec.\n ");
    printf("Current %d s target %d s \t ",(int)s.tv_sec,(int)e.tv_sec   );
    printf("Current %06ld target us  too %06ld  us\n",s.tv_usec,e.tv_usec   );
  }

  gettimeofday(&s,NULL);
//  printf("<uPause(%g)\n", noUsec);
}

void closePrg(int sig){
  noBreak=0;
  return;
}

void killPrg(int sig){
  noBreak=0;
  cout << "KILLED!!! " << endl;
  cout<<"Sent a total of " << runPkts << " pkts."<<endl;
  exit(EXIT_SUCCESS);
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
printf ("helllo %s \n",filename);
  FILE *pFile;
pFile=fopen(filename,"w+");
 if( (pFile=fopen(filename,"a+"))== NULL)
printf("--------\n");
//printf ("%d %d --- hellow orld \n", samples, sleeptime);

 fprintf( pFile,"Time\tCpu Cycles\tFrequency\n");
  

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
    fprintf(pFile, "%s.%06ld\t %llu\t\t%f \n",tms, st[n].tv_usec,cputime[n],freq);
  }
  fprintf(pFile,"Average CPU = %f\n", (freq_avg)/(double)(samples));
  fprintf(pFile,"***\n");
  fclose(pFile);
  return (freq_avg)/(double)(samples); 

}


