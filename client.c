#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <string.h>

/* ping-pong client, Kai Wu's version 1.2. takes four parameters, the server host name,
 the server port number, size of message to send and No. of message exchanges to perform
 
 ->
 1. hostname
 The host where the server is running. You should support connecting to a server by domain name (current list of CLEAR servers: ring.clear.rice.edu, sky.clear.rice.edu, glass.clear.rice.edu, water.clear.rice.edu).
 2. port
 The port on which the server is running (on CLEAR, the usable range is 18000 <= port <= 18200).
 3. inputSize
 The size in bytes of each message to send (10 <= size <= 65,535).
 4. inputCount
 The number of message exchanges to perform (1 <= count <= 10,000).*/

int main(int argc, char** argv) {

  /* our client socket */
  int sock;

  /* address structure for identifying the server */
  struct sockaddr_in sin;

  /* convert server domain name to IP address */
  struct hostent *host = gethostbyname(argv[1]);
  unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

  /* server port number */
  unsigned short server_port = atoi (argv[2]);

    char *buffer, *sendbuffer, *stdinBuffer, *sendbufferpointer;
  struct timeval time; //get the current time
    char* inputSizecheck = argv[3];
    int inputSizecheckint = atoi(argv[3]);
    unsigned short inputSize = atoi(argv[3]); //The size in bytes of each message to send (10 <= size <= 65,535).
    int inputCount = atoi(argv[4]); // The number of message exchanges to perform (1 <= count <= 10,000)
    int messagesentSEC = 0, messagesentUSEC= 0; //the time message sent;
    int messagegetSEC= 0, messagegetUSEC= 0;//the time message receive;
    int durationSEC= 0, durationUSEC= 0; //the time data traveled
    int totalLatencySEC= 0, totalLatencyUSEC= 0; //the total latency
    float avgLatencySEC = 0;
    float avgLatencyUSEC = 0; //the avg. latency
    int result; int remainer; //used in calculation the avg. latency
    int i,x,datasize,looptime,sendcount,returncount,u,alreadysend;
    
    u = strlen(inputSizecheck);
    /*validate the inputSize*/
    if (inputSize < 10 || inputSizecheck[0] == '-' || u > 5 || inputSizecheckint > 65535) {
        perror("the assigned size for each message to send should between 10 and 65535");
        abort();
    }
    
    /*validate the inputed message count*/
    if (inputCount < 1 || inputCount > 10000) {
        perror("the number of message exchanges to perform should between 1 and 10000");
        abort();
    }

  /* allocate a memory buffer in the heap */
  /* putting a buffer on the stack like:

         char buffer[500];

     leaves the potential for
     buffer overflow vulnerability */
  buffer = (char *) malloc(inputSize);
  if (!buffer)
    {
      perror("failed to allocated buffer");
      abort();
    }

  sendbuffer = (char *) malloc(inputSize);
  if (!sendbuffer)
    {
      perror("failed to allocated sendbuffer");
      abort();
    }


  /* create a socket */
  if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
      perror ("opening TCP socket");
      abort ();
    }

  /* fill in the server's address */
  memset (&sin, 0, sizeof (sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = server_addr;
  sin.sin_port = htons(server_port);

  /* connect to the server */
  if (connect(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
    {
      perror("connect to server failed");
      abort();
    }

  /* everything looks good, since we are expecting a
     message from the server in this example, let's try receiving a
     message from the socket. this call will block until some data
     has been received */
  //count = recv(sock, buffer, size, 0);
  /*if (count < 0)
    {
      perror("receive failure");
      abort();
    }
     */
  /* in this simple example, the message is a string, 
     we expect the last byte of the string to be 0, i.e. end of string */
 /* if (buffer[count-1] != 0)
    {
       In general, TCP recv can return any number of bytes, not
	 necessarily forming a complete message, so you need to
	 parse the input to see if a complete message has been received.
         if not, more calls to recv is needed to get a complete message.
      
      printf("Message incomplete, something is still being transmitted\n");
    } 
  else
    {
      printf("Here is what we got: %s", buffer);
    }
*/
    looptime = inputCount;
  while (looptime > 0){
      
       
      datasize = inputSize - 10;

      looptime = looptime - 1; //still have inputCount messages to send
      
      *(unsigned short*)sendbuffer = (unsigned short)htons(inputSize);//size at the beginning of the data package (int unsigned short)
    
      sendbufferpointer = sendbuffer;

       
      stdinBuffer = malloc(sizeof(char) * (datasize));
      
     
      for (x = 0; x < datasize; x++) {
          stdinBuffer[x] = 'a' + x%26;
          
      }
      
      gettimeofday(&time,NULL);//get the current time
      messagesentSEC = (int) time.tv_sec;
      messagesentUSEC = (int) time.tv_usec;
      *(int *) (sendbuffer+2) = (int) htonl(time.tv_sec);
      *(int *) (sendbuffer+6) = (int) htonl(time.tv_usec);
      for (i = 0; i < datasize; i++) {
           *(char *) (sendbuffer + 10 + i) = stdinBuffer[i];
      }
     
      alreadysend = 0;
      sendcount = send(sock, sendbufferpointer, inputSize, 0);
      while (alreadysend < sendcount) {
          while (alreadysend < sendcount) {// --------------------------!!!!!need to add functions for jump out this while loop if the client is recieving forever!!!!!-------
              returncount = recv(sock, buffer, inputSize, 0);
              printf("recieved: %d\n",returncount);
              alreadysend += returncount;
              sendbufferpointer += alreadysend;
          }
          if(alreadysend >= sendcount){
             printf("the sending process done!%d times left\n",looptime);
             break;
          }
          printf("Need to send more data!\n");
          send(sock, sendbuffer, inputSize-alreadysend, 0);
      }
      
      gettimeofday(&time,NULL);//get the current time
      messagegetSEC = (int)time.tv_sec;
      messagegetUSEC = (int) time.tv_usec;
      //calculate the latency
      if (messagegetUSEC >= messagesentUSEC) {
        durationSEC = messagegetSEC - messagesentSEC;
        durationUSEC = messagegetUSEC - messagesentUSEC;
        }else{
            durationSEC = messagegetSEC - messagesentSEC - 1;
            durationUSEC = 1000000 + messagegetUSEC - messagesentUSEC;
            }
              totalLatencySEC = totalLatencySEC + durationSEC;
              totalLatencyUSEC = totalLatencyUSEC + durationUSEC;
          
          
      
      
      
      //clear data
      messagegetSEC = 0;
      messagegetUSEC = 0;
      messagesentSEC = 0;
      messagesentUSEC = 0;
      durationSEC = 0;
      durationUSEC = 0;
  }
    
    
    result = 0;
    remainer = 0;
    //regular the totalatency value
    if (totalLatencyUSEC >= 1000000) {
        totalLatencySEC = totalLatencyUSEC/1000000 + totalLatencySEC;
        totalLatencyUSEC = totalLatencyUSEC%1000000;
    }
    //calculate the avg. latency
    result = totalLatencySEC/inputCount;
    remainer = totalLatencySEC%inputCount;
    avgLatencySEC = result*1.000; //seperate the sec and usec part to avoid overflow dangers.
    avgLatencyUSEC = (remainer*1000000.000 + totalLatencyUSEC*1.000)/(inputCount*1.000);
   
    printf("Average latency is :%0.3f microseconds\n", avgLatencyUSEC + avgLatencySEC*1000000);
    return 0;
}
