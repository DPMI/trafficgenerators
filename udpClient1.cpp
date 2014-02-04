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
#include <string.h>
#include <iomanip>
#include <getopt.h> /* Variable calling of main */
#include <iostream>
#include <math.h>
#include "udpgen.h"

using namespace std;

void uPause(double noUsec);
void closePrg(int sig);
void killPrg(int sig);

//double estimateCPU(int samples, int sleeptime, char* fname);

static inline u_int64_t realcc(void){
 u_int64_t cc;
 asm volatile("rdtsc":"=&A"(cc));
 return cc;
}

struct timeval *s;
struct timeval start,data,stop;
double runPkts;
int size,noBreak;

int main(int argc, char *argv[]) {
  int option_index,op,sd, rc, hflag, REMOTE_SERVER_PORT,reqFlag;
  double waittime,linkCapacity,sleepTime;
  struct sockaddr_in cliAddr, remoteServAddr;
  struct sockaddr_in myAddr;
  struct hostent *h;
  char *serverName=0;
  u_int32_t exp_id,run_id,key_id;
  exp_id=run_id=key_id=0;

  double CPU_before, CPU_after;
  struct timeval GTOD_before, GTOD_after, PktDept;
  u_int64_t TSC_before,TSC_after;
  CPU_before=CPU_after=0;
  TSC_before=TSC_after=0;
 
  int runType; /* 0= default, forever, 1= nopkts, 2=time */
  noBreak=1;
  linkCapacity=9600;
  
  static struct option long_options[] =  {
	{"expid ",required_argument,0,'e'},
	{"keyid ",required_argument,0,'r'},
	{"runid ",required_argument,0,'k'},
	{"server",required_argument, 0, 's'},
	{"port", required_argument,0,'p'},
	{"pkts", required_argument,0,'n'},
	{"pktLen", required_argument, 0, 'l'},
	{"waittime", required_argument, 0, 'w'},
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

  waittime=0;  
  while ( (op =getopt_long(argc, argv, "k:e:r:s:p:n:l:w:h",long_options, &option_index))!=EOF) {
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
    case 'n': /* number of pkts */
      runPkts=atof(optarg);
      runType=1;
      break;
      break;
    case 'l': /*pkt length*/
      size=atoi(optarg);
      break;

    case 'w': /*pkt length*/
      sleepTime=atoi(optarg);
      waittime=sleepTime;
      reqFlag=4;
      break;
      
    case 'h': /*Help*/
      hflag=1;
      
      printf("%s\n",argv[0]);
      printf(" (C)2003 Patrik.Carlsson@bth.se\n http://www.bth.se/its/staff/pca\n");
      printf(" This tool was developed in the INGA project, based on funding from VINNOVA.\n\n");
      printf(" -h help (this text)\n");
      printf(" -e(--expid) Experiment id [required]\n");
      printf(" -r(--runid) Run id [required]\n");
      printf(" -k(--keyid) Key id [required]\n");
      printf(" -s (--server) Destination Server [required] \n");
      printf(" -p (--port) <Destination Port> [optional default = 1500] \n");
      printf(" -n (--pkts) <Number of packets to send> [optional default = forever]\n");
      printf(" -l (--pktLen) <Packet Length> [bytes] [optional default = 1224]\n\n");
      printf(" -w (--waittime) <Inter frame gap, in usec.> [optional, but if set, voids desired]\n");
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

  char fname_cpu[200];
  bzero(&fname_cpu,200);
  /*
  strcat(fname_cpu, exp_id);
  strcat(fname_cpu, "/");
  strcat(fname_cpu, run_id);
  strcat(fname_cpu, "_send_cpueval.txt");
  */
  sprintf(fname_cpu,"%d/%d_send_cpueval.txt", exp_id,run_id);
  /*
  printf("Writes cpu data to %s.\n", fname_cpu);
  CPU_before=estimateCPU(40,100000,fname_cpu);
  TSC_before=realcc();
  gettimeofday(&GTOD_before,NULL);
  printf("Estimated cpu to %f Hz.\n",CPU_before); 
  */

  printf("%s\nSending data to %s:%s port %d)\n", argv[0], h->h_name,inet_ntoa(*(struct in_addr *)h->h_addr_list[0]),REMOTE_SERVER_PORT);
  remoteServAddr.sin_family = h->h_addrtype;
  memcpy((char *) &remoteServAddr.sin_addr.s_addr,
	 h->h_addr_list[0], h->h_length);
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
  
  //  printf("Src port : %d \n", ntohs(cliAddr.sin_port));
  
  socklen_t len;
  len=128;
  

  rc=getsockname(sd, (struct sockaddr* ) &myAddr, &len);

    transfer_data sender;
    transfer_data *psender;

    string test(1499,'x');
    strcpy(sender.junk, test.c_str());
    psender=&sender;
    
    u_int64_t istart,istop,istart0,istop0;/*Var used for send start-stop time*/
    istart=0;
    istop=0;
    istart0=0;
    istop0=0;
    
    /* send data */
    s=&data;
    gettimeofday(s,NULL);
    
    //    cout<<s->tv_sec<<","<<s->tv_usec<<endl;
    start=*s;
    
    
    if(runType==1) {
    printf("will run %g pkts.\n",runPkts);
    double di=0;

    sender.exp_id=htonl(exp_id);
    sender.run_id=htonl(run_id);
    sender.key_id=htonl(key_id);
    
    printf("Experiment id=%d, run id=%d and key id = %d\n", exp_id,run_id,key_id);
    /*
    printf("HORD:%d:%d:%d\n", exp_id,run_id,key_id);
    printf("NORD%d:%d:%d\n", sender.exp_id,sender.run_id,sender.key_id);
    */
    PktDept.tv_sec=0;
    PktDept.tv_usec=0;

    while(di<runPkts){
      sender.counter=htonl((int)di);
      sender.starttime=istart;
      sender.stoptime=istop;
      sender.depttime=PktDept;
      istart=realcc();
      rc =sendto(sd, &sender,size, 0,(struct sockaddr *) &remoteServAddr,sizeof(remoteServAddr));
      istop=realcc();
      gettimeofday(&PktDept,NULL); 

      if(rc<0)          {
	printf("%s: cannot send data %d \n",argv[0],(int)(di-1));
	close(sd);
	exit(1);
      }
      //	printf("%d\t %llu\t %llu\n", sender.counter, istart,istop);
      di++;
      
      if(int(di)%1000==0) {
	cout << di << " pkts." <<endl;
      }
      uPause(waittime);
    }
    gettimeofday(s,NULL);
    stop=*s;
    }
    printf("Done\n");
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
  }
  if(s.tv_usec+usecs>1000000) {
    e.tv_sec=e.tv_sec+1;
    e.tv_usec=s.tv_usec+(long int)usecs-1000000;
    secJump=1;
  } else {
    e.tv_usec=s.tv_usec+(long int)usecs;
  }
  
  if(secJump==1) {
   while(s.tv_sec<e.tv_sec){
       gettimeofday(&s,NULL);
       loops++;
   }
  }
  while(s.tv_usec<e.tv_usec){
      gettimeofday(&s,NULL);
      loops++;
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
  FILE *pFile;
  pFile=fopen(filename,"a+");
  fprintf(pFile, "Time\tCpu Cycles\tFrequency\n");
  

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


