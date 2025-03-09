/**
 * Circular Buffer
 * File: circularbuffer.c
 * 
 * Program Description: This file is the SERVER'S circular buffer 
 * meant to track packets that have been sent but may need retransmission 
 * if a Selective-Reject (SREJ) is received by the client. The packets are 
 * stored in the buffer until the server receives an acknowledgment (RR) from the client.
 * The server tracks the oldest/lowest unacknowledged packet, current and most recent sent packet, and the highest .
 * 
 * 
 * reads from disk 
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/01/2025
 * 
 */

#include "windowBuffer.h"

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h> // For htonl() and ntohl() conversions

/* Defines */
#define MAX_PDU 1407        // Max size for a PDU (header(7) + payload(1400))
#define PACKETNUM_BYTES 4   // Number of bytes for packet number in PDU

/* Structs */
struct windowBuffer {
    struct packetinWBuffer *packets;
    uint32_t lowest_pktnum;  // Keeps track of the next packet to write to disk
    uint32_t highest_pktnum; // Keeps track of the highest packet
    uint32_t current_pktnum; // Keeps track of the current packet
    uint32_t size;
};

struct packetinWBuffer {
    uint8_t packet[MAX_PDU];   // Holds entire PDU
    uint8_t validFlag;          // 0 = invalid, 1 = valid
    uint32_t packet_num;        // Sequence number (host order)
    uint32_t size;              // Size of PDU
};

/* Function Prototypes */
void initWindow(int window_size);
void readFromFile(char *filename);
void storetoWindowBuffer(uint8_t *packet, uint32_t packet_size);
void createPDU(uint8_t *packet, uint32_t packet_size);
void sendACKPacketsFromBuff();
void freewindowbuffer();

/* Global Variable */
struct windowBuffer *windowbuff;

/* Initialize rcopy Circular Buffer */
void initWindow(int window_size) {
    // Allocate memory for the circular buffer structure
    windowbuff = (struct windowBuffer*) malloc(sizeof(struct windowBuffer));
    // Error Check
    if (!windowbuff) {
        perror("Malloc for windowbuffer failed");
        exit(EXIT_FAILURE);
    }

    // Allocate memory for the packets in the buffer
    windowbuff->packets = (struct packetinWBuffer*) malloc(sizeof(struct packetinWBuffer) * window_size);
    // Error Check
    if (!windowbuff->packets) {
        perror("Malloc for packetinBuffer failed");
        free(windowbuff);
        exit(EXIT_FAILURE);
    }

    // Initializations
    windowbuff->lowest_pktnum = 0xFFFFFFFF; // Set to max value
    windowbuff->highest_pktnum = 0;
    windowbuff->current_pktnum = 0;
    windowbuff->size = window_size;

    // Initialize each packet in the buffer
    for (int32_t i = 0; i < window_size; i++) {
        windowbuff->packets[i].validFlag = 0;  // Invalid (slot is unused)
        windowbuff->packets[i].packet_num = 0; // Set to 0
        windowbuff->packets[i].size = 0;       // Set to 0
    }
}

void readFromFile(char *filename) {
    FILE *fp;
}

 /* Store Packet Into the Circular Buffer */
void storetoWindowBuffer(uint8_t *packet, uint32_t packet_size) {
    // Extract packet number from packet
    uint32_t packet_num = getPacketNum(packet); // change to createPDU

    // Indexing the buffer (this line is what makes buffer circular)
    uint32_t buff_indx = packet_num % windowbuff->size;

    // Store packet and other packet info into buffer
    memcpy(windowbuff->packets[buff_indx].packet, packet, packet_size);
    windowbuff->packets[buff_indx].validFlag = 1;
    windowbuff->packets[buff_indx].packet_num = packet_num;
    windowbuff->packets[buff_indx].size = packet_size;

    // Update lowest packet number
    if (packet_num < windowbuff->lowest_pktnum) {
        windowbuff->lowest_pktnum = packet_num;
    }
}


/* Free rcopy Circular Buffer */
void freewindowbuffer() {
    free(windowbuff->packets);
    free(windowbuff);
}

