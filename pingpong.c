#include "server.h"

char *errorMsg = "your message is wrong!";

void PPreadClient(struct node *current, struct node *head)
{
	char buf[BUF_LEN];
	char sendbuffer[100];
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
		if (*(short*)buf != count) {
			printf("Message incomplete, something is still being transmitted.\n");
			printf("CLose this client: IP: %s\n", inet_ntoa(current->client_addr.sin_addr));
			close(current->socket);
			dump(head, current->socket);
			return;
		}
		else {
			if (count < 10)
			{
				printf("message doesn't comply ping-pong protocol");
				*buf = (short) htons(11 + strlen(errorMsg));
				*(int *)(buf + 2) = 0;
				*(int *)(buf + 6) = 0;
				buf[10] = '\0';
				strcat(buf, errorMsg);
			}
			else {
				receivedTime_sec=(int) ntohl(*(int *)(buf+2));
				receivedTime_usec=(int) ntohl(*(int *)(buf+6));
				printf("Received the time: %d %d.\n", receivedTime_sec, receivedTime_usec);
			}

			send(current->socket, buf, buf[0],0);
		}
	}
}
