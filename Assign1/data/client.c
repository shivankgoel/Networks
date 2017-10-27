//Used References :: http://www.linuxhowtos.org/C_C++/socket.htm

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

void error(const char *e)
{
    perror(e);
    exit(0);
}

struct data
{
	int rc;
	int id;
	time_t time;
	
}mydata;

int main(int argc, char *argv[])
{
   int sock, n;
   //stores size of server of type sockaddr_in 
   unsigned int length;

   //sockaddr_in is a structure used to store socket addresses. server will store address of server. from will be used to receive ack 
   struct sockaddr_in server, from; 
   //hostent is a class that stores host details like host name, address type(unix,internet),address length and address itself 
   struct hostent *hp;
   
   // client has to pass 3 arguments to , if it hasn't passed than return error
   if (argc < 4) { printf("Give 5 arguments HostPCName Port T P OutputFile\n");
                    exit(1);
   }
   int port = atoi(argv[2]);

   //socket() creates a new socket. AF_INET specify socket adress type is internet.Socket type is datagram.0 will choose UDP for datagram.
   sock= socket(AF_INET, SOCK_DGRAM, 0);
   // sock is a file descriptor. That serves as a pointer in table storing input/output streams. 		
   if (sock < 0) error("socket");

   // sockaddr_in structure has fields family:type of sock addr , port , address   
   server.sin_family = AF_INET;
   
   // returns a hostent object which has info oabout host 
   hp = gethostbyname(argv[1]);
   if (hp==0) error("Unknown host");
   // copy host address into server address
   bcopy((char *)hp->h_addr, (char *)&server.sin_addr,hp->h_length);
   
   // server port is received from command line args. Now we have all info about server. htons converts into network big endian.
   server.sin_port = htons(port);
   length=sizeof(struct sockaddr_in);

   int T = atoi(argv[3]);
   int P = atoi(argv[4]);
   mydata.id = 0;
   mydata.time = -1;
 
   FILE *ofp;
   
   if(ofp ==NULL )ofp = fopen(argv[5],"w");
   if(ofp!= NULL)fprintf(ofp,"For T: %d and P: %d\n",T, P);
   
   for(int iter=0;iter<50;iter++){
        mydata.rc = T;
   	mydata.time = clock();
        mydata.id = mydata.id + 1;
	   
           while(mydata.rc > 0){
	   
	   // while sending we have to provide a socket:sock , and sockaddr_in of server 
	   n=sendto(sock,&mydata,P,0,(const struct sockaddr *)&server,length);
	   if (n < 0) error("Sendto");

	   // while receiving we have to provide socket:sock , and a sockaddr_in from that will be storing info about who sent data
	   n = recvfrom(sock,&mydata,P,0,(struct sockaddr *)&from, &length);
	   if (n < 0) error("recvfrom");
	   mydata.rc =mydata.rc - 1;
	   }

	time_t endtime = clock();
	
	
	if(ofp != NULL){
             fprintf(ofp, "ID: %d RTT: %d\n",mydata.id, (int)(endtime - mydata.time));
				
	}else{
		 fprintf(stderr, "Can't open output file %s!\n",argv[5]);
                 printf("ID: %d RTT: %d\n",mydata.id, (int)(endtime - mydata.time));
	} 
    }

   if(ofp!=NULL)fclose(ofp);
   close(sock);
   return 0;
}

