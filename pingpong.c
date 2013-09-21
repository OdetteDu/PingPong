#include "server.h"

char *errorMsg = "your message is wrong!";

void PPreadClient(struct node *current, struct node *head)
{
	const int bufferLen = 65536;
	char buf[bufferLen], *temp;
	int receivedTime_sec = 0, receivedTime_usec = 0, count, sendLen, receiveLen, i;
	struct timeval current_time;

	count = recv(current->socket, buf, bufferLen, 0);
	if (count <= 0) {
		if (count == 0) {
			printf("Client closed connection. Client IP address is: %s\n",
			inet_ntoa(current->client_addr.sin_addr));
		}
		else {
			perror("error receiving from a client.....1");
		}

		/* connection is closed, clean up */
		close(current->socket);
		dump(head, current->socket);
	}
	else {
		printf("receive data: %d, left: %d\n", count, receiveLen - count);
		receiveLen = ntohs(*(unsigned short*)buf);
		temp = buf + count;
		receiveLen -= count;

		/* keep receiving all data */
		while (receiveLen > 0) {
			count = recv(current->socket, temp, receiveLen, 0);
			if (count <= 0) {
				if (errno == EAGAIN) continue;
				if (count == 0) {
				printf("Client closed connection. Client IP address is: %s\n",
					inet_ntoa(current->client_addr.sin_addr));
				}
				else {
					perror("error receiving from a client....2");
				}

				/* connection is closed, clean up */
				close(current->socket);
				dump(head, current->socket);
				return;
			}
			else {
				temp += count;
				receiveLen -= count;
				printf("receive data: %d, left: %d\n", count, receiveLen);
			}
		}

		if (receiveLen > 0 || ntohs(*(unsigned short*)buf) < 10) {
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
			printf("...\n\n");
		}

		/* start to send back message */
		printf("Start to send data...\n");
		temp = buf;
		while (sendLen > 0) {
			count = send(current->socket, temp, sendLen,0);
			if (count <= 0) {
				if (errno == EAGAIN) continue;
				if (count == 0) {
					printf("Client closed connection. Client IP address is: %s\n",
						inet_ntoa(current->client_addr.sin_addr));
				}
				else {
					perror("error receiving from a client....3");
				}
				
				/* connection is closed, clean up */
				close(current->socket);
				dump(head, current->socket);
				return;
			}
			else {
				temp += count;
				sendLen -= count;
				printf("Send data: %d, left: %d\n", count, sendLen);
			}
		}
		printf("\n\n");
	}
}
