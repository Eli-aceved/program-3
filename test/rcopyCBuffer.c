/**
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
 */

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

/* Extracts Packet Number from PDU */
int getPacketNum(uint8_t *packet) {
    // Extract packet number from packet
    uint32_t packetnum_netword = 0;
    // Copy packet number from packet
    memcpy(&packetnum_netword, packet, PACKETNUM_BYTES);
    // Convert to host order  (from network order)
    uint32_t packetnum_hostord = ntohl(packetnum_netword);

    return packetnum_hostord;
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

    // Update lowest packet number
    if (packet_num < rcopybuff->lowest_pktnum) {
        rcopybuff->lowest_pktnum = packet_num;
    }
}


/* Write Packet to Disk Once Having Received the Missing Packet */
void writePDUtoDisk(uint32_t packet_num, uint8_t *target_buffer, uint32_t buff_indx) {
    // Copy packet to leaving_packet that will get written to disk
    memcpy(target_buffer, rcopybuff->packets[buff_indx].packet, rcopybuff->packets[buff_indx].size);

    rcopybuff->packets[buff_indx].validFlag = 0;
    /*
    // Write packet to disk
    // Open file
    FILE *fp = fopen("output.txt", "a");
    // Error check
    if (!fp) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    // Write packet to file
    fwrite(packet, sizeof(packet), 1, fp);
    // Close file
    fclose(fp);
    */

}

/* Retrieve Packet from Buffer 
void retrieveFromBuffer(uint32_t packet_num, uint8_t *packet, uint8_t *target_buffer) {
    // Indexing the buffer (this line is what makes buffer circular)
    uint32_t buff_indx = packet_num % rcopybuff->size;

    // Check if packet is in buffer
    if (rcopybuff->packets[buff_indx].validFlag == 1 && rcopybuff->packets[buff_indx].packet_num == rcopybuff->lowest_pktnum) {
        // Write to disk
        writePDUtoDisk(packet_num, target_buffer, buff_indx);
        
        // Update next lowest packet number (only if valid and packet number is less than new lowest)
        for (int32_t i = 0; i < rcopybuff->size; i++) {
            if (rcopybuff->packets[i].validFlag == 1 && rcopybuff->packets[i].packet_num > rcopybuff->lowest_pktnum) {
                rcopybuff->lowest_pktnum = rcopybuff->packets[i].packet_num;
            }
        }
        // If no packets are available, reset `lowest_pktnum` to max value
        rcopybuff->lowest_pktnum = 0xFFFFFFFF;
        
    }
}
*/

/* Retrieve Next Packet from Buffer that Will Get Written to Disk */
void retrieveNextPacket(uint8_t *packet) {
    // Check if buffer is empty
    if (rcopybuff->lowest_pktnum == 0xFFFFFFFF) {
        printf("Buffer is empty\n");
    }

    // Indexing the buffer (this line is what makes buffer circular)
    uint32_t buff_indx = rcopybuff->lowest_pktnum % rcopybuff->size;

    // Write to disk
    writePDUtoDisk(rcopybuff->lowest_pktnum, packet, buff_indx);

    // Find the next smallest valid packet
    uint32_t next_lowest = 0xFFFFFFFF;
    for (int32_t i = 0; i < rcopybuff->size; i++) {
        if (rcopybuff->packets[i].validFlag == 1 && rcopybuff->packets[i].packet_num < next_lowest) {
            next_lowest = rcopybuff->packets[i].packet_num;
        }
    }
    // Update lowest packet number to the next available packet or reset if no packets are available
    rcopybuff->lowest_pktnum = next_lowest;
}

/* Free rcopy Circular Buffer */
void freeRcopyBuffer() {
    free(rcopybuff->packets);
    free(rcopybuff);
}

/* Print data structure for testing/debugging */
void printBuffer() {
	printf("\nSmallest Packet Sequence Number: %u\n", rcopybuff->lowest_pktnum);
	for (int i = 0; i < rcopybuff->size; i++) {
		printf("Index %d:\n", i);
		printf("\tValid: %d\n", rcopybuff->packets[i].validFlag);
		printf("\tSize: %d\n", rcopybuff->packets[i].size);
		printf("\tPacket Seq Num: %u\n", rcopybuff->packets[i].packet_num);
	}

}

void testStorePkt(int seq_num, int size) {
	uint8_t packet[size];
	uint32_t pkt_num = htonl(seq_num);
	memcpy(packet, &pkt_num, sizeof(pkt_num));

	storetoBuffer(packet, size);
}

int main(int argc, char *argv[]) {
	initBuffer(6);

    testStorePkt(1, 1111);
	testStorePkt(2, 50);
	testStorePkt(3, 1000);
	testStorePkt(5, 1234);
    testStorePkt(9, 90);
    testStorePkt(12, 12);

	uint8_t packet[MAX_PDU];
	
	printBuffer(); // 

	retrieveNextPacket(packet); // 

	printBuffer();

	retrieveNextPacket(packet); // 

	printBuffer();

	retrieveNextPacket(packet); // 

	printBuffer();

	freeRcopyBuffer();

	return 0;
}

