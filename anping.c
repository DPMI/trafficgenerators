/* 
 * anping 1.3
 *
 * ...is a simple ping programm, that
 * sends icmp packets to a host to see
 * whether it is up or not
 * 
 * anping works with raw sockets, so you
 * need to be root
 *
 * compiling:   gcc anping.c -o anping
 * usage:       [root@localhost home]# ./anping
 * 
 * coded by anfeu (gsicht) 06.11.2004
 * anfeu@lycos.de
 * 
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>

#define ANSWER_TIMEOUT 3
#define WHITE  "[0m"
#define RED  "[1;31m"
#define GREEN   "[1;32m"

void printerr(char *message)
{
        printf("%sERROR: %s%s\n",RED,message,WHITE);
}

void usage(char *prog)
{
        printf("\n%sUSAGE:%s\n",GREEN,WHITE);
        printf("%s -s [source ip] -d [target host] -t <timeout> -l Plen -n seqnr\n",prog);
        printf(" -d [host]\tdest host\n");
        printf(" -p [port]\tport to connect\n");
        printf(" -t [seconds]\ttimeout for answer\n");
        printf(" -c [packet number]\thow much packets?\n");
       printf(" -l [packet length]\tpacket size?\n");
       printf(" -n [Sequence nr to use]\thow much packets?\n");
  
 
       printf("example: %s -s 80.80.80.80 -d 80.61.54.22 -t 1 -c 5\n\n",prog);
        exit(0);
}

void sigfunc(int sig)
{
        char c;

        if(sig != SIGINT)
        return;
        else
        {
  printf("\n--interrupt--\n");
  sleep(1);
        }
        exit(0);
}


// i found this function somewhere in the internet
unsigned short in_cksum(unsigned short *ptr, int nbytes)
{
        register long sum;
        u_short oddbyte;
        register u_short answer;
 
        sum = 0;
        while(nbytes > 1)
        {
  sum += *ptr++;
  nbytes -= 2;
        }
 
        if(nbytes == 1)
        {
  oddbyte = 0;
  *((u_char *) &oddbyte) = *(u_char *)ptr;
  sum += oddbyte;
        }
 
        sum  = (sum >> 16) + (sum & 0xffff);
        sum += (sum >> 16);
        answer = ~sum;

        return(answer);
}
////


int timeval_subtract (struct timeval *result, struct timeval *x, struct timeval*y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}



int main(int argc, char *argv[])
{
        
        signal(SIGINT,sigfunc);

        int sock,test=1,num;
        int recvd = 0,sent = 0;
        int i,n,c,timeout;

        struct iphdr *ip;
        struct icmphdr *icmp;

        struct sockaddr_in addr;
        struct in_addr addr2;

        fd_set fdreadme;

        char *packet;
//        char *data = "PING";
        char *data = "PING lksjdfhlkjsdhfuiwehfkjbvlwuebvkawjbvluisebf,awkjbv,iauwrbv,awkvbawiuvhlawirfbawluvhzlsiurbva,wekruvawliuuawlrubvwalurbvawubvluaibveiarvbaeliuvhaiurvhaeiruvhaeliuvhlriuvhsleiurhwlvituhwrituverhliguqerhltuihaelriug3hqguhsiugh4uitheo98ygw8y t3h9a84ytw9p83eyrgbp98a34ybå08934yatn9psawe4ypöt98ysaworn9t78yw3580yu ba3p489ntyw3b0589yws3n978typöw308957pty9a8wythoösedirgujp9w8374ny6b0835uy å80aye5pt89gy 3v589t 73e5pr g983y5t9 8w3y5tp 9a735ytp978q3v5ytp 9a78erytpa3w5 ytöqb83a5yö89aeyrgo87setyo5 79tuwh3l589tuyse89rgysaowe479 tya9pw48y vp29a48yt pa9w78rgy 359y8t wb9845yg se985n vt795y8tp98e57typ9w385 yu6t98w4seyg78s4etyo9se4y858vw4 v6985y to9q83y5to9 38q5yt 9q835yt9q358tywse89yg9s8ey5htm9s8eyg9s8eyrt98se5yt b9s8e4yt np9s8wey4t9pw384yt89se7rt98w3y5tpb98w3y5t89p syerp9gyw35öto9hse8ghs89ghe59p hvt35iuht p9se8rgy9we78ty935ohylw34iguzose89rtyw3o45uithpe89rgyse958yti8sy3ept9v8q3y ö4ot8ypw98typw3498yvpw398ytpseio8gyp9w358ytper8guy3pwq948tygseop789tyw3p98t yep9t8 y3vhqpöe89ghp3985thp9e8aygp938ehpryg9w34h8vt 8oöahwerpg9ws73 yhö4t98s ehgöp489eyb pse98uh g39pawe8ygh9xödor5hyöwuihspe58yöw3se9ugå59yps03 u95yp0se9ub9pse8ugow83eubpw9348uy bpm9385uyg 9pw348yu gp9e48y5gps98etygpw893v5hy 98se ypbv9es4y5hgpi8suygp984wy5hbiurhlbixduygpiuq3hwelib8she5lögouihqaelpr98ghwl4obhrtlkbjhezdovuhbedfbvdfhgkejhgktjhgkehgksehrlghedig78eligyh5kjghekjfhgil8eyt vo9735y to938ytiu shdfiugh3 5 lig8hserogl9u35hg89sh5l3gu8 hseo9vghdklvhi8seuyt89w3yhlgsejhd viluezhv o978z8eyryg9s 8nglis3ae5htiusahvo978rtyw l9i85r7y tw89734syeg87segh rukfythy.";
        char buffer[512];

       uint16_t ICMP_IDnr=1;
       uint16_t ICMP_seq=0;
      int icmplen=64;
      int k=0;
        char *hostname;
        char *sip;
        struct hostent *host;
      struct timeval before, after;


        for(i=0;i<8;i++)
        {
  if(argv[i] == NULL)
  usage(argv[0]);
//  if((strcmp(argv[i],"-h") != NULL) || (strcmp(argv[i],"--help") != NULL))
//  usage(argv[0]);
        }

        

        while((c = getopt(argc, argv, "s:d:t:c:l:n:"))!= EOF)
        {
  switch (c)
  {
         case 's':
         sip = optarg;
         if(sip == NULL)
         {
    printerr("i don't know your ip address");
    usage(argv[0]);
         }       
         break;

	case 'n': 
	  ICMP_seq=atoi(optarg);
	  break;
	case 'l':
	  icmplen=atoi(optarg);
	  break;

         case 'd':
         hostname = optarg;
         if(hostname == NULL)
         {
    printerr("do this: -d dest host");
    usage(argv[0]);
         }
         break;

         case 't':
         if(optarg == NULL)
         timeout = ANSWER_TIMEOUT;
         timeout = atoi(optarg);
         break;

         case 'c':
         num = 3;
         if(optarg != NULL)
         num = atoi(optarg);
         break;

         default:
         usage(argv[0]);
         return 1;
  }
        }

        ip = (struct iphdr *) malloc(sizeof(struct iphdr));
        icmp = (struct icmphdr *) malloc(sizeof(struct icmphdr));
        packet = (char *) malloc(sizeof(struct iphdr) + sizeof(struct icmphdr) + sizeof(data) + 1);

        memset(packet,0x00,sizeof(packet));

        ip = (struct iphdr *)packet;
        icmp = (struct icmphdr *) (packet + sizeof(struct iphdr));
        strcpy(packet+sizeof(struct iphdr)+sizeof(struct icmphdr),data);

        printf("\nsending ICMP packets from %s ..\n\n",sip);

        ip->ihl = 5;
        ip->version = 4;
        ip->tos = 0;
        ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr) + icmplen + 1;
        ip->id = htons(getuid());
        ip->ttl = 255;
        ip->protocol = IPPROTO_ICMP;
        ip->saddr = inet_addr(sip);
        ip->daddr = inet_addr(hostname);
        ip->check = in_cksum((unsigned short *)ip, sizeof(struct iphdr));

        sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
        if(sock == -1)
        {
  printerr("couldn't creat socket");
  return -1;
        }

        if(setsockopt(sock, IPPROTO_IP,IP_HDRINCL,&test,sizeof(test)) < 0)
        {
  printerr("couldn't set IP_HDRINCL");
  return -1;
        }
        
        icmp->type = ICMP_ECHO;
        icmp->code = 0;
         icmp->un.echo.id = htons(ICMP_IDnr);
        icmp->un.echo.sequence = htons(ICMP_seq);
        icmp->checksum = in_cksum((unsigned short *)icmp,sizeof(struct icmphdr)+icmplen+1); 

        addr.sin_family = AF_INET;
        addr.sin_port = htons(31337);
        addr.sin_addr.s_addr = inet_addr(hostname);

        printf("starting...\n");

	  gettimeofday(&before,NULL);
        for(i=0;i<num;i++)
        {
	  //fprintf(stdout,"PING...");
	  printf("[%d/%d],%d.%06ld : %d bytes : ", i, num,(int)before.tv_sec, before.tv_usec, ip->tot_len );
	  if((sendto(sock,packet,ip->tot_len,0,(struct sockaddr *)&addr,sizeof(struct sockaddr))) < ip->tot_len)
	  {
		printerr("couldn't send the packet");
	  } else {
	    sent++;
	  }
	  struct timeval tv;
	  tv.tv_usec = 0;
	  tv.tv_sec = timeout;

	  FD_SET(sock, &fdreadme);
	  FD_SET(0, &fdreadme);

	  if(select(sock+1, &fdreadme, NULL, NULL, &tv) < 0 )
	  {
		printerr("select() error");
		return -1;
	  }

	  if(FD_ISSET(sock, &fdreadme)) 
	  {
		if(n = recv(sock, buffer, sizeof(buffer), 0) > 0)
		{
		  gettimeofday(&after,NULL);
		  addr2.s_addr = ip->daddr;
		  printf("received ICMP packet from %s ",inet_ntoa(addr2));
		  struct timeval elapsed;
		  k=timeval_subtract(&elapsed,&after,&before);
		    
		  printf("%d.%06ld : %d bytes : ", (int)before.tv_sec, before.tv_usec, ip->tot_len );
		  printf("Stop:%d.%06ld ", (int)after.tv_sec, after.tv_usec);
		  printf(" %d.%06ld s\n", (int)elapsed.tv_sec, elapsed.tv_usec);	

		  recvd++;
		}
		if(n == -1)
		printerr("recv() failed");
	  }
	  else{
	      printf("-1 \n");
	      printerr("didn't receive answer. host seems down");
	  }
        }
       printf("\n-------- stats --------\n");
       printf("%d packets transmitted, %d received\n...and the rest is silence\n",sent,recvd);
        return 0;
}
/**** EOF ****/
