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
#include <netinet/tcp.h>

#include <getopt.h> /* Variable calling of main */
#include <iostream>
#include <time.h>
#include <math.h>

#include "rnd.h"
#include "rndunif.h"
#include "rndexp.h"
#include "rnddet.h"
#include "rndunid.h"


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
double runPkts,runPkts_1;
int size,noBreak,size1;
int difference_size,sample_length;


#define MAX_MSG 65536

int SERVER_PORT=1500;


int main (int argc, char *argv[]) {

  int option_index,op,sd, rc, hflag, reqFlag;
  double waittime,sleepTime,waittime1, sleepTime1;
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
  waittime1= 1000000;
  char psd;
  char wtd;
  psd ='x';
  wtd ='x';
  
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
    {"waittimemin", required_argument, 0, 'w'},
    {"waittimemax", required_argument, 0, 'W'},
    {"waitdist", required_argument, 0, 'v'},
    {"samplelength", required_argument, 0, 'z'},
    {"down", required_argument, 0, 'd'},
    {"help", required_argument, 0, 'h'},
    {0, 0, 0, 0}
  };
  
  

  /* check command line args, so that we are atleast in the correct "area" */
  if(argc<1){
    printf("use %s -h for help\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  hflag=0;
  runType=0;
  reqFlag=0;
  size=1224;
  size1 = 1224;
  sleepTime=-1;
  waittime1 = 1000000;

  waittime=0;  
  while ( (op =getopt_long(argc, argv, "k:e:r:s:p:m:n:l:L:v:w:W:z:d:h",long_options, &option_index))!=EOF) {
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
      runPkts=atof(optarg);
      runType=1;
      break;
      break;
    case 'm': /*pkt size distribution*/
      psd=*optarg;
      cout <<" PSD is"<<psd <<"\n" ;
      break;
    case 'l': /*pkt length*/
      size=atoi(optarg);
      break;
     case 'L': /*pkt length maxima*/
      size1=atoi(optarg);
      cout<< "Max packet Size is "<<size1 <<"\n";
      break;
    case 'd': /* download */
      direction=1;
      break;
    case 'v': /*waittime distribution*/
	wtd=*optarg;
	break;
    
		
    case 'w': /*waittime*/
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
      printf(" -m (--pktsize distribution) e- exponential u- uniform d- discrete uniform default- deterministic\n\n");
      printf(" -w (--waittime) <Inter frame gap, in usec.> [optional, but if set, voids desired]\n");
      printf(" -v (--wait time distribution) e- exponential u- uniform d- discrete uniform default- deterministic\n\n");
	  printf(" -d (--down) 	Download, do not upload.\n");
	  printf(" -z  Enter the sample length (integer)\n");    
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

  RND* myRND1;// packet size distribution
  RND* myRND2; // wait time distribution
  switch(psd){
	case 'e':
		printf("Expontial...");
		myRND1=new RNDEXP(size1);
	break;
	case 'u':
		printf("Uniform...");
		myRND1=new RNDUNIF(size,size1);
	break;

	case 'd':
               printf ("uniform discrete");
                myRND1 = new RNDUNID(size,size1);
               break;
	default:
	printf("DEfault is to deterministic ");
		myRND1=new RNDDET(size1);
	 	break;
  }

  switch(wtd){
	case 'e':
		printf("Expontial...");
		myRND2=new RNDEXP(waittime1);
	break;
	case 'u':
		printf("Uniform...");
		myRND2=new RNDUNIF(waittime,waittime1);
	break;

	case 'd':
         printf("uniform discrete");
		myRND2=new RNDUNID(waittime,waittime1);
          break;
	default:
	printf("DEfaults to determ");
		myRND2=new RNDDET(waittime1);
	 	break;
  }
  (*myRND1).printseed();
  (*myRND2).printseed();

  double pps= (1e6/(double)(waittime1));


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

  int flag=1;
  int resultTCP=setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
  if(resultTCP<0){ 
    perror("problem.");
  }

    
  


 
    transfer_data sender;
    char* psender=(char*)&sender;
    u_int64_t istart,istop;//,istart0,istop0;/*Var used for send start-stop time*/

    string test(1499,'x');
    strcpy(sender.junk, test.c_str());
	
//----------------
difference_size = size1 -size;
  runPkts_1 = floor (((difference_size)*runPkts)/sample_length) + runPkts;
   printf("will run %g pkts.\n",runPkts);
  cout <<" Experiment will run an overall of " << runPkts_1 <<"samples";
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
    istart=0;
    istop=0;

    printf("should take %g seconds .\n", (double)(runPkts_1)/pps);

    while(di<runPkts_1){
    // size=int(myRND1->Rnd());
     //    cout<< size<<"\n";
    waittime = (int) (myRND2->Rnd());
//cout<<waittime<<" wait time\n";
//waittime = waittime*1000;
		
      
      sender.counter=htonl((int)di);
      sender.starttime=istart;
      sender.stoptime= istop;
      sender.depttime=PktDept;
      istart=realcc();

      rc =write(sd, &sender,size);
      istop=realcc();
      gettimeofday(&PktDept,NULL); 
     // cout<<PktDept.tv_sec<<"."<<PktDept.tv_usec <<"\n";

      if(rc<0)          {
	printf("%s: cannot send data, Packet N#  %d,  size was %d bytes, sender %p \n",argv[0],(int)(di-1), size,&sender );
	close(sd);
	exit(1);
      }
      //	printf("%d\t %llu\t %llu\n", sender.counter, istart,istop);
      di++;
      if (int (di) %(int)runPkts == 0)
	{
	size = size + sample_length;
	//sleep(1);
	}
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

   if(noUsec>10000) {
    //    printf("Look Im slow.\n");
    usleep(noUsec);
  } else {
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
	 printf("sec loops 100000.\n ");
       }
     }
     
     loops=0;
     while(s.tv_sec<=e.tv_sec && s.tv_usec<e.tv_usec && loops<1000000 ){
       gettimeofday(&s,NULL);
       loops++;
     }
     if(loops>=1000000){
       printf("usec loops 100000.\n ");
       printf("Current %06ld s target %06ld s \t ",s.tv_sec,e.tv_sec   );
       printf("Current %06ld target us  too %06ld  us\n",s.tv_usec,e.tv_usec   );
     }
     if(s.tv_sec>e.tv_sec){
       printf("s sec > e sec.\n ");
       printf("Current %06ld s target %06ld s \t ",s.tv_sec,e.tv_sec   );
       printf("Current %06ld target us  too %06ld  us\n",s.tv_usec,e.tv_usec   );
     }
     
     gettimeofday(&s,NULL);
   }
//  printf("<uPause(%g)\n", noUsec);
}



