#ifndef RCOPYCBUFFER_H
#define RCOPYCBUFFER_H

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
struct rcopyBuffer {
    struct packetinBuffer *packets;
    uint32_t lowest_pktnum; // Keeps track of the next packet to write to disk
    uint32_t size;
};

struct packetinBuffer {
    uint8_t packet[MAX_PDU];   // Holds entire PDU
    uint8_t validFlag;          // 0 = invalid, 1 = valid
    uint32_t packet_num;        // Sequence number (host order)
    uint32_t size;              // Size of PDU
};


/* Function Prototypes */
void initBuffer(int window_size);
int getPacketNum(uint8_t *packet);
uint32_t getNextPacketNum();
void storetoBuffer(uint8_t *packet, uint32_t packet_size);
int retrieveNextPacket(uint8_t *packet);
int isPacketValid(uint32_t packet_num);
void freeRcopyBuffer();


#endif