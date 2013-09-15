#include "server.h"

char *errorMsg = "your message is wrong!";

void PPreadClient(struct node *current, struct node *head)
{
	char buf[BUF_LEN];
	int receivedTime_sec = 0, receivedTime_usec = 0, count, sendLen;
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
		if ((int) ntohs(*(unsigned short*)buf) != count) {
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
				sendLen = 11 + strlen(errorMsg);
				*buf = (unsigned short) htons(11 + strlen(errorMsg));
				*(int *)(buf + 2) = (int)0;
				*(int *)(buf + 6) = (int)0;
				buf[10] = '\0';
				strcat(buf, errorMsg);
			}
			else {
				sendLen = (int) ntohs(*(unsigned short *)buf);
				receivedTime_sec=(int) ntohl(*(int *)(buf+2));
				receivedTime_usec=(int) ntohl(*(int *)(buf+6));
				buf[sendLen] = '\0';
				printf("Received the time: %d %d.\n", receivedTime_sec, receivedTime_usec);
				printf("Received the data: %s\n\n", buf+10);
			}

			send(current->socket, buf, sendLen,0);
		}
	}
}
