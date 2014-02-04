/* fpont 12/99 */
/* pont.net    */
/* tcpClient.c */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h> /* close */
#include <string.h> /* memset() */
#include <stdlib.h>
#include <sys/time.h> /* select() */

#include <getopt.h> /* Variable calling of main */
#include <iostream>
#include <time.h>
#include <math.h>


#include "udpgen.h"
using namespace std;

void uPause(double noUsec);
void closePrg(int sig);
void killPrg(int sig);

double estimateCPU(int samples, int sleeptime, char* fname);

static inline u_int64_t realcc(void){
 u_int64_t cc;
 asm volatile("rdtsc":"=&A"(cc));
 return cc;
}

struct timeval *s;
struct timeval start,data,stop;
double runPkts;
int size,noBreak;



#define MAX_MSG 65536

int SERVER_PORT=1500;


int main (int argc, char *argv[]) {

 int option_index,op,sd, rc, hflag, REMOTE_SERVER_PORT,reqFlag;
  double waittime,linkCapacity,sleepTime;
  struct sockaddr_in servAddr; //cliAddr, 
  struct sockaddr_in myAddr;
  struct hostent *h;
  char *serverName=0;
  u_int32_t exp_id,run_id,key_id;
  exp_id=run_id=key_id=0;

//  double CPU_before, CPU_after;
  struct timeval PktDept; //GTOD_before, GTOD_after, 
//  u_int64_t TSC_before,TSC_after;

	int direction=0;
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
	{"down", required_argument, 0, 'd'},
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
  while ( (op =getopt_long(argc, argv, "k:e:r:s:p:n:l:w:dh",long_options, &option_index))!=EOF) {
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
      SERVER_PORT=atoi(optarg);
      break;
    case 'n': /* number of pkts */
      runPkts=atof(optarg)+1;
      runType=1;
      break;
      break;
    case 'l': /*pkt length*/
      size=atoi(optarg);
      break;
	case 'd': /* download */
		direction=1;
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
	  printf(" -d (--down) 	Download, do not upload.\n");
	  
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

  servAddr.sin_family = h->h_addrtype;
  memcpy((char *) &servAddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
  servAddr.sin_port = htons(SERVER_PORT);

  /* create socket */
 
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if(sd<0) {
    perror("cannot open socket to");
    exit(1);
  }

  /* bind any port number */
  myAddr.sin_family = AF_INET;
  myAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  myAddr.sin_port = htons(0);
  
  printf("Connecting to: %s:%d \n", serverName, SERVER_PORT);;

  rc = bind(sd, (struct sockaddr *) &myAddr, sizeof(myAddr));
  if(rc<0) {
    printf("%s: cannot bind port TCP %u\n",argv[0],SERVER_PORT);
    perror("error ");
    exit(1);
  }
				
  /* connect to server */

  rc = connect(sd, (struct sockaddr *) &servAddr, sizeof(servAddr));
  if(rc<0) {
    perror("cannot connect ");
    exit(1);
 }

	
 
	transfer_data sender;
//    transfer_data *psender;
    u_int64_t istart,istop;//,istart0,istop0;/*Var used for send start-stop time*/

	 string test(1499,'x');
    strcpy(sender.junk, test.c_str());
	
//----------------
   printf("will run %g pkts.\n",runPkts);
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

    while(di<runPkts){
      sender.counter=htonl((int)di);
      sender.starttime=istart;
      sender.stoptime=istop;
      sender.depttime=PktDept;
      istart=realcc();
      rc =write(sd, &sender,size);
      istop=realcc();
      gettimeofday(&PktDept,NULL); 

      if(rc<0)          {
	printf("%s: cannot send data, Packet N#  %d,  size was %d bytes, sender %p \n",argv[0],(int)(di-1), size,&sender );
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

//----------------
 
 

  printf("Closing.\n");
  close(sd);
return 0;
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
    e.tv_usec=s.tv_usec+(suseconds_t)usecs-1000000;
    secJump=1;
  } else {
    e.tv_usec=s.tv_usec+(suseconds_t)usecs;
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
