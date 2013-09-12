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

/* simple client, takes two parameters, the server domain name,
   and the server port number */

void main(int argc, char** argv)
{

	/* our client socket */
	int sock;

	/* address structure for identifying the server */
	struct sockaddr_in sin;

	/* convert server domain name to IP address */
	struct hostent *host = gethostbyname(argv[1]);
	unsigned int server_addr = *(unsigned int *) host->h_addr_list[0];

	/* server port number */
	unsigned short server_port = atoi (argv[2]);

	char *buffer, *sendbuffer;
	int size = 500;
	int count;
	int msgNumber = atoi (argv[3]);
	int i;
	//int num;

	struct timeval tim;

	/* allocate a memory buffer in the heap */
	buffer = (char *) malloc(size);
	if (!buffer)
	{
		perror("failed to allocated buffer");
		abort();
	}

	sendbuffer = (char *) malloc(size);
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

	/* receive data */
	count = recv(sock, buffer, size, 0);
	if (count < 0)
	{
		perror("receive failure");
		abort();
	}

	/* expect the last byte of the string to be 0 */
	if (buffer[count-1] != 0)
	{
		printf("Message incomplete, something is still being transmitted\n");
	}
	else
	{
		printf("Here is what we got: %s", buffer);
	}

	for (i = 0; i < msgNumber; i++)
	{
		gettimeofday(&tim,NULL);
		printf("\nGet time of day: %d %d.\n",(int)tim.tv_sec,(int)tim.tv_usec);
		sendbuffer[0] = 8;

		*(int *) (sendbuffer+1) = (int) htonl(tim.tv_sec);
		*(int *) (sendbuffer+5) = (int) htonl(tim.tv_usec);

		send(sock, sendbuffer, sendbuffer[0]+1, 0);
		recv(sock, buffer, size, 0);
		printf("Received the time: %d, %d.\n", (int) ntohl(*(int *)(buffer+1)), (int) ntohl(*(int *) (buffer+5)));
	}
}
