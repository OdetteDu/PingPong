#include "server.h"

char *rspMsg[] = {	"HTTP/1.1 200 OK \r\nContent-type: text/html \r\n\r\n",
			"HTTP/1.1 400 Bad Request \r\n\r\n",
			"HTTP/1.1 404 Not Found \r\n\r\n",
			"HTTP/1.1 500 Internal Server Error \r\n\r\n"
			};

/* Check if the request complies the HTTP protocol.
 * param: string from request
 * return: 0 if OK, negative value indicates error code
 * Error code map: -1: Bad Request
 */
int checkProtocol(char* str)
{
	// check the length
	if ((strlen(str) < 3) || (str[3] != ' '))
		return -1;

	// check the "GET"
	str[3] = '\0';
	if (strcmp(str, "GET"))
		return -1;
	str += 4;

	// bypass the path string (find the first space character)
	while (str[0] != ' ')
		str++;
	
	// check the "HTTP/1.1\r\n"
	str++;
	str[10] = '\0';
	if (strcmp(str, "HTTP/1.1\r\n"))
		return -1;

	// everything is right, set string to be a path, return 0 indicates OK
	(--str)[0] = '\0';
	return 0;
}

/* Check if the request path violates the security requirement.
 * param: string of path
 * return: 0 if OK, negative value indicates error code
 * Error code map: -1: Bad Request
 */
int checkPath(char* path)
{
	if (strstr(path,"../"))
		return -1;
	else
		return 0;
}

/* try to access the file with file path
 * return: file descriptor is OK, negative value indicates error code
 * Error code map: -2: Not Found
 */
int getFile(char* root, char* path)
{
	int fd;
	FILE *fp;
	int rootLen = strlen(root), pathLen = strlen(path);
	char *temp = malloc(sizeof(char) * (rootLen + pathLen + 1));

	// get the real path of file
	temp[0] = '\0';
	strcat(temp,root);
	temp[rootLen] = '\0';
	if (pathLen == 1 && path[0] == '/') {
		strcat(temp, "/index.html");
		pathLen = 11;
	}
	else
		strcat(temp,path);
	temp[rootLen + pathLen] = '\0';

	/* check if the file exists */
	fp = fopen(temp, "r");
	fd = open(temp, O_RDONLY);
	free(temp);

	// check if file can be read
	if (fp == NULL)
		return -2;	// file not found
	else {
		fclose(fp);	// free the testing pointer
		return fd;	// file can be read, return the descriptor
	}
}

/* Send requested data to client
 * param: fd is descriptor of the file will be sent, client is the target
 *        offset is the offset of the beginning data inside the file
 * return: 0 if OK, negative value indicates error code
 * Error code map: -3: Internal Server Error
 */
int sendData(int fd, off_t offset, struct node *client)
{
	char buf[BUF_LEN+1];
	char *temp;
	int readCount, sendCount;

	if (client->pending_fd < 0)
		/* this is the new request */
		sendCount = send(client->socket, rspMsg[0], strlen(rspMsg[0]), 0);

	if (sendCount < 0) {
		perror("error sending HTTP head to a client");
		return -3;
	}

	readCount = lseek(fd, offset, SEEK_SET);
	if (readCount < 0) {
		perror("error seeking position in file");
		return -3;
	}

	readCount = read(fd, buf, BUF_LEN);
	if (readCount < 0) {
		/* Some internal error occurs */
		perror("error reading from file");
		return -3;
	}

	while (readCount) {
		temp = buf;
		while (readCount) {
			sendCount = send(client->socket, temp, readCount, 0);
			if (sendCount < 0) {
				/* stop sending and start to backup */
				if (errno == EAGAIN) {
					/* nothing goes wrong, just need to be sent again */
					temp[readCount] = '\0';
					if (client->sendbuf != NULL)
						free(client->sendbuf);
					client->sendbuf = (char*)malloc(sizeof(char) * (BUF_LEN + 1));
					strcpy(client->sendbuf, temp);
					client->pending_data = readCount;
					client->pending_fd = fd;
					client->pending_index = lseek(fd, 0, SEEK_CUR);
					return 0;
				}
				else {
					/* Some internal error occurs */
					perror("error reading from file");
					return -3;
				}
			}
			readCount -= sendCount;
			temp += sendCount;
		}
		readCount = read(fd, buf, BUF_LEN);
	}

	/* finish sending the file, reset the pending value in client node */
	client->pending_fd = -1;
	client->pending_index = -1;
	return 0;
}

/* Send requested file to client
 * param: fd is descriptor of the file will be sent, client is the target
 *        offset is the offset of the beginning data inside the file
 */
void sendFile(int fd, off_t offset, struct node *current, struct node *head) {
	if (sendData(fd, offset, current) != 0) {
		send(current->socket, rspMsg[3], strlen(rspMsg[3]), 0);
		printf("***Internal Error when connecting with: %s\n\n", inet_ntoa(current->client_addr.sin_addr));
		close(current->socket);
		dump(head, current->socket);
		return;
	}

	if (current->pending_fd < 0) {
		/* connection is closed, clean up */
		printf("Finish Sending file. Client IP address is: %s\n", inet_ntoa(current->client_addr.sin_addr));
		close(current->socket);
		dump(head, current->socket);
	}
}

void SVreadClient(struct node *current, struct node *head, char *rootDir)
{
	char buf[BUF_LEN];	// buffer for receiving
	int count;		// number of received bytes
	int ret;		// other return value store

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
		/* check if received request complies the protocol */
		if ((ret = checkProtocol(buf)) != 0 || checkPath(buf+4) != 0) {
			send(current->socket, rspMsg[1], strlen(rspMsg[1]), 0);
			printf("***Bad Requst from: %s\n\n", inet_ntoa(current->client_addr.sin_addr));
			close(current->socket);
			dump(head, current->socket);
			return;
		}
		
		/* Check if requested file can be read. If so, get the file descriptor. */
		if ((ret = getFile(rootDir, buf+4)) < 0) {
			send(current->socket, rspMsg[2], strlen(rspMsg[2]), 0);
			printf("***Access Non-exist File from: %s\n\n", inet_ntoa(current->client_addr.sin_addr));
			close(current->socket);
			dump(head, current->socket);
			return;
		}

		sendFile(ret, 0, current, head);
	}
}
