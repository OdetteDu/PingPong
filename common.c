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
