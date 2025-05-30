
// Hugh Smith April 2017
// Network code to support TCP/UDP client and server connections

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "networks.h"
#include "gethostbyname.h"

struct connectionInfo *connect_info;

// This funciton creates a UDP socket on the server side and binds to that socket.  
// It prints out the port number and returns the socket number.

int udpServerSetup(int serverPort)
{
	struct sockaddr_in6 serverAddress;
	int socketNum = 0;
	int serverAddrLen = 0;	
	
	// create the socket
	if ((socketNum = socket(AF_INET6,SOCK_DGRAM,0)) < 0)
	{
		perror("socket() call error");
		exit(-1);
	}
	
	// set up the socket
	memset(&serverAddress, 0, sizeof(struct sockaddr_in6));
	serverAddress.sin6_family = AF_INET6;    		// internet (IPv6 or IPv4) family
	serverAddress.sin6_addr = in6addr_any ;  		// use any local IP address
	serverAddress.sin6_port = htons(serverPort);   // if 0 = os picks 

	// bind the name (address) to a port
	if (bind(socketNum,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
	{
		perror("bind() call error");
		exit(-1);
	}

	/* Get the port number */
	serverAddrLen = sizeof(serverAddress);
	getsockname(socketNum,(struct sockaddr *) &serverAddress,  (socklen_t *) &serverAddrLen);
	printf("Server using Port #: %d\n", ntohs(serverAddress.sin6_port));

	return socketNum;	
	
}

// This function opens a socket and fills in the serverAdress structure using the hostName and serverPort.  
// It assumes the address structure is created before calling this.
// Returns the socket number and the filled in serverAddress struct.

int setupUdpClientToServer(struct sockaddr_in6 *serverAddress, char * hostName, int serverPort)
{
	int socketNum = 0;
	char ipString[INET6_ADDRSTRLEN];
	uint8_t * ipAddress = NULL;
	
	// create the socket
	if ((socketNum = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket() call error");
		exit(-1);
	}
  	 	
	memset(serverAddress, 0, sizeof(struct sockaddr_in6));
	serverAddress->sin6_port = ntohs(serverPort);
	serverAddress->sin6_family = AF_INET6;	
	
	if ((ipAddress = gethostbyname6(hostName, serverAddress)) == NULL)
	{
		exit(-1);
	}
		
	
	inet_ntop(AF_INET6, ipAddress, ipString, sizeof(ipString));
	printf("Server info - IP: %s Port: %d \n", ipString, serverPort);
		
	return socketNum;
}

void setupConnectionInfo(uint8_t *from_filename, uint8_t *to_filename, uint32_t window_size, uint16_t buffer_size, float errorRate, char *hostName, int serverPort, struct sockaddr_in6 *serverAddress) {
	// Allocate memory for connectionInfo struct
	connect_info = (struct connectionInfo *)malloc(sizeof(struct connectionInfo));
	if (connect_info == NULL) {
		perror("Failed to allocate memory for connectionInfo");
		exit(EXIT_FAILURE);
	}

	// Set the fields of the connectionInfo struct
	connect_info->from_filename = from_filename;
	connect_info->to_filename = to_filename;
	connect_info->window_size = window_size;
	connect_info->buffer_size = buffer_size;
	connect_info->errorRate = errorRate;
	connect_info->hostName = hostName;
	connect_info->serverPort = serverPort;
	connect_info->serverAddress = serverAddress;
}

struct connectionInfo getConnectionInfo() {
	return *connect_info;
}

int replaceSocket(int old_socket) {
	int new_socket = 0;
	close(old_socket);
	new_socket = setupUdpClientToServer(connect_info->serverAddress, connect_info->hostName, connect_info->serverPort);
	return new_socket;
}

void freeConnectInfo() {
	// Free the memory allocated for connectionInfo struct
	if (connect_info) {
		free(connect_info);
	}
}