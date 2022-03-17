/* This is the client part for sending one file through network stream socket 
 and waiting for its return.  
This writer can run anywhere. 
gcc writern.c -lsocket -lxnet
clang and most gcc (like on osnodes) does not want any libraries included (there by default)
explicit library calls are required depending on the OS.

4/13/17: Writes from info from Command line. */


#include <sys/types.h> 
#include <sys/socket.h>
#include <stdlib.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define IP_ADDR "10.247.53.97"   /* osnode16's IP address, where server 
                                    must run */
#define IP_ADDR_LOCAL "127.0.0.1"
/* For test #define PORT_NUM 1150 */
/* #define PORT_NUM 1050 */
//#define PORT_NUM 1044
//#define PORT_NUM 1050
//#define PORT_NUM 1159
//#define PORT_NUM 1051
//#define PORT_NUM 1050
#define PORT_NUM 10501
#define BUFFLEN 100

int  main(int argc, char *argv[])
{
	int len;
	int sock;
	struct sockaddr_in  server;
	char buf[BUFFLEN];

	if ( argc != 2 ) { /* argc should be 2 for correct execution */
		/* We print argv[0] assuming it is the program name */
		printf( "usage: %s filename", argv[0] );
		exit(0);
	}
	/* socket() call generates one socket for ipc */
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Opening Stream Socket");
		exit(1);
	}

	/* address naming for the internet domain socket */
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT_NUM);
	server.sin_addr.s_addr = inet_addr(IP_ADDR_LOCAL);

	/* use the socket and server address to initiate connection */
	if(connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0)	{
		perror("connecting stream socket");
		exit(1);
	}

	len = strlen(argv[1]);
	printf("Connected and sending out a message of len %d\n", len);

	if (send(sock, argv[1], len, 0) == -1) {
		perror("Write to Socket Channel");
		exit(1);
	}

	len = recv(sock, buf, BUFFLEN, 0);
	printf("Got back: \n");
	fflush(stdout);
	write(0, buf, len);
	printf("\nEnd msg*** \n");

	/* close the communication socket and listenning socket */
	close(sock);
}