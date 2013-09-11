#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* simple client, takes two parameters, the server domain name,
   and the server port number */

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

  char *buffer;
  int size = 500;
  int count;

  /* allocate a memory buffer in the heap */
  /* putting a buffer on the stack like:

         char buffer[500];

     leaves the potential for
     buffer overflow vulnerability */
  buffer = (char *) malloc(size);
  if (!buffer)
    {
      perror("failed to allocated buffer");
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
  count = recv(sock, buffer, size, 0);
  if (count < 0)
    {
      perror("receive failure");
      abort();
    }

  /* in this simple example, the message is a string, 
     we expect the last byte of the string to be 0, i.e. end of string */
  if (buffer[count-1] != 0)
    {
      /* In general, TCP recv can return any number of bytes, not
	 necessarily forming a complete message, so you need to
	 parse the input to see if a complete message has been received.
         if not, more calls to recv is needed to get a complete message.
      */
      printf("Message incomplete, something is still being transmitted\n");
    } 
  else
    {
      printf("Here is what we got: %s", buffer);
    }

  while (1) {
    printf("Type something, hit enter: ");
    fgets(buffer, size, stdin);
    /* we will have the newline character \n in the buffer,
       so wipe it out with a null to terminate the string */
    buffer[strlen(buffer)-1] = 0;
    if (strncmp(buffer, "bye", 3) == 0) {
      /* free the resources, generally important! */
      close(sock);
      free(buffer);
      break;
    } else {
      send(sock, buffer, strlen(buffer)+1, 0);
    }
  }

  return 0;
}
