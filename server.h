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

#define MODE_PP 0x01
#define MODE_SV 0x02

#define BUF_LEN 1000

/* A linked list node data structure to maintain application
information related to a connected socket */
struct node
{
	int socket;
	struct sockaddr_in client_addr;
	struct node *next;
	
	/* for sending pending data */
	char *sendbuf;		// pending data buffer
	int pending_data;	// size of pending data (byte) inside the buffer
	int pending_fd;		// file descriptor to file which is still being sent
	off_t pending_index;	// index inside the pending file
};

/* functions in common.c */
int checkmode(int argc, char* argv[]);
int checkRoot(char *str);
int establish(unsigned short server_port);
void dump(struct node *head, int socket);
void add(struct node *head, int socket, struct sockaddr_in addr);
int newClient(int server_sock, struct node *head);
void runServer(int sock, int mode, char* rootDir);

/* functions in pingpong.c */
void PPreadClient(struct node *current, struct node *head);
void pingpongSendAgain(struct node *current, struct node *head);

/* functions in webserver.c */
int checkProtocol(char* str);
int checkPath(char* path);
int getFile(char* root, char* path);
int sendData(int fd, off_t offset, struct node *client);
void sendFile(int fd, off_t offset, struct node *current, struct node *head);
void SVreadClient(struct node *current, struct node *head, char *rootDir);
