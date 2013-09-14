#include "server.h"

void PPreadClient(struct node *current, struct node *head)
{
	char buf[BUF_LEN];
	char sendbuffer[10];
	int receivedTime_sec = 0, receivedTime_usec = 0, count;
	struct timeval current_time;

	count = recv(current->socket, buf, BUF_LEN, 0);

	if (count <= 0) {
		if (count == 0) {
			printf("Client closed connection. Client IP address is: %s\n",
			inet_ntoa(current->client_addr.sin_addr));
		}
		else {
			perror("error receiving from a client");
		}

		/* connection is closed, clean up */
		close(current->socket);
		dump(head, current->socket);
	}
	else {
		if (buf[0]+1 != count) {
			printf("Message incomplete, something is still being transmitted.\n");
			printf("CLose this client: IP: %s\n", inet_ntoa(current->client_addr.sin_addr));
			close(current->socket);
			dump(head, current->socket);
			return;
		}
		else {
			receivedTime_sec=(int) ntohl(*(int *)(buf+1));
			receivedTime_usec=(int) ntohl(*(int *)(buf+5));
			printf("Received the time: %d %d.\n", receivedTime_sec, receivedTime_usec);

			gettimeofday(&current_time, NULL);
			sendbuffer[0]=8;

			*(int *)(sendbuffer+1)=(int) htonl(current_time.tv_sec);
			*(int *)(sendbuffer+5)=(int) htonl(current_time.tv_usec);

			send(current->socket, sendbuffer, sendbuffer[0]+1,0);
			printf("Sent the time :%d %d.\n",(int) current_time.tv_sec, (int)current_time.tv_usec);
		}
	}
}
