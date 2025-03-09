#ifndef RCOPYCBUFFER_H
#define RCOPYCBUFFER_H

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
void storetoBuffer(uint8_t *packet, uint32_t packet_size);
void writePDUtoDisk(uint32_t packet_num, uint8_t *leaving_packet, uint32_t buff_indx);
void retrieveFromBuffer(uint32_t packet_num, uint8_t *packet, uint8_t *leaving_packet);
void testStorePkt(int seq_num, int size);
void printBuffer();
void freeRcopyBuffer();


#endif