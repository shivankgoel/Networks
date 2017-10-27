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
#include <pthread.h>
#include <stdbool.h>

#define min(a,b)(a<b?a:b) 

time_t start_time[8334]; //This stores start time of packets
int MSS = 1000;  //Maz data size that can be sent
int W = 1000;   //Initial Window Size
int sent = 0;   //Number of bytes sent successfully
int ack = 0;    //Variable to store last acknowledgement
int flow = 100000; //total data to be sent
int pkt_size = 1000;  //initial pkt size
int seq_no = 0;      //seq number we are sending
int temp_w = 1000;   //window size that is still empty
int start=0,end =0;  //end-start tells us number of packets we have sent
bool binni = true;   //variable for debugging
time_t timer1;       // to store initial time when program starts
int mode=0;          // to store 0,1 mode

//Sock to check if socket is made
int mysocket;
//End 

//Set server address ip and port
struct sockaddr_in serverAddr,clientAddr;
int port,lengthAddr;
//End


//generate error message and exit
void error(const char *e)
{
    perror(e);
    exit(0);
}

//send packet on separate thread other than receivethread
//method to use if packet is not being dropped in between sender and receiver
void *sendthread1(void* x){
  
  int sendNo;

  while(sent<flow){
    
    //size of packet to be sent
    pkt_size = min(min(temp_w,MSS),flow-seq_no);

    if(pkt_size>0){
    //put seq_no and size of packet and packet data into container "data"
    int data[pkt_size/4 + 2];
    data[0] = seq_no;
    data[1] = pkt_size; 

    start_time[end] = clock();

    binni = false;
    temp_w -= pkt_size; //decrease the space remaining in window
    seq_no += pkt_size; //update seq number for next packet
    end += 1;
    printf("%s %d %s %d %s %d\n","W= ",W,"Time= ",(int)(clock()-timer1),"Seq No= ",(seq_no - pkt_size));
    //send the packet to receiver without dropping in between
    sendNo=sendto(mysocket,&data,sizeof(data),0,(const struct sockaddr *)&serverAddr,lengthAddr);
    if (sendNo < 0) {
      error("Sendto");
      temp_w += pkt_size;
      seq_no -= pkt_size; 
      end -= 1;
    }
    binni = true;

    }
  }
  
}

//method to use if packet is not being dropped in between sender and receiver
void *sendthread2(void* x){
  
  int sendNo;

  while(sent<flow){
    int r;
    //size of packet to be sent
    pkt_size = min(min(temp_w,MSS),flow-seq_no);

    if(pkt_size>0){
    //put seq_no and size of packet and packet data into container "data"
    int data[pkt_size/4 + 2];
    data[0] = seq_no;
    data[1] = pkt_size; 

    start_time[end] = clock();

    binni = false;
    temp_w -= pkt_size; //decrease the space remaining in window
    seq_no += pkt_size; //update seq number for next packet
    end += 1;
    printf("%s %d %s %d %s %d\n","W= ",W,"Time= ",(int)(clock()-timer1),"Seq No= ",(seq_no - pkt_size));
    //generate a random number between 1 and 100 inclusive
    r=rand()%100 + 1;
    //probability of dropping packet is 0.05
    if(r%20 != 0){
      sendNo=sendto(mysocket,&data,sizeof(data),0,(const struct sockaddr *)&serverAddr,lengthAddr);
      if (sendNo < 0) {
        error("Sendto");
        temp_w += pkt_size;
        seq_no -= pkt_size; 
        end -= 1;
      }
    }
    binni = true;

    }
  } 
}
//receive packet on separate thread
void *receivethread(void* x){

  int recvNo;
  while(ack<flow){

    if(binni){
      //wait till packet arrives
      recvNo = recvfrom(mysocket,&ack,sizeof(int),0,(struct sockaddr *)&clientAddr, &lengthAddr);
      if (recvNo < 0) error("recvfrom");

      if(ack == -1){
        //Do nothing
      }
      //If correct ack is received
      else if(ack>sent && ack-sent<=1000){
        int W_old = W;
        W += (MSS*MSS)/W_old;
        temp_w += (MSS*MSS)/W_old;
        temp_w += ack - sent;
        sent = ack;
        start += 1;  
      }
      //if receiver is demanding an earlier sent packet
      else if(ack == sent){
        //printf("%s %d\n", "Data Was Lost Sending Again" ,ack);
        W = MSS;
        temp_w = MSS;
        seq_no = sent;
        end = start;
      }
      printf("%s %d %s %d %s %d\n","W= ",W,"Time= ",(int)(clock()-timer1),"Ack= ", ack);
    //}
    }

  }
  //close the socket
  close(mysocket);
   
}

//separate thead to keep track of timeout for already sent packets
void *timeout_thread(void* x){
  
  while(sent<flow || ack<flow){

    for(int j = start ; j <end ;j++){
      //if timeout decrease the window size to  1 MSS and 
      //start sending the packet from byte next to already sent bytes
      if((int)(clock() - start_time[j]) > 10000 && sent!=ack ){
        //PacketDrop 
        W = MSS;
        temp_w = MSS;
        seq_no = sent;
        end = start;
        // printf("%s %d \n", "Timeout For Packet" , seq_no );
        
      }
    } 

  }

}


int main(int argc, char *argv[]){

  if (argc < 2) { 
   	printf("Give IpAdress and Port\n");
    exit(1);
  }
  //if -l flag is given
  else if(argc == 4){
    mode = atoi(argv[4]);
  }
  
  //Sock to check if socket is made
  mysocket= socket(AF_INET, SOCK_DGRAM, 0);
  if (mysocket < 0) error("socket");
  //End

  //Set server address ip and port
  memset((char*)&serverAddr,'\0',sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  port = atoi(argv[2]);  
  serverAddr.sin_port = htons(port);
  lengthAddr=sizeof(struct sockaddr_in);
  //End

  timer1 = clock();
  //thread IDs for different threads
  pthread_t tid1,tid2,tid3;
  //thread for sending packets
  if(mode==0){
    pthread_create(&tid1,NULL,sendthread1,NULL);
  }
  else{
    pthread_create(&tid1,NULL,sendthread2,NULL); 
  }
  //thread for receiving packets
  pthread_create(&tid2,NULL,receivethread,NULL);
  //thread to keep track of timeouts for packets which are sent but still ack has not arrived
  pthread_create(&tid3,NULL,timeout_thread,NULL);
  //exit the threads
  pthread_exit(NULL);
  return 0;
}

