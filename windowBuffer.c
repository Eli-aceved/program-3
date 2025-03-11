/**
 * Circular Buffer
 * File: windowBuffer.c
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
    windowbuff->lowest_pktnum = 0;
    windowbuff->highest_pktnum = window_size;
    windowbuff->current_pktnum = 0;
    windowbuff->size = window_size;

    // Initialize each packet in the buffer
    for (int32_t i = 0; i < window_size; i++) {
        windowbuff->packets[i].packet_num = 0; // Set to 0
        windowbuff->packets[i].size = 0;       // Set to 0
    }
}

 /* Store Packet Into the Circular Buffer */
void storetoWindowBuffer(uint8_t *packet, uint32_t packet_size) {
    // Extract packet number from packet
    uint32_t packet_num = 0; // Initialize to 0
    memcpy(&packet_num, &packet[0], sizeof(packet_num));
    packet_num = ntohl(packet_num);

    // Indexing the buffer (this line is what makes buffer circular)
    uint32_t buff_indx = packet_num % windowbuff->size;

    // Store packet and other packet info into buffer
    memcpy(windowbuff->packets[buff_indx].packet, packet, packet_size);
    windowbuff->packets[buff_indx].packet_num = packet_num;
    windowbuff->packets[buff_indx].size = packet_size;

    // Update current packet number
    windowbuff->current_pktnum = packet_num + 1; // Increments by 1 for next packet since it's initialized to 0
}


/* Retrieve Packet from Buffer */
int retrieveFromWindowBuffer(uint8_t *dest_buffer, uint32_t packet_num) {
    // Indexing the buffer (this line is what makes buffer circular)
    uint32_t buff_indx = packet_num % windowbuff->size;

    // Check if packet is in buffer
    if (windowbuff->packets[buff_indx].packet_num == packet_num) {
        // Copy packet to destination buffer
        memcpy(dest_buffer, windowbuff->packets[buff_indx].packet, windowbuff->packets[buff_indx].size);
    }

    return windowbuff->packets[buff_indx].size;
}


void retrieveLowestPacket(uint8_t *dest_buffer) {
    retrieveFromWindowBuffer(dest_buffer, windowbuff->lowest_pktnum);
}

uint32_t getCurrent() {
    return windowbuff->current_pktnum;
}

/* Check if Window is Closed */
int windowisClosed() {
    if (windowbuff->current_pktnum == windowbuff->highest_pktnum) {
        return 1;
    }
    return 0;
}

/* Slide Window */
void slideWindow(uint32_t RR_packetnum) {
    // Determine slide amount
    uint32_t slide_amount = (RR_packetnum) - (windowbuff->lowest_pktnum);

    // Slide window by updating the lowest and highest packet numbers
    windowbuff->lowest_pktnum += slide_amount;
    windowbuff->highest_pktnum += slide_amount;
}

/* Free rcopy Circular Buffer */
void freewindowbuffer() {
    free(windowbuff->packets);
    free(windowbuff);
}

