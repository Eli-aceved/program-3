/*********************************************************************************************************************
 * Circular Buffer for rcopy (client)
 * File: rcopyCBuffer.c
 * 
 * Program Description: This file is the CLIENT'S circular buffer meant to 
 * store packets while waiting for missing packets, used for error recovery. If there are any missing packets, 
 * this places the packets out of order keeping packets from being written to the disk 
 * until all the packets within the window are received.
 * The buffer for rcopy tracks the lowest unprocessed sequence/packet number 
 * (so it keeps a check on the order of packets ensuring they get written to disk in order).
 * 
 * Note: Rcopy requests missing packets using SREJ and acknowledges packets using RR. 
 *       Rcopy doesn't send data, it only sends RR acknoledgements and retranmission requests (SREJ) to the server.
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/06/2025
 * 
 *********************************************************************************************************************/

/* Includes */
#include "rcopyCBuffer.h"

/* Global Variable */
struct rcopyBuffer *rcopybuff;

/* Initialize rcopy Circular Buffer */
void initBuffer(int window_size) {
    // Allocate memory for the circular buffer structure
    rcopybuff = (struct rcopyBuffer*) malloc(sizeof(struct rcopyBuffer));
    // Error Check
    if (!rcopybuff) {
        perror("Malloc for rcopyBuffer failed");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the packets in the buffer
    rcopybuff->packets = (struct packetinBuffer*) malloc(sizeof(struct packetinBuffer) * window_size);
    // Error Check
    if (!rcopybuff->packets) {
        perror("Malloc for packetinBuffer failed");
        free(rcopybuff);
        exit(EXIT_FAILURE);
    }

    // Initializations
    rcopybuff->lowest_pktnum = 0xFFFFFFFF; // Set to max value
    rcopybuff->size = window_size;

    // Initialize each packet in the buffer
    for (int32_t i = 0; i < window_size; i++) {
        rcopybuff->packets[i].validFlag = 0;  // Invalid (slot is unused)
        rcopybuff->packets[i].packet_num = 0; // Set to 0
        rcopybuff->packets[i].size = 0;       // Set to 0
    }

}

/* Extracts Packet/Sequence Number from PDU */
int getPacketNum(uint8_t *packet) {
    // Extract packet number from packet
    uint32_t packetnum_netword = 0;
    // Copy packet number from packet
    memcpy(&packetnum_netword, packet, PACKETNUM_BYTES);
    // Convert to host order  (from network order)
    uint32_t packetnum_hostord = ntohl(packetnum_netword);

    return packetnum_hostord;
}

/* Returns packet sequence/number of next packet to be flushed */
uint32_t getNextPacketNum() {
	return rcopybuff->lowest_pktnum;
}

/* Returns whether packet with packet_num */
int isPacketValid(uint32_t packet_num) {

	// Calculate the index of buffer to store packet at
	int buf_indx = packet_num % rcopybuff->size;

	return rcopybuff->packets[buf_indx].validFlag;
}

/* Stores Out-of-Order Packets Into the Circular Buffer */
void storetoBuffer(uint8_t *packet, uint32_t packet_size) {
    // Extract packet number from packet
    uint32_t packet_num = getPacketNum(packet);

    // Indexing the buffer (this line is what makes buffer circular)
    uint32_t buff_indx = packet_num % rcopybuff->size;

    // Store packet and other packet info into buffer
    memcpy(rcopybuff->packets[buff_indx].packet, packet, packet_size);
    rcopybuff->packets[buff_indx].validFlag = 1;
    rcopybuff->packets[buff_indx].packet_num = packet_num;
    rcopybuff->packets[buff_indx].size = packet_size;

    // Update lowest packet number in rcopy buffer
    if (packet_num < rcopybuff->lowest_pktnum) {
        rcopybuff->lowest_pktnum = packet_num;
    }
}


/* Retrieve Next Packet from Buffer that Will Get Written to Disk */
int retrieveNextPacket(uint8_t *dest_buff) {
    // Check if buffer is empty
    if (rcopybuff->lowest_pktnum == 0xFFFFFFFF) {
        printf("Buffer is empty\n");
    }

    uint32_t next_packet_num = rcopybuff->lowest_pktnum;

    // Indexing the buffer (calculation of index that packet will be stored to)
    uint32_t buff_indx = next_packet_num % rcopybuff->size;

    // Copy packet to destination buffer
    memcpy(dest_buff, rcopybuff->packets[buff_indx].packet, rcopybuff->packets[buff_indx].size);

    rcopybuff->packets[buff_indx].validFlag = 0; // Mark packet as invalid (packet has been processed)

    // Find the next smallest valid packet
    uint32_t next_lowest = 0xFFFFFFFF;
    
    for (int32_t i = 0; i < rcopybuff->size; i++) {
        if (rcopybuff->packets[i].validFlag == 1 && rcopybuff->packets[i].packet_num < next_lowest) {
            next_lowest = rcopybuff->packets[i].packet_num;
        }
    }
    // Update lowest packet number to the next available packet or reset if no packets are available
    next_packet_num = next_lowest;
    

    return rcopybuff->packets[buff_indx].size;

}

/* Free rcopy Circular Buffer */
void freeRcopyBuffer() {
    free(rcopybuff->packets);
    free(rcopybuff);
}


