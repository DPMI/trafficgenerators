#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/time.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
//#include <netinet/ip_var.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
//#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <math.h>
#include <getopt.h>
#include "icmpgen.h"
#include "rndexp.h"
#include "rndunif.h"
#include "rnd.h"
#include "rnddet.h"
#include "rndunid.h"


using namespace std;
void uPause(double noUsec);

uint16_t in_cksum(uint16_t *addr, unsigned len);

#define	DEFDATALEN	(64-ICMP_MINLEN) //56	/* default data length */
#define	MAXIPLEN	60
#define	MAXICMPLEN	76
#define	MAXPACKET	(65536 - 60 - ICMP_MINLEN)/* max packet size */
u_char *packet, outpack[MAXPACKET];

int ping(int sss,u_int32_t exp, u_int32_t run, u_int32_t key, string target,u_int32_t count ,int size_packet)
{

  int  i, cc, packlen, datalen = size_packet;// DEFDATALEN;
  struct hostent *hp;
  struct sockaddr_in to, from;
  //struct protoent	*proto;
  struct ip *ip;
  //u_char *packet, outpack[MAXPACKET];
  char hnamebuf[MAXHOSTNAMELEN];
  string hostname;
  struct icmp *icp;
  int ret, fromlen, hlen;
  fd_set rfds;
  struct timeval tv;
  int retval;
  struct timeval start, end;
  int /*start_t, */end_t;
  bool cont = true;
  
  to.sin_family = AF_INET;
  
  // try to convert as dotted decimal address, else if that fails assume it's a hostname
  to.sin_addr.s_addr = inet_addr(target.c_str());
  if (to.sin_addr.s_addr != (u_int)-1)
    hostname = target;
  else 
    {
      hp = gethostbyname(target.c_str());
      if (!hp)
	{
	  cerr << "unknown host "<< target << endl;
	  return -1;
	}
      cout<<hp<<"\n";
      to.sin_family = hp->h_addrtype;
      bcopy(hp->h_addr, (caddr_t)&to.sin_addr, hp->h_length);
      strncpy(hnamebuf, hp->h_name, sizeof(hnamebuf) - 1);
      hostname = hnamebuf;
    }
  packlen = datalen + MAXIPLEN + MAXICMPLEN; //56 +60 + 76 = 192
  
  
  icp = (struct icmp *)outpack;
  icp->icmp_type = ICMP_ECHO;
  icp->icmp_code = 0;
  icp->icmp_cksum = 0;
  icp->icmp_seq = 12345;	/* seq and id must be reflected */
  icp->icmp_id = getpid();
  
  
  cc = datalen + ICMP_MINLEN;/////////vvvvvvvvvvvvvvvvvv
  
  struct transfer_data v;
  v.exp_id = htonl(exp);
  v.run_id = htonl(run);
  v.key_id = htonl(key);
  v.counter = htonl(count);
  struct transfer_data * xx;
  xx = &v;
  size_t ivan = 8;
  memmove(&outpack[ivan], &xx->exp_id,sizeof(xx->exp_id));
  ivan+= sizeof(xx->exp_id);
  memmove(&outpack[ivan], &xx->run_id,sizeof(xx->run_id));
  ivan+= sizeof(xx->run_id);
  memmove(&outpack[ivan], &xx->key_id,sizeof(xx->key_id));
  ivan+= sizeof(xx->key_id);
  memmove(&outpack[ivan], &xx->counter,sizeof(xx->counter));
  ivan+= sizeof(xx->counter);
  
  icp->icmp_cksum = in_cksum((unsigned short *)icp,cc);
  
  gettimeofday(&start, NULL);
  
  
  i = sendto(sss, (char *)outpack, cc , 0, (struct sockaddr*)&to, (socklen_t)sizeof(struct sockaddr_in));//independent of icmp checksum..
  if (i < 0 || i != cc)
    {
      if (i < 0)
	perror("sendto error");
      //cout << "wrote " << hostname << " " <<  cc << " chars, ret= " << i << endl;
    }
  /*	
  // Watch stdin (fd 0) to see when it has input.
  FD_ZERO(&rfds);
  FD_SET(s, &rfds);
  // Wait up to one seconds.
  tv.tv_sec = 1;
  tv.tv_usec = 0;
  
  while(cont)
  {
  retval = select(s+1, &rfds, NULL, NULL, &tv);
  if (retval == -1)
  {
  perror("select()");
  return -1;
  }
  else if (retval)
  {
  fromlen = sizeof(sockaddr_in);
  if ( (ret = recvfrom(s, (char *)packet, packlen, 0,(struct sockaddr *)&from, (socklen_t*)&fromlen)) < 0)
  {
  perror("recvfrom error");
  return -1;
  }
  
  // Check the IP header
  ip = (struct ip *)((char*)packet); 
  hlen = sizeof( struct ip ); 
  if (ret < (hlen + ICMP_MINLEN)) 
  { 
  cerr << "packet too short (" << ret  << " bytes) from " << hostname << endl;;
  return -1; 
  } 
  
  // Now the ICMP part 
  icp = (struct icmp *)(packet + hlen); 
  if (icp->icmp_type == ICMP_ECHOREPLY)
  {
  cout << "Recv: echo reply"<< endl;
  if (icp->icmp_seq != 12345)
  {
  cout << "received sequence # " << icp->icmp_seq << endl;
  continue;
  }
  if (icp->icmp_id != getpid())
  {
  cout << "received id " << icp->icmp_id << endl;
  continue;
  }
  cont = false;
  }
  else
  {
  cout << "Recv: not an echo reply" << endl;
  continue;
  }
  
  gettimeofday(&end, NULL);
  end_t = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
  
  if(end_t < 1)
  end_t = 1;
  
  cout << "Elapsed time = " << end_t << " usec" << endl;
  return end_t;
  }
  else
  {
  cout << "No data within one seconds.\n";
  return 0;
  }
  }*/
  return 0;
}

uint16_t in_cksum(uint16_t *addr, unsigned len)
{
  uint16_t answer = 0;
  /*
   * Our algorithm is simple, using a 32 bit accumulator (sum), we add
   * sequential 16 bit words to it, and at the end, fold back all the
   * carry bits from the top 16 bits into the lower 16 bits.
   */
  uint32_t sum = 0;
  while (len > 1)  {
    sum += *addr++;
    len -= 2;
  }

  // mop up an odd byte, if necessary
  if (len == 1) {
    *(unsigned char *)&answer = *(unsigned char *)addr ;
    sum += answer;
  }

  // add back carry outs from top 16 bits to low 16 bits
  sum = (sum >> 16) + (sum & 0xffff); // add high 16 to low 16
  sum += (sum >> 16); // add carry
  answer = ~sum; // truncate to 16 bits
  return answer;
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
//  printf("<uPause(%g)\n", noUsec);
}


int main(int argc, char** argv)
{
char *serverName=0;
  u_int32_t exp_id,run_id,key_id;
  exp_id=run_id=key_id=0;

 int option_index,op,sd, rc, hflag,reqFlag;
  double waittime,waittime1,linkCapacity,sleepTime,sleepTime1;
struct hostent *h;
 char psd, wtd;  
  psd='d';
  wtd='d';
int runType;

double runPkts, runPkts_1;
int size,noBreak,size1;
int difference_size,sample_length;
	

	/*if (argc != 2)
	{
		cout << "Usage: ping hostname" << endl;
		exit(1);
	} */


  static struct option long_options[] =  {
	{"expid ",required_argument,0,'e'},
	{"keyid ",required_argument,0,'r'},
	{"runid ",required_argument,0,'k'},
	{"server",required_argument, 0, 's'},
	{"pkts", required_argument,0,'n'},
	{"pktdist", required_argument,0,'m'},	
	{"pktLenmin", required_argument, 0, 'l'},
        {"pktLenMax", required_argument, 0, 'L'},
	{"waitdist", required_argument, 0, 'v'},
	{"waittimemin", required_argument, 0, 'w'},
	{"waittimemax", required_argument, 0, 'W'},
	{"samplelength", required_argument, 0, 'z'},
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
  sleepTime=-1;

  waittime=0;  
  while ( (op =getopt_long(argc, argv, "k:e:r:s:p:n:m:l:L:v:w:W:z:h",long_options, &option_index))!=EOF) {
    switch (op){
    case 'e':/*exp_id*/
      exp_id=(u_int32_t)atoi(optarg);
      cout <<exp_id <<"\n";
      reqFlag++;
      break;
    case 'r':/*exp_id*/
      run_id=(u_int32_t)atoi(optarg);
      reqFlag++;
      cout <<run_id <<"\n";
      break;
    case 'k':/*key_id*/
      key_id=(u_int32_t)atoi(optarg);
      cout<<key_id <<"\n";
      reqFlag++;
      break;
    case 's': /* Server */
      serverName=optarg;
      cout <<serverName <<"\n";
      reqFlag++;
      break;
    case 'n': /* number of pkts */
      runPkts=atof(optarg);
      cout <<runPkts <<"\n";
      runType=1;
      break;
      break;
    case 'm': /*pkt size distribution*/
      psd=*optarg;
      cout <<" PSD is"<<psd <<"\n" ;
      break;
    case 'l': /*pkt length maxima*/
      size=atoi(optarg);
      cout<< "min packet size Size is "<<size <<"\n";
      break;
   case 'L': /*pkt length maxima*/
      size1=atoi(optarg);
      cout<< "Max packet Size is "<<size1 <<"\n";
      break;


    case 'v': /*wait time distribution*/
	wtd=*optarg;
       cout<<"distribution is "<<wtd<<"\n";
	break;
    case 'w': /*wait time maxima*/
      sleepTime=atoi(optarg);
       cout<<"sleeptime is "<<sleepTime<<"\n";
      waittime=sleepTime;
      reqFlag=4;
      break;
    case 'W': /*wait time maxima*/
      sleepTime1=atoi(optarg);
       cout<<"sleeptime Max is "<<sleepTime1<<"\n";
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
      printf(" -n (--pkts) <Number of packets to send> [optional default = forever]\n");
      printf(" -l (--pktLen) <Packet Length> [bytes] [optional default = 1224]\n\n");
      printf(" -m (--pktsize distribution) e- exponential u- uniform d- discrete uniform default- deterministic\n\n");
      printf(" -w (--waittime) <Inter frame gap, in usec.> [optional, but if set, voids desired]\n");
      printf(" -v (--wait time distribution) e- exponential u- uniform d- discrete uniform default- deterministic\n\n");
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
 printf ("Hello world\n");
    printf("%s: unknown host '%s' \n", argv[0], argv[1]);
    exit(1);
  }


  RND* myRND1;// packet size distribution
  RND* myRND2; // wait time distribution
  switch(psd){
	case 'e':
		printf("Packet size is Expontial...\n");
		myRND1=new RNDEXP(size1);
	break;
	case 'u':
		printf("Packet size Uniform...\n");
		myRND1=new RNDUNIF(size,size1);
	break;

	case 'd':
               printf ("Packet size uniform discrete\n");
                myRND1 = new RNDUNID(size,size1);
               break;
	default:
	printf("Packet size DEfault is to deterministic \n");
		myRND1=new RNDDET(size1);
	 	break;
  }

  switch(wtd){
	case 'e':
		printf("waittime isExpontial...\n");
		myRND2=new RNDEXP(waittime1);
	break;
	case 'u':
		printf("wait time is Uniform...\n");
		myRND2=new RNDUNIF(waittime,waittime1);
	break;

	case 'd':
         printf("wait time uniform discrete\n");
		myRND2=new RNDUNID(waittime,waittime1);
          break;
	default:
	printf("wait time DEfaults to determ\n");
		myRND2=new RNDDET(waittime1);
	 	break;
  }
(*myRND1).printseed();
(*myRND2).printseed();

 
        double di; // Di is num of run times..
u_int32_t counter1;
int si;
if ( (si = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
	{
		perror("socket");	/* probably not running as superuser */
		return -1;
	}
difference_size = size1 -size;
  runPkts_1 = floor (((difference_size)*runPkts)/sample_length) + runPkts;
printf("will run %g pkts for each size.\n",runPkts);
	cout <<" Experiment will run an overall of " << runPkts_1 <<"samples";
 while (di < runPkts_1)
	 {
//size=int(myRND1->Rnd());
//cout << size<<"packet size\n";
waittime = double (myRND2->Rnd());
//cout<<waittime<<" wait time in microseconds\n";

di++;
if (int (di) %(int)runPkts == 0)
	{
	size = size + sample_length;
	//sleep(1);
	}
      
      if(int(di)%1000==0) {
	cout << di << " pkts." <<endl;
      }
//cout <<"Packet number  "<<di<<" out of runPkts "<<runPkts<<"\n";// send di..
counter1 = (u_int32_t) di;
ping (si,exp_id, run_id, key_id,serverName,counter1,size);
uPause(waittime); 

}
close(si);
	return 0;
}
