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
#include "cpe464.h"
#include "windowBuffer.h"

#define MAXBUF 80
#define MAX_PAYLOAD 1400    // Max size for payload
#define PACKETNUM_BYTES 4   // Number of bytes for packet number in PDU

void processClient(int socketNum);
int checkArgs(int argc, char *argv[]);

int main(int argc, char *argv[])
{ 
	int socketNum = 0;				
	int portNumber = 0;

	portNumber = checkArgs(argc, argv);
	socketNum = udpServerSetup(portNumber);
	setupPollSet();
	addToPollSet(socketNum);
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
	float errorRate = 0.0;

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
		errorRate = atof(argv[0]);
		portNumber = atoi(argv[1]);
	}
	return errorRate;
	return portNumber;
}


size_t readFromFile(char *filename, uint16_t buffer_size, size_t data_chunk) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL || !file) {
        perror("Error opening file");
        return -1;
    }
    // Read from file
    size_t bytes_read = fread(data_chunk, 1, buffer_size, file);
    if (bytes_read == 0) {
        perror("Error reading from file");
        return 1;
    }
    if (bytes_read > 0) {
        createPDU(data_chunk, bytes_read, );
    }
    // Debugging: print data_chunk
    // Close file
    fclose(file);
}

uint8_t *createPDU(uint8_t *data_chunk, size_t bytes_read, uint32_t packet_num, uint8_t flag) {
	// Initialize PDU buffer
	uint8_t pdu[MAX_PDU] = {0};

	/* Add the header to the PDU */
	// Add a packet sequence number to the PDU (4-bytes)
	memcpy(pdu, &packet_num, PACKETNUM_BYTES);
	// Add checksum to the PDU (2 bytes)
	uint16_t checksum = 0; // Placeholder for checksum
	memcpy(pdu + PACKETNUM_BYTES, &checksum, sizeof(checksum));

	// Add flag to the PDU (1 byte)
	memcpy(pdu + PACKETNUM_BYTES + sizeof(checksum), &flag, sizeof(flag));

	// Handle different PDU types based on flag value
	if (flag == 5) {	//  RR packet
		// Add the RR sequence number to the PDU (4 bytes) which packet number are we RRing to?
		uint32_t RR_num = 0; // Placeholder for RR sequence number
		memcpy(pdu + PACKETNUM_BYTES + sizeof(checksum) + sizeof(flag), &RR_num, sizeof(RR_num));

	}
	else if (flag == 6) {	// SREJ packet
		// Add the SREJ sequence number to the PDU (4 bytes) which packet number are we SREJing to?
		uint32_t SREJ_num = 0; // Placeholder for SREJ sequence number
		memcpy(pdu + PACKETNUM_BYTES + sizeof(checksum) + sizeof(flag), &SREJ_num, sizeof(SREJ_num));

	}
	else if (flag == 8 || flag == 9 || flag == 10 || flag == 16) {  
		/* Flags: 
		Contains filename/buffer-size/window-size (rcopy to server)
		Contains the response to the filename packet (server to rcopy)
		EOF indication or is the last packet (server to rcopy)
		Data packet (server to rcopy)
		*/
		// Add relevant data to the payload
		memcpy(pdu + PACKETNUM_BYTES + sizeof(checksum) + sizeof(flag), data_chunk, bytes_read);

	}
	else {
		fprintf(stderr, "Error: Invalid PDU flag value %d\n", flag);
		return NULL;
	}

}

void serverWindowControl(int socketNum, FILE *file, int buffer_size, int window_size, struct sockaddr_in *client_addr, socklen_t client_len) {
	uint8_t flag = 16; // Default at data packet flag
	uint8_t data_chunk[MAX_PAYLOAD] = {0};

	// Initialize window
	initWindow(window_size);

	// Initialize counters
	int count = 0;
	// while not at EOF
	while (!feof(file) ) {
		// Check if window is open
		if (!windowisClosed) {
			// Read from file
			readFromFile(file, buffer_size, data_chunk);
			// Check if reached end of file
			if (feof(file)){
				flag == 10; // EOF flag
			}
			// Create PDU
			uint8_t *pdu = createPDU(data_chunk, buffer_size, count, flag);
			// Send packets from buffer
			sendtoErr(socketNum, pdu, buffer_size, 0, (struct sockaddr *) client_addr, client_len);
			
			// Poll for RRs/SREJs (non-blocking)
			while (pollCall(0)){
				// Process RRs/SREJs
				//processRRs();processSREJs();
			}
		}

		// While window is closed
		while (windowisClosed) {
			// If timeout occurs, resend lowest unacknowledged packet
			while (pollCall(1) ) {
				uint8_t *lowest_pdu = retrieveLowestPacket( ); // Get lowest packet
				// Resend lowest packet

				sendtoErr(socketNum, lowest_pdu, buffer_size, 0, (struct sockaddr *) client_addr, client_len);
				count++;
			}
			// Process RRs/SREJs
			//processRRs();processSREJs();
		}
		// free window buffer
		//freewindowbuffer();
	}

}

