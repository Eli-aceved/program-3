/**
 * Helper functions for rcopy and server packet creation
 * File: helperfuncs.c
 * 
 * Program Description: 
 * This file contains helper functions for creating PDUs for the use of rcopy and server.
 * Author: Elizabeth Acevedo
 * Date created: 03/11/2025
 * 
 */


#include "helperfuncs.h"
#include "cpe464.h"

int createPDU(uint8_t *pdu, uint8_t *data_chunk, uint16_t data_size, uint32_t packet_num, uint8_t flag) {
	// Convert packet number to network order
	//uint32_t packet_num_netword = htonl(packet_num);

	/* Add the header to the PDU */
	// Add a packet sequence number to the PDU (4-bytes)
	memcpy(pdu, &packet_num, PACKETNUM_BYTES);
    // Add temporary filler for checksum to the PDU (2 bytes)
	uint16_t checksum = 0; 
	memcpy(&pdu[4], &checksum, 2);

	// Add flag to the PDU (1 byte)
	memcpy(&pdu[6], &flag, 1);

    // Add data to the PDU
	memcpy(&pdu[7], data_chunk, data_size);

    // Calculate checksum and to the PDU (2 bytes)
    checksum = in_cksum((unsigned short *)pdu, data_size + 7);
    memcpy(&pdu[4], &checksum, 2);

	return data_size + 7; // Return the size of the PDU (data size + header size)
}


float getErrorRate(char *errate_entry) {
	// Convert error rate entry to float
	char *endptr;
	float errorRate = strtof(errate_entry, &endptr);

	// Check for conversion errors
	if (errate_entry == endptr) {
        	printf("Error: '%s' is not a valid float.\n", errate_entry);
        	return -1;
    }

    return errorRate;

}
