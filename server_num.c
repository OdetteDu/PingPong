#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

/**************************************************/
/* a few simple linked list functions             */
/**************************************************/
char *message = "HTTP/1.1 200 OK \r\nContent-Type: text/html \r\n\r\n";
char *message200 = "HTTP/1.1 200 OK \r\n";
char *message400 ="HTTP/1.1 400 Bad Request \r\n";
char *message404 ="HTTP/1.1 404 Not Found \r\n";
char *message500 ="HTTP/1.1 500 Internal Server Error \r\n";
char *message501 ="HTTP/1.1 501 Not Implemented \r\n";
/* A linked list node data structure to maintain application
information related to a connected socket */
struct node
{
	int socket;
	struct sockaddr_in client_addr;
	int pending_data; /* flag to indicate whether there is more data to send */
	struct node *next;
};

/* remove the data structure associated with a connected socket
used when tearing down the connection */
void dump(struct node *head, int socket)
{
	struct node *current, *temp;

	current = head;

	while (current->next)
	{
		if (current->next->socket == socket)
		{
			temp = current->next;
			current->next = temp->next;
			free(temp);
			return;
		}
		else
		{
			current = current->next;
		}
	}
}

/* create the data structure associated with a connected socket */
void add(struct node *head, int socket, struct sockaddr_in addr)
{
	struct node *new_node;

	new_node = (struct node *)malloc(sizeof(struct node));
	new_node->socket = socket;
	new_node->client_addr = addr;
	new_node->pending_data = 0;
	new_node->next = head->next;
	head->next = new_node;
}

char* getPath(char* str)
{
	int pos;
	char *temp;

	//Check if the first three character is GET
	temp=malloc(sizeof(char) * 4);
	strncpy(temp,str,3);
	temp[4]='\0';
	if(!(strcmp(temp,"GET")==0))
	{
		printf("Violate HTTP Protocol, the first three character is not GET\n");
		return 0;
	}
	free(temp);

	//Get the path by getting the string after GET and before a space
	str+=4;	  
	pos=strcspn(str," ");
	temp=malloc(sizeof(char) * (pos+1));
	strncpy(temp,str,pos);
	temp[pos+1]='\0';
	return temp;
}

char* getFile(char* root,char* path)
{
	int fd;
	int count;
	char *temp;
	char *file;

	temp=malloc(sizeof(char)*(strlen(root)+strlen(path)));
	strcat(temp,root);
	strcat(temp,path);
	fd = open(temp, O_RDONLY);
	count = read(fd, file, BUF_LEN);
	free(temp);
	if(count<0)
	{
		temp=malloc(sizeof(char)*(strlen(message404)));
		temp=message404;
	}
	else
	{
		temp=malloc(sizeof(char)*count+strlen(message));
		strcat(temp,message);
		strcat(temp,file);
	}
	return temp;

}


/*****************************************/
/* main program                          */
/*****************************************/

/* simple server, takes one parameter, the server port number */
int main(int argc, char **argv)
{
	int isWWWMode=0;
	char *root;

	/* socket and option variables */
	int sock, new_sock, max;
	int optval = 1;

	/* server socket address variables */
	struct sockaddr_in sin, addr;
	unsigned short server_port = atoi(argv[1]);

	/* socket address variables for a connected client */
	socklen_t addr_len = sizeof(struct sockaddr_in);

	/* maximum number of pending connection requests */
	int BACKLOG = 5;

	/* variables for select */
	fd_set read_set, write_set;
	struct timeval time_out,current_time;
	int select_retval;

	/* number of bytes sent/received */
	int count;

	/* linked list for keeping track of connected sockets */
	struct node head;
	struct node *current, *next;

	/* a buffer to read data */
	char *buf,*sendbuffer, *temp;
	int BUF_LEN = 1000;
	int receivedTime_sec, receivedTime_usec;

	buf = (char *)malloc(BUF_LEN);
	sendbuffer = (char *)malloc(BUF_LEN);

	/* initialize dummy head node of linked list */
	head.socket = -1;
	head.next = 0;

	if(strcmp(argv[2],"WWW")==0)
	{
		isWWWMode=1;
		root=argv[3];
		printf("Switch to WWW Mode. The root directory is %s", root);
	}

	/* create a server socket to listen for TCP connection requests */
	if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror ("opening TCP socket");
		abort ();
	}

	/* set option so we can reuse the port number quickly after a restart */
	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
	{
		perror ("setting TCP socket option");
		abort ();
	}

	/* fill in the address of the server socket */
	memset (&sin, 0, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons (server_port);

	/* bind server socket to the address */
	if (bind(sock, (struct sockaddr *) &sin, sizeof (sin)) < 0)
	{
		perror("binding socket to address");
		abort();
	}

	/* put the server socket in listen mode */
	if (listen (sock, BACKLOG) < 0)
	{
		perror ("listen on socket failed");
		abort();
	}

	while (1)
	{
		FD_ZERO (&read_set);
		FD_ZERO (&write_set);

		FD_SET (sock, &read_set); /* put the listening socket in */
		max = sock; /* initialize max */

		for (current = head.next; current; current = current->next)
		{
			FD_SET(current->socket, &read_set);

			if (current->pending_data)
			{

				FD_SET(current->socket, &write_set);
			}

			if (current->socket > max)
			{

				max = current->socket;
			}
		}

		time_out.tv_usec = 100000;
		time_out.tv_sec = 0;

		select_retval = select(max+1, &read_set, &write_set, NULL, &time_out);
		if (select_retval < 0)
		{
			perror ("select failed");
			abort ();
		}

		if (select_retval == 0)
		{
			continue;
		}

		if (select_retval > 0)
		{
			if (FD_ISSET(sock, &read_set)) /* check the server socket */
			{
				/* there is an incoming connection, try to accept it */
				new_sock = accept (sock, (struct sockaddr *) &addr, &addr_len);

				if (new_sock < 0)
				{
					perror ("error accepting connection");
					abort ();
				}

				if (fcntl (new_sock, F_SETFL, O_NONBLOCK) < 0)
				{
					perror ("making socket non-blocking");
					abort ();
				}

				/* the connection is made, everything is ready */
				/* let's see who's connecting to us */
				printf("Accepted connection. Client IP address is: %s\n",
					inet_ntoa(addr.sin_addr));

				/* remember this client connection in our linked list */
				add(&head, new_sock, addr);

				/* let's send a message to the client just for fun */
				count = send(new_sock, message, strlen(message)+1, 0);
				if (count < 0)
				{
					perror("error sending message to client");
					abort();
				}
			}

			for (current = head.next; current; current = next)
			{
				next = current->next;

				if (FD_ISSET(current->socket, &write_set))
				{
					count = send(current->socket, buf, BUF_LEN, MSG_DONTWAIT);
					if (count < 0)
					{
						if (errno == EAGAIN)
						{

						}
						else
						{
							/* something else is wrong */
						}
					}
				}

				if (FD_ISSET(current->socket, &read_set))
				{
					count = recv(current->socket, buf, BUF_LEN, 0);
					if (count <= 0)
					{
						if (count == 0)
						{
							printf("Client closed connection. Client IP address is: %s\n",
								inet_ntoa(current->client_addr.sin_addr));
						}
						else
						{
							perror("error receiving from a client");
						}

						/* connection is closed, clean up */
						close(current->socket);
						dump(&head, current->socket);
					}
					else
					{
						if (buf[0]+1 != count)
						{

							printf("Message incomplete, something is still being transmitted\n");
							return 0;
						}
						else
						{
							if(!isWWWMode)
							{
								receivedTime_sec=(int) ntohl(*(int *)(buf+1));
								receivedTime_usec=(int) ntohl(*(int *)(buf+5));
								printf("Received the time: %d %d.\n",receivedTime_sec,receivedTime_usec);

								gettimeofday(&current_time,NULL);
								sendbuffer[0]=8;

								*(int *)(sendbuffer+1)=(int) htonl(current_time.tv_sec);
								*(int *)(sendbuffer+5)=(int) htonl(current_time.tv_usec);

								send(current->socket, sendbuffer, sendbuffer[0]+1,0);
								printf("Sent the time :%d %d.\n",(int)current_time.tv_sec,(int)current_time.tv_usec);
							}
							else
							{
								temp=getPath(buf);
								printf("Received get request with path: %s\n", temp);
								sendbuffer=getFile(root,temp);
								printf("The file will be send to client: \n%s\n",sendbuffer);
								send(current->socket, sendbuffer, sendbuffer[0]+1,0);
							}

						}
					}
				}
			}
		}
	}
}
