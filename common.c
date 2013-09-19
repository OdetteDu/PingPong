#include "server.h"

/* use parameters to determine in which mode the server will run
 * param: the same as main routine
 * return: negative if no paramters; positive value indicates running mode
 */
int checkmode(int argc, char* argv[])
{
	if (argc < 2)
		return -1;
	else if (argc < 3 || strcmp(argv[2],"www") != 0)
		return MODE_PP;
	else
		return MODE_SV;
}

/* check if the root directory exists
 * param: the path will be checked
 * return: 0 if exists, -1 otherwise
 */
int checkRoot(char *str)
{
	FILE *fp = fopen(str, "r");
	if (fp == NULL)
		return -1;
	else {
		fclose(fp);
		return 0;
	}
}

/* initialize socket and setups the listen 
 * param: listening port
 * return: server socket
 */
int establish(unsigned short server_port)
{
	/* socket and option variables */
	int sock;
	int optval = 1;

	/* server socket address variables */
	struct sockaddr_in sin;

	/* maximum number of pending connection requests */
	int BACKLOG = 10;

	/* create a server socket to listen for TCP connection requests */
	if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror ("opening TCP socket");
		return -1;
	}

	/* set option so we can reuse the port number quickly after a restart */
	if (setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof (optval)) <0)
	{
		perror ("setting TCP socket option");
		return -2;
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
		return -3;
	}

	/* put the server socket in listen mode */
	if (listen (sock, BACKLOG) < 0)
	{
		perror ("listen on socket failed");
		return -4;
	}

	/* listening begins, return listening socket to main */
	return sock;
}

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
			if (temp->sendbuf != NULL)
				free(temp->sendbuf);
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
	new_node->sendbuf = NULL;
	new_node->pending_data = 0;
	new_node->pending_fd = -1;
	new_node->pending_index = -1;
	new_node->next = head->next;
	head->next = new_node;
}

int newClient(int server_sock, struct node *head)
{
	/* Welcome message */
	char *welmessage = "Welcome to my server! Yanfei Wu, Odette Du\n";
	int count;

	/* server socket address variables */
	struct sockaddr_in addr;

	/* socket address variables for a connected client */
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	int new_sock = accept (server_sock, (struct sockaddr *) &addr, &addr_len);

	if (new_sock < 0)
	{
		perror ("error accepting connection");
		return -1;
	}

	if (fcntl (new_sock, F_SETFL, O_NONBLOCK) < 0)
	{
		perror ("making socket non-blocking");
		return -2;
	}

	/* the connection is made, everything is ready */
	/* let's see who's connecting to us */
	printf("Accepted connection. Client IP address is: %s\n", inet_ntoa(addr.sin_addr));

	/* remember this client connection in our linked list */
	add(head, new_sock, addr);

	return new_sock;
}

/* server is running */
void runServer(int sock, int mode, char* rootDir)
{
	/* clients socket variables */
	int new_sock, max;

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
	int receivedTime_sec, receivedTime_usec;

	buf = (char *)malloc(BUF_LEN);
	sendbuffer = (char *)malloc(BUF_LEN);

	/* initialize dummy head node of linked list */
	head.socket = -1;
	head.next = 0;

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
				FD_SET(current->socket, &write_set);

			if (current->socket > max)
				max = current->socket;
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
				new_sock = newClient(sock, &head);
				if (new_sock < 0)
					printf("accepting client error occurs. Error code: %d.\n", new_sock);
			}

			for (current = head.next; current; current = next)
			{
				next = current->next;

				if (FD_ISSET(current->socket, &write_set))
				{
					count = send(current->socket, current->sendbuf, BUF_LEN, MSG_DONTWAIT);
					if (count < 0 && errno != EAGAIN)
					{
						/* something is wrong, so shut down the client */
						printf("Some sending procedure goes wrong.\n");
						printf("CLose this client: IP: %s\n", inet_ntoa(current->client_addr.sin_addr));
						close(current->socket);
						dump(&head, current->socket);
					}
					else {
						/* update the pending_date value*/
						current->pending_data -= count;
					}
				}

				if (FD_ISSET(current->socket, &read_set))
				{
					if (mode == MODE_PP)
						PPreadClient(current, &head);
					else if (mode == MODE_SV) {
						if (current->pending_fd > 0 && current->pending_data == 0)
							// unfinished HTTP request
							sendFile(current->pending_fd, current->pending_index, current, &head);
						else
							// new HTTP requent
							SVreadClient(current, &head, rootDir);
					}
				}
			}
		}
	}
}
