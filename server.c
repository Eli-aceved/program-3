/* Server side - UDP Code				    */
/* By Hugh Smith	4/1/2017	*/
/* Modified by: Elizabeth Acevedo */

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
#include "pollLib.h"
#include "cpe464.h"
#include "windowBuffer.h"
#include "helperfuncs.h"

#define MAX_PAYLOAD 1400    // Max size for payload

void processClients(int socketNum, float errorRate);
int checkArgs(int argc, char *argv[]);
size_t readFromFile(uint8_t *pdu, FILE *file, uint16_t buffer_size, uint8_t *data_chunk, uint8_t flag, uint32_t packet_num);
void parseFilenamePacket(int socketNum, uint8_t *filename_packet, size_t filename_packet_size, struct sockaddr_in6 client_addr);
void serverWindowControl(int socketNum, FILE *file, uint16_t buffer_size, uint32_t window_size, struct sockaddr_in6 client_addr);
int processRRs_N_SREJs(int socketNum);

int main(int argc, char *argv[])
{ 
	int socketNum = 0;				
	int portNumber = 0;
	float errorRate = 0.0;

	portNumber = checkArgs(argc, argv);
	errorRate = atof(argv[1]);

	socketNum = udpServerSetup(portNumber);
	setupPollSet();
	addToPollSet(socketNum);

	while(1) {
		processClients(socketNum, errorRate);
	}
	
	return 0;
}


void processClients(int socketNum, float errorRate) {
	uint8_t filename_packet[MAX_PDU] = {0};	  
	struct sockaddr_in6 client;		
	int clientAddrLen = sizeof(client);

	// Poll for incoming packets
	if (pollCall(0) > 0) {

		int filename_packet_size = safeRecvfrom(socketNum, filename_packet, MAX_PDU, 0, (struct sockaddr *) &client, &clientAddrLen);

		// FORK HERE
		int pid = fork();
		if (pid < 0) {
			perror("Error forking");
			exit(EXIT_FAILURE);
		}
		if (pid == 0) {
			// Child process
			sendErr_init(errorRate, DROP_ON, FLIP_OFF, DEBUG_OFF, RSEED_OFF);
			parseFilenamePacket(socketNum, filename_packet, filename_packet_size, client);
			exit(EXIT_SUCCESS);
		}
		else {
			// Parent doesn't do anything special
		}
	}

}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 3 || argc < 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	/**
	 * server uses two command line arguments: 
	 * server <error rate> <port number>
	 */
	if (argc == 3)
	{
		portNumber = atoi(argv[2]);
	}
	return portNumber;
}


size_t readFromFile(uint8_t *pdu, FILE *file, uint16_t buffer_size, uint8_t *data_chunk, uint8_t flag, uint32_t packet_num) {
    // Read from file
    uint16_t bytes_read = fread(data_chunk, 1, buffer_size, file);
    if (bytes_read == 0) {
        perror("Error reading from file");
        return 1;
    }
    if (bytes_read > 0) {
        createPDU(pdu, data_chunk, bytes_read, packet_num, 16); // Data packet flag
    }

	storetoWindowBuffer(pdu, bytes_read + 7);

	return bytes_read;
}


void parseFilenamePacket(int socketNum, uint8_t *filename_packet, size_t filename_packet_size, struct sockaddr_in6 client_addr) {

	uint8_t filename[MAX_PDU] = {0};
	uint16_t buffer_size = 0;
	uint32_t window_size = 0;
	uint8_t filename_length = 0;

	filename_length = filename_packet[7];

	// Extract filename from filename packet
	memcpy(filename, &filename_packet[8], filename_length);
	// Extract buffer size from filename packet
	memcpy(&buffer_size, &filename_packet[8 + filename_length], sizeof(buffer_size));
	buffer_size = ntohs(buffer_size);
	// Extract window size from filename packet
	memcpy(&window_size, &filename_packet[8 + filename_length + sizeof(buffer_size)], sizeof(window_size));
	window_size = ntohl(window_size);

	printf("Received message from client\n");
	printf(" Length: %u\n", filename_length);
	printf(" Filename: %s\n", (char *)filename);
	printf(" Buffer Size: %u\n", buffer_size);
	printf(" Window Size: %u\n", window_size);

	// Open file
	FILE *file = fopen((char *)filename, "rb");
	if (!file) {
		perror("Error opening file");
		exit(EXIT_FAILURE);
	}


	serverWindowControl(socketNum, file, buffer_size, window_size, client_addr);

	// Close file
    fclose(file);
}

void serverWindowControl(int socketNum, FILE *file, uint16_t buffer_size, uint32_t window_size, struct sockaddr_in6 client_addr) {
	uint8_t flag = 16; // Default at data packet flag
	uint8_t data_chunk[MAX_PAYLOAD] = {0};
	uint8_t pdu[MAX_PDU] = {0};	// Creating PDUs
	uint8_t dest_buffer[MAX_PDU] = {0};	// Retrieving packets from buffer (holds lowest packet)

	// Initialize window
	initWindow(window_size);

	// while not at EOF
	while (!feof(file) ) {
		// Check if window is open
		if (!windowisClosed()) {
			// Read from file
			readFromFile(pdu, file, buffer_size, data_chunk, flag, getCurrent());
			// Check if reached end of file
			if (feof(file)){
				flag = 10; // EOF flag
			}
			// Create PDU
			//createPDU(pdu, data_chunk, buffer_size, getCurrent(), flag);
			// Send packets from buffer
			sendtoErr(socketNum, pdu, buffer_size, 0, (struct sockaddr *) &client_addr, sizeof(client_addr));
			
			// Poll for RRs/SREJs (non-blocking)
			while (pollCall(0)){
				// Process RRs/SREJs
				processRRs_N_SREJs(socketNum);
			}
		}

		// Check if window is closed
		if (windowisClosed()) {
			int count = 0;
			// If timeout occurs, resend lowest unacknowledged packet
			while (pollCall(1000) && count < 10) {
				retrieveLowestPacket(dest_buffer); // Get lowest packet
				// Resend lowest packet

				sendtoErr(socketNum, dest_buffer, buffer_size, 0, (struct sockaddr *) &client_addr, sizeof(client_addr));
				count++;
			}
			// Process RRs/SREJs
			processRRs_N_SREJs(socketNum);
		}
	}

}

int processRRs_N_SREJs(int socketNum) {
	int dataLen = 0; 
	char buffer[MAX_PDU] = {0};	  
	uint8_t pdu[MAX_PDU] = {0};
	struct sockaddr_in6 client;		
	int clientAddrLen = sizeof(client);

	uint32_t packet_num = 0;

	dataLen = safeRecvfrom(socketNum, (char *)buffer, MAX_PDU, 0, (struct sockaddr *) &client, &clientAddrLen);

	// Assign designated flag
	uint8_t flag = buffer[6];

	if (in_cksum((unsigned short *)buffer, dataLen) == 0) {
		// Extract packet number from packet

		memcpy(&packet_num, &buffer[7], sizeof(packet_num));
		packet_num = ntohl(packet_num);

		// Check for EOF ACK flag for graceful termination
		if (flag == 34) { // End of file ACK flag
			return 3;	// return fave number
		}

		// Check if RR or SREJ
		if (flag == 5) {	// RR flag
			// Slide window
			slideWindow(packet_num);
		}
		else if (flag == 6) {	// SREJ flag
			uint8_t requested_pdu[MAX_PDU] = {0};
			int pdu_size = retrieveFromWindowBuffer(requested_pdu, packet_num);
			createPDU(pdu, &requested_pdu[7], pdu_size, packet_num, 17); // Resent data packet flag
			sendtoErr(socketNum, (char *)requested_pdu, dataLen, 0, (struct sockaddr *) &client, clientAddrLen);
		}
	}

	return 0;
}




