#include "server.h"

/*****************************************/
/* main program                          */
/*****************************************/

/*
 * One parameter: ping-pong mode, will exchange simple message with clients by using required protocol;
 * The second parameter is "www": web-server mode, take the third parameter as root directory, using HTTP.
*/
int main(int argc, char **argv)
{
	/* server mode */
	int server_mode = -1;

	/* listening socket */
	int sock;

	/* default root directory of web server */
	char *defRoot = "./file";

	/* end of all definitions */

	/* check the server mode through paramters */
	if ((server_mode = checkmode(argc, argv)) < 0) {
		printf("***Error***\n\tparamters are invalid\n\tPlease try again.\n");
		exit(0);
	}
	
	/* start to listen */
	sock = establish(atoi(argv[1]));

	/* run in either ping-pong mode or web-server mode */
	switch (server_mode) {
	case MODE_PP:
		runServer(sock, server_mode, NULL);
		break;
	case MODE_SV:
		if (argc < 4)
			runServer(sock, server_mode, defRoot);
		else
			runServer(sock, server_mode, argv[3]);
		break;
	default:
		printf("***Error***\n\tFatal error occurs. I don't know why. Sorry!\n");
		exit(0);
	}
}
