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

  char *receivebuffer,*sendbuffer;
  int size = 500;
  int count;

  receivebuffer = (char *) malloc(size);
  sendbuffer = (char *) malloc(size);

  if (!receivebuffer)
    {
      perror("failed to allocated receivebuffer");
      abort();
    }

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

  count = recv(sock, receivebuffer, size, 0);
  if (count < 0)
    {
      perror("receive failure");
      abort();
    }

  if (receivebuffer[count-1] != 0)
    {
      printf("Message incomplete, something is still being transmitted\n");
    } 
  else
    {
      printf("Here is what we got: %s",receivebuffer);
    }

  while (1) {
    printf("Please enter the get request: ");
    fgets(sendbuffer, size, stdin);

    sendbuffer[strlen(sendbuffer)-1] = 0;
    if (strncmp(sendbuffer, "q", 1) == 0) {
      /* free the resources, generally important! */
      close(sock);
      free(sendbuffer);
      break;
    } else {
		
		if (strncmp(sendbuffer, "GET", 3)!=0)
		{
			  printf("You have entered invalid HTTP Request. Please try again.");
		}

		send(sock, sendbuffer, strlen(sendbuffer)+1, 0);
    }
  }

  return 0;
}
