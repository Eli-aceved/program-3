// Client side - UDP Code				    
// By Hugh Smith	4/1/2017		
// Modified by: Elizabeth Acevedo 03/05/2025

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

#include "gethostbyname.h"
#include "networks.h"
#include "safeUtil.h"
#include "helperfuncs.h"
#include "cpe464.h"
#include "rcopyCBuffer.h"
#include "pollLib.h"

#define MAXBUF 80
#define MAX_PDU 1407
#define MAXFILENAMELEN 100	// Max length of a file name


struct rcopyArgs {
	uint8_t *from_filename;
	uint8_t *to_filename;
	uint32_t window_size;
	uint16_t buffer_size;
	float error_rate;
	char * remote_machine;
	int remote_port;
	struct sockaddr_in6 serverAddress;
};

void talkToServer(int socketNum, struct rcopyArgs *args);
int checkArgs(int argc, char * argv[]);
void fillRcopyArgs(struct rcopyArgs *args, char *argv[], struct sockaddr_in6 serverAddress);
int createFilenamePacket(uint8_t *filename_packet, uint8_t *filename, uint16_t buffer_size, uint32_t window_size);


int main (int argc, char *argv[])
 {
	int socketNum = 0;				
	struct sockaddr_in6 server;		// Supports 4 and 6 but requires IPv6 struct
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);
	
	socketNum = setupUdpClientToServer(&server, argv[6], portNumber);

	setupPollSet();
	addToPollSet(socketNum);

	struct rcopyArgs args;
	fillRcopyArgs(&args, argv, server);

	sendErr_init(args.error_rate, DROP_ON, FLIP_ON, DEBUG_OFF, RSEED_ON);

	talkToServer(socketNum, &args);
	
	close(socketNum);

	return 0;
}

void talkToServer(int socketNum, struct rcopyArgs *args)
{
	int serverAddrLen = sizeof(struct sockaddr_in6);
	int dataLen = 0; 
	uint8_t buffer[MAX_PDU];

	dataLen = createFilenamePacket(buffer, args->from_filename, args->buffer_size, args->window_size);
	
	sendtoErr(socketNum, buffer, dataLen, 0, (struct sockaddr *) &args->serverAddress, serverAddrLen);

	int end_file = 0;

	while(end_file == 0) {
		
		if(pollCall(0)) {
			dataLen = safeRecvfrom(socketNum, buffer, MAXBUF, 0, (struct sockaddr *) &args->serverAddress, &serverAddrLen);
			
			if (buffer[6] == 10) {
				end_file = 1;
			}

			uint32_t packet_num = 0;
			memcpy(&packet_num, buffer, PACKETNUM_BYTES);
			packet_num = ntohl(packet_num);

			printf("Packet number: %u\n", packet_num);
		}
	}
	printf("Download Complete ;)\n");
	      
	
}

int checkArgs(int argc, char * argv[])
{

    int portNumber = 0;
	
        /** check command line arguments  
		 * rcopy uses 7 command line arguments
		 * rcopy <from-filename> <to-filename> <window-size> <buffer-size> <error-rate> <remote-machine> <remote-port>
		*/
	if (argc != 8)
	{
		printf("usage: %s host-name port-number \n", argv[0]);
		exit(1);
	}
	
	portNumber = atoi(argv[7]);
		
	return portNumber;
}

void fillRcopyArgs(struct rcopyArgs *args, char *argv[], struct sockaddr_in6 serverAddress) {
	args->from_filename = (uint8_t *) argv[1];
	args->to_filename = (uint8_t *) argv[2];
	args->window_size = (uint32_t) atoi(argv[3]);
	args->buffer_size = (uint16_t) atoi(argv[4]);
	args->error_rate = atof(argv[5]);
	args->remote_machine = argv[6];
	args->remote_port = atoi(argv[7]);
	args->serverAddress = serverAddress;
}

int createFilenamePacket(uint8_t *filename_packet, uint8_t *filename, uint16_t buffer_size, uint32_t window_size) {
	
	uint8_t payload[MAX_PDU] = {0};
	
	// Add filename to the packet
	uint8_t filename_length = strlen((char *)filename);

	// Add filename length to the packet
	memcpy(payload, &filename_length, 1);
	memcpy(&payload[1], filename, filename_length);

	// Add buffer size to the packet
	uint16_t buffer_size_netword = htons(buffer_size);
	memcpy(&payload[1 + filename_length], &buffer_size_netword, sizeof(buffer_size_netword));

	// Add window size to the packet
	uint32_t window_size_netword = htonl(window_size);
	memcpy(&payload[1 + filename_length + sizeof(buffer_size)], &window_size_netword, sizeof(window_size_netword));

	createPDU(filename_packet, payload, filename_length + 7, 0, 8); // Filename packet flag

	return filename_length + 7 + 7;
}