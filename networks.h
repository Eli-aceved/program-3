
// 	Writen - HMS April 2017
//  Supports TCP and UDP - both client and server


#ifndef __NETWORKS_H__
#define __NETWORKS_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/* Defines */
#define LISTEN_BACKLOG 10

/* Structs */
struct connectionInfo {
	uint8_t *from_filename;
	uint8_t *to_filename;
	uint32_t window_size;
	uint16_t buffer_size;
	float errorRate;
	char *hostName;
	int serverPort;
	struct sockaddr_in6 *serverAddress;
};

/* Function Prototypes */
// For UDP Server and Client
int udpServerSetup(int serverPort);
int setupUdpClientToServer(struct sockaddr_in6 *serverAddress, char * hostName, int serverPort);
void setupConnectionInfo(uint8_t *from_filename, uint8_t *to_filename, uint32_t window_size, uint16_t buffer_size, float errorRate, char *hostName, int serverPort, struct sockaddr_in6 *serverAddress);
struct connectionInfo getConnectionInfo();
int replaceSocket(int old_socket);
void freeConnectInfo();
#endif
