CC	 	= gcc
LD	 	= gcc
CFLAGS	 	= -Wall -g

LDFLAGS	 	= 
DEFS 	 	=

all:	server_num server_text client_num client_text name_addr

server_num: server_num.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o server_num server_num.c

client_num: client_num.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o client_num client_num.c

server_text: server_text.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o server_text server_text.c

client_text: client_text.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o client_text client_text.c

name_addr:	name_addr.c
	$(CC) $(DEFS) $(CFLAGS) $(LIB) -o name_addr name_addr.c

clean:
	rm -f *.o
	rm -f *~
	rm -f core
	rm -f server_num
	rm -f client_num
	rm -f server_text
	rm -f client_text
	rm -f name_addr
