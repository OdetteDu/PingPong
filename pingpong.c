#include "server.h"

char *errorMsg = "your message is wrong!";

void PPreadClient(struct node *current, struct node *head)
{
	const int bufferLen = 65536;
	char buf[bufferLen], *temp;
	int receivedTime_sec = 0, receivedTime_usec = 0, count, sendLen, receiveLen, i;
	struct timeval current_time;

	/* start to receive message from client */
	printf("Start receiving message from: %s\n", inet_ntoa(current->client_addr.sin_addr));

	count = recv(current->socket, buf, bufferLen, 0);
	if (count <= 0) {
		if (count == 0) {
			printf("Client closed connection. Client IP address is: %s\n",
			inet_ntoa(current->client_addr.sin_addr));
		}
		else {
			perror("error receiving from a client at first time");
		}

		/* connection is closed, clean up */
		close(current->socket);
		dump(head, current->socket);
	}
	else {
		receiveLen = ntohs(*(unsigned short*)buf);
		temp = buf + count;
		receiveLen -= count;

		/* keep receiving all data */
		while (receiveLen > 0) {
			count = recv(current->socket, temp, receiveLen, 0);
			if (count <= 0) {
				if (count < 0 && errno == EAGAIN) continue;
				if (count == 0) {
				printf("Client closed connection. Client IP address is: %s\n",
					inet_ntoa(current->client_addr.sin_addr));
				}
				else {
					perror("error receiving from a client when data is still transmitting");
				}

				/* connection is closed, clean up */
				close(current->socket);
				dump(head, current->socket);
				return;
			}
			else {
				temp += count;
				receiveLen -= count;
			}
		}

		if (receiveLen > 0 || ntohs(*(unsigned short*)buf) < 10) {
			// imcomplete message or not comply the ping-pong protocol
			printf("Message incomplete, something is still being transmitted.\n");
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
			printf("Received length: %d\n", sendLen);
			printf("Received the time: %d %d.\n", receivedTime_sec, receivedTime_usec);
			printf("Received the data: ");
			for (i = 10; i < sendLen && i < 110; i++)
				printf("%c", buf[i]);
			printf("...\n");
		}

		/* start to send back message */
		printf("Start to send data back to client: %s\n", inet_ntoa(current->client_addr.sin_addr));
		temp = buf;
		while (sendLen > 0) {
			count = send(current->socket, temp, sendLen, 0);
			if (count <= 0) {
				if (count < 0 && errno == EAGAIN) {
					/* nothing goes wrong, just need to be sent again */
					temp[sendLen] = '\0';
					if (current->sendbuf != NULL)
						free(current->sendbuf);
					current->sendbuf = (char*)malloc(sizeof(char) * (sendLen + 1));
					strcpy(current->sendbuf, temp);
					current->pending_data = sendLen;
				}
				else if (count == 0) {
					printf("Client closed connection. Client IP address is: %s\n",
						inet_ntoa(current->client_addr.sin_addr));
					/* connection is closed, clean up */
					close(current->socket);
					dump(head, current->socket);
				}
				else {
					perror("error sending back to a client");
					/* connection is closed, clean up */
					close(current->socket);
					dump(head, current->socket);
				}

				return;
			}
			else {
				temp += count;
				sendLen -= count;
			}
		}

		printf("finish sending data to client.\n\n");
	}
}

void pingpongSendAgain(struct node *current, struct node *head) {
	int count;
	char *temp = current->sendbuf + (strlen(current->sendbuf) - current->pending_data);
	printf("resume sending...\n");
	while (current->pending_data > 0) {
		count = send(current->socket, temp, current->pending_data, 0);
		if (count <= 0) {
			if (count < 0 && errno == EAGAIN) {
				/* nothing goes wrong, just need to be sent again */
				return;
			}
			else if (count == 0) {
				printf("Client closed connection. Client IP address is: %s\n",
					inet_ntoa(current->client_addr.sin_addr));
				/* connection is closed, clean up */
				close(current->socket);
				dump(head, current->socket);
			}
			else {
				perror("error sending back to a client");
				/* connection is closed, clean up */
				close(current->socket);
				dump(head, current->socket);
			}

			return;
		}
		else {
			temp += count;
			current->pending_data -= count;
		}
	}

	printf("finish sending data to client.\n\n");
}
