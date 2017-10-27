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

//generate error message and exit
void error(const char *msg)
{
    perror(msg);
    exit(0);
}
//packet type
struct packet
{
  int seqno;
  int size;
  char* data;
  
}mydata;

int main(int argc, char *argv[])
{

  if (argc < 1) {
      fprintf(stderr, "Please provide PortNumber\n");
      exit(0);
  }
  
  //Sock to check if socket is made
  int mysocket;
  mysocket= socket(AF_INET, SOCK_DGRAM, 0);
  if (mysocket < 0) error("socket");
  int optval = 1;
  setsockopt(mysocket,SOL_SOCKET,SO_REUSEADDR,(const void*)&optval,sizeof(int));
  //End 

  //Set server address ip and port
  struct sockaddr_in serverAddr,clientAddr;
  memset((char*)&serverAddr,'\0',sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  int port = atoi(argv[1]);  
  serverAddr.sin_port = htons(port);
  int lengthAddr=sizeof(struct sockaddr_in);
  serverAddr.sin_addr.s_addr=INADDR_ANY;
  //End

  //Bind socket with serveradrr(ip+port)
  int bindcheck;
  bindcheck= bind(mysocket,(struct sockaddr*)&serverAddr,sizeof(struct sockaddr_in));
  if (bindcheck < 0) error("Cant Bind");
  //End

  //Select Function
  fd_set readfds;
  FD_SET( mysocket , &readfds);
  struct timeval tv;
  tv.tv_sec=0;
  tv.tv_usec=1;
  //End
  int data[252];
  int ack=0;
  int sendNo,recvNo;
  int seqno,size;    
  while (1) {
      //Receive packet sent by sender
      recvNo = recvfrom(mysocket,&data,sizeof(252 * sizeof(int)),0,(struct sockaddr *)&clientAddr, &lengthAddr);
      if (recvNo < 0) error("recvfrom");
      seqno=data[0];
      size = data[1];
      //Tell the sender that getting duplicate packets
      if(seqno < ack){

        printf("%s %d \n", "Already Existing Packet!!!" ,seqno);
        int temp =-1;
        sendNo=sendto(mysocket,&temp,sizeof(int),0,(const struct sockaddr *)&clientAddr,lengthAddr);
        if (sendNo < 0) error("Sendto");
        printf("%s %d\n", "Ack Sent To Tell Reqd Seq No", -1 );
        //
      }
      //Send ack for received packet
      else if(seqno == ack)
      {
      	ack += size;	
      	printf("%s %d \n", "New Packet Received Succesfully " ,seqno);
        sendNo=sendto(mysocket,&ack,sizeof(int),0,(const struct sockaddr *)&clientAddr,lengthAddr);
        if (sendNo < 0) error("Sendto");
        printf("%s %d\n", "Ack Sent for new packet ", ack );
    
      }
      //Ask for missing packets
      else
      {
        printf("%s %d \n", "In Between Packet Missed**!" ,seqno);
        sendNo=sendto(mysocket,&ack,sizeof(int),0,(const struct sockaddr *)&clientAddr,lengthAddr);
        if (sendNo < 0) error("Sendto");
        printf("%s %d\n", "Reqd Seq No", ack );
      }
      
      
    //}

     
  }
  
  //close the socket
  close(mysocket);
  return 0;

}

