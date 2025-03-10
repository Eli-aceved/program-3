/* Server side - UDP Code				    */
/* By Hugh Smith	4/1/2017	*/
/* Modified by: */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"

#define MAXBUF 80

void processClient(int socketNum);
int checkArgs(int argc, char *argv[]);

int main ( int argc, char *argv[]  )
{ 
	int socketNum = 0;				
	int portNumber = 0;

	portNumber = checkArgs(argc, argv);
		
	socketNum = udpServerSetup(portNumber);

	processClient(socketNum);

	close(socketNum);
	
	return 0;
}

void processClient(int socketNum)
{
	int dataLen = 0; 
	char buffer[MAXBUF + 1];	  
	struct sockaddr_in6 client;		
	int clientAddrLen = sizeof(client);	
	
	buffer[0] = '\0';
	while (buffer[0] != '.')
	{
		dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) &client, &clientAddrLen);
	
		printf("Received message from client with ");
		printIPInfo(&client);
		printf(" Len: %d \'%s\'\n", dataLen, buffer);

		// just for fun send back to client number of bytes received
		sprintf(buffer, "bytes: %d", dataLen);
		safeSendto(socketNum, buffer, strlen(buffer)+1, 0, (struct sockaddr *) & client, clientAddrLen);

	}
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;
	//float errorRate = 0.0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	/**
	 * server uses two command line arguments: 
	 * server <error rate> <port number>
	 */

	if (argc == 2)
	{
		// errorRate = atof(argv[0]);
		portNumber = atoi(argv[1]);
	}
	
	// return errorRate;
	return portNumber;
}


