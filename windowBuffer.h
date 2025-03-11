/**
 * Window Buffer Header File
 * File: uffer.h
 *
 * Program Description: This header
 * 
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/01/2025
 * 
 * Modified: 03/10/2025
 * 
 */

#ifndef WINDOWBUFFER_H
#define WINDOWBUFFER_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <arpa/inet.h> // For htonl() and ntohl() conversions

/* Defines */
#define MAX_PDU 1407        // Max size for a PDU (header(7) + payload(1400))

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
void storetoWindowBuffer(uint8_t *packet, uint32_t packet_size);
void windowisClosed();
void freewindowbuffer();


#endif 
