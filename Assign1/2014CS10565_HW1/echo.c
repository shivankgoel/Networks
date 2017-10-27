//Used References :: http://www.linuxhowtos.org/C_C++/socket.htm

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>

void error(const char *msg)
{
    perror(msg);
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
   int sock, length, n;         //sock:pointer to new socket - a file descriptor , length : length of sockaddr_in  
   socklen_t fromlen;            // length of sockaddr_in but type is not int type is socklen_t , used for receiving data
   struct sockaddr_in server;   //to bind socket to this server 
   struct sockaddr_in from;     // to receive data
   
   if (argc < 3) {
      fprintf(stderr, "Please provide PortNumber and Packet Size as argument \n");
      exit(0);
   }
   int port = atoi(argv[1]);
   int P = atoi(argv[2]);

   //create a new socket 
   sock=socket(AF_INET, SOCK_DGRAM, 0);
   if (sock < 0) error("Cant open socket");

   // this length will be used to bind sock with server
   length = sizeof(server);
   // store all information of server in server , ADD Type , Port , Address
   bzero(&server,length);
   //  Address type is internet
   server.sin_family=AF_INET;
   //  Serever address is address of current machine which we get by INADDR_ANY
   server.sin_addr.s_addr=INADDR_ANY;
   // server port is received as input
   server.sin_port=htons(port);
   // We try to bind socket:sock to this server with above address and port. If port is not already in use it will bind.
   if (bind(sock,(struct sockaddr *)&server,length)<0)  error("binding");
   
   
   fromlen = sizeof(struct sockaddr_in);
   
   while (1) {

       // wait until it receive a datagram
       n = recvfrom(sock,&mydata,P,0,(struct sockaddr *)&from,&fromlen);
       if (n < 0) error("Cant receive from client");
       
       //printf("RC: %d ID: %d TIME: %d\n",mydata.rc,mydata.id,(int)mydata.time);
       
       mydata.rc = mydata.rc - 1;
       
       // sends back 
       n = sendto(sock,&mydata,P,0,(struct sockaddr *)&from,fromlen);
       if (n  < 0) error("Cant send back to client");
       
       if(mydata.id == 50 && mydata.rc == 1)break;

   }
   printf("Task Completed , Server Shutting Down.");
   close(sock);
   return 0;
 }

