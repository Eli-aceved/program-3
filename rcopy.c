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
#include "buffmgmnt.h"


#define MAXBUF 80
#define MAX_PDU 1407
#define MAXFILENAMELEN 100	// Max length of a file name


struct rcopyArgs {
	uint8_t *from_filename;
	uint8_t *to_filename;
	uint32_t window_size;
	uint16_t buffer_size;
	float errorRate;
	char *remote_machine;
	int remote_port;
	struct sockaddr_in6 serverAddress;
};

void talkToServer(int socketNum, struct rcopyArgs *args);
int checkArgs(int argc, char * argv[]);
void fillRcopyArgs(struct rcopyArgs *args, char *argv[], struct sockaddr_in6 serverAddress);
int createFilenamePacket(uint8_t *filename_packet, uint8_t *filename, uint16_t buffer_size, uint32_t window_size);
void createFilenameData(uint8_t *dest_buff, uint8_t filename_len);
int establishFilename(int *socketNum);
int filename_state(int *socketNum, int *attempt_cnt, int poll_result, uint8_t *filename_packet, int filename_len);
int recvDataPackets(int socketNum);
int verifyFilename(uint8_t *recv_packet, int packet_len);

int main (int argc, char *argv[])
 {
	int socketNum = 0;				
	struct sockaddr_in6 server;		// Supports 4 and 6 but requires IPv6 struct
	int portNumber = 0;
	float errorRate = 0.0;
	
	setupPollSet();
	portNumber = checkArgs(argc, argv);
	
	socketNum = setupUdpClientToServer(&server, argv[6], portNumber);

	addToPollSet(socketNum);

	if ((errorRate = getErrorRate(argv[5])) == -1) {
		return -1;
	}

	struct rcopyArgs args;
	fillRcopyArgs(&args, argv, server);

	sendErr_init(args.errorRate, DROP_ON, FLIP_ON, DEBUG_OFF, RSEED_ON);

	setupConnectionInfo(args.from_filename, args.to_filename, args.window_size, args.buffer_size, args.errorRate, args.remote_machine, args.remote_port, &args.serverAddress);
	establishFilename(&socketNum); // Establish the filename with the server
	freeConnectInfo(); // Free the connection info after use

	return 0;
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
	args->errorRate = atof(argv[5]);
	args->remote_machine = argv[6];
	args->remote_port = atoi(argv[7]);
	args->serverAddress = serverAddress;
}

int establishFilename(int *socketNum) {
	int attempt_cnt = 0;
	int result = 0;
	struct connectionInfo cinfo = getConnectionInfo();
	uint8_t filename_packet[MAX_PDU + 1] = {0};
	uint8_t filename_data[MAX_PDU + 1] = {0};

	int filename_len = strlen((char *)cinfo.from_filename);

	createFilenameData(filename_data, (uint8_t)filename_len);
	int data_size = createPDU(filename_packet, filename_data, filename_len + 7, 0, 8); // Filename packet flag
	
	
	sendtoErr(*socketNum, filename_packet, data_size, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));
	int poll_result = pollCall(1000);

	result = filename_state(socketNum, &attempt_cnt, poll_result, filename_packet, data_size);
	if (result == 1) {
		recvDataPackets(*socketNum);
	}
	else if (result == -1) {
		return 0;
	}

	return 0;
}

void createFilenameData(uint8_t *dest_buff, uint8_t filename_len) {
	struct connectionInfo cinfo = getConnectionInfo();

	memcpy(dest_buff, &filename_len, 1);
	memcpy(&dest_buff[1], cinfo.from_filename, filename_len);
	memcpy(&dest_buff[filename_len + 1], &cinfo.buffer_size, 2);
	memcpy(&dest_buff[filename_len + 3], &cinfo.window_size, 4);

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

int filename_state(int *socketNum, int *attempt_cnt, int poll_result, uint8_t *filename_packet, int filename_len) {

	struct connectionInfo cinfo = getConnectionInfo();
	int serverAddrLen = sizeof(struct sockaddr_in6);
	int p_len = 0;

	uint8_t recv_packet[MAXBUF] = {0};

	while (*attempt_cnt < 9) {

		if (poll_result > 0) {
			p_len =  safeRecvfrom(*socketNum, recv_packet, MAXBUF, 0, (struct sockaddr *) cinfo.serverAddress, &serverAddrLen);
		}
		
		if (poll_result == 0 || verifyFilename(recv_packet, p_len) == -1) {
			close(*socketNum);
			removeFromPollSet(*socketNum);
			*socketNum = replaceSocket(*socketNum);
			addToPollSet(*socketNum);
			sendtoErr(*socketNum, filename_packet, filename_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));
			(*attempt_cnt)++;
			poll_result = pollCall(1000);

		}
		else if (verifyFilename(recv_packet, p_len) == 1) {
			printf("Filename OK!\n");
			return 1;
		}
		else if (verifyFilename(recv_packet, p_len) == 0) {
			printf("Server couldn't open file '%s'\n", cinfo.from_filename);
			return -1;
		}
	}
	printf("Timed out waiting for server\n");

	return -1;
}

int recvDataPackets(int socketNum) {
	struct connectionInfo cinfo = getConnectionInfo();

	FILE *file = fopen((char *)cinfo.to_filename, "wb");
	if (!file) {
		perror("Failure opening file");
		return -1;
	}

	initBuffer(cinfo.window_size);

	while (pollCall(10000) > 0) {
		dataMngmntFSM(socketNum, file);
	}
	printf("Timed out waiting for server\n");
	return -2;
}

int verifyFilename (uint8_t *recv_packet, int packet_len) {
	// Check if checksum is valid and if the flag is correct
	if (in_cksum((uint16_t *)recv_packet, packet_len) != 0 || recv_packet[6] != 9) { 
		return -1;
	}
	else { 
		// Either 0 or 1 depending if good or bad filename
		return recv_packet[7];
	}
}