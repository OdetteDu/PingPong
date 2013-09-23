all:
	gcc server.c common.c pingpong.c webserver.c -o server
	gcc client.c -o client
