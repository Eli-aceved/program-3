/**
 * Buffer Management Finite State Machine for rcopy (client)
 * File: buffmgmnt.c
 * 
 * Program Description: 
 * This file implements a finite state machine (FSM) for managing
 * the buffer that manages the packets that were sent to the client
 * 
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/05/2025
 * 
 */

/* Includes */
#include "buffmgmnt.h"



/* Buffer FSM Initialization */
typedef enum BuffManagementState{
    IN_ORDER,    // rcopy receives correct/in order PDU
    BUFFER,     // PDU received is greater than expected
    FLUSH       // Flush buffer (write to disk) when you receive the expected PDU that was missing
} State; 

/* Global Variables */
State curr_state = IN_ORDER;
int next = 0;

/* FSM */
void dataMngmntFSM(int socketNum, FILE *file) {
    uint32_t expected = 0;
	uint32_t highest = 0;

    switch (curr_state) {
        case IN_ORDER:
            next = in_order_state(socketNum, file, &expected, &highest);
            if (next == 2) {
                curr_state = BUFFER;
            }
            else if (next == 3) {
                printf("Download Complete\n");
                return 1;
            }
            break;

        case BUFFER:
            next = buff_state(socketNum, file, &expected, &highest);
            if (next == 2) {
                curr_state = FLUSH;
            }
            break;

        case FLUSH:
            next = flush_state(socketNum, file, &expected, &highest);
            if (next == 2) {
                curr_state = IN_ORDER;
            }
            else if (next == 4) {
                curr_state = BUFFER;
            }
            else if (next == 3) {
                printf("Download Complete\n");
                return 1;
            }
            break;

        default:
            break;
            
    }
}

int in_order_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest) {
	struct connectionInfo cinfo = getConnectionInfo();
	int serverAddrLen = sizeof(struct sockaddr_in6);
	int recv_packet_len = 0;
	int send_packet_len = 0;
	uint8_t recv_packet[MAX_PDU] = {0};
	uint8_t send_PDU[MAX_PDU] = {0};
	uint32_t expected_netord = 0;

	recv_packet_len = safeRecvfrom(socketNum, recv_packet, MAX_PDU, 0, (struct sockaddr *) cinfo.serverAddress, &serverAddrLen);

	if (in_cksum((uint16_t *)recv_packet, recv_packet_len) == 0) {

		uint32_t packet_num = extractPktNum(recv_packet);
		
		if (recv_packet[6] == 10) {
			fwrite(&recv_packet[7], 1, recv_packet_len - 7, file);
			expected_netord = htonl(*expected);
			send_packet_len = createPDU(send_PDU, 0, 32, (uint8_t *) &expected_netord, 1);

			sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));

			return 2;
		}

		if (packet_num == *expected) {

			fwrite(&recv_packet[7], 1, recv_packet_len - 7, file);

			*highest = *expected;
			(*expected)++;

			expected_netord = htonl(*expected);

			// RR expected
			send_packet_len = createPDU(send_PDU, 0, 5, (uint8_t *) &expected_netord, 4);
			sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));

			return 1;
		}
		else if (packet_num > *expected) {

			expected_netord = htonl(*expected);

			// SREJ expected
			send_packet_len = createPDU(send_PDU, 0, 6, (uint8_t *) &expected_netord, 4);
			sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));

			// Buffer received
			storePacket(recv_packet, recv_packet_len);

			*highest = packet_num;

			return 3;
		}
		else if (packet_num < *expected) {
			expected_netord = htonl(*expected);
			int send_packet_len = createPDU(send_PDU, (uint8_t *) &expected_netord, 4, 0, 5);

			sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));
		}
	}

	return 0;
}


int buff_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest) {
	
	struct connectionInfo cinfo = getConnectionInfo();
	int serverAddrLen = sizeof(struct sockaddr_in6);
	int recv_packet_len = 0;
	uint8_t send_PDU[MAX_PDU];
	uint8_t recv_packet[MAX_PDU];

	recv_packet_len =  safeRecvfrom(socketNum, recv_packet, MAX_PDU, 0, (struct sockaddr *) cinfo.serverAddress, &serverAddrLen);

	if (in_cksum((uint16_t *)recv_packet, recv_packet_len) == 0) {

		uint32_t packet_num = extractPktNum(recv_packet);

		if (packet_num > *expected) {

			storePacket(recv_packet, recv_packet_len);

			*highest = packet_num;

			return 1;

		}
		else if (packet_num == *expected) {
			fwrite(&recv_packet[7], 1, recv_packet_len - 7, file);
			(*expected)++;

			return 2;
		}
		else if (packet_num < *expected) {
			uint32_t expected_netord = htonl(*expected);
			int send_pkt_len = createPDU(send_PDU, (uint8_t *) &expected_netord, 4, 0, 5);
			sendtoErr(socketNum, send_PDU, send_pkt_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));
		}

	}


	return 0;
}

int flush_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest) {

	struct connectionInfo cinfo = getConnectionInfo();
	int flushed_packet_len = 0;
	int send_packet_len = 0;
	uint8_t send_PDU[MAX_PDU];
	uint8_t flushed_PDU[MAX_PDU];
	uint32_t expected_netord = 0;
	uint32_t next_packet = getNextPacketNum();


	if (*expected == *highest) {
		flushed_packet_len = retrieveNextPacket(flushed_PDU);

		if (flushed_PDU[6] == 10) {
			fwrite(&flushed_PDU[7], 1, flushed_packet_len - 7, file);
			return 3;
		}
		else {
			fwrite(&flushed_PDU[7], 1, flushed_packet_len - 7, file);
		}
		(*expected)++;
		expected_netord = htonl(*expected);

		// RR expected
		send_packet_len = createPDU(send_PDU, (uint8_t *) &expected_netord, 4, 0, 5);
		sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));

		return 2;

	}
	else if (next_packet == *expected) {
		flushed_packet_len = retrieveNextPacket(flushed_PDU);

		fwrite(&flushed_PDU[7], 1, flushed_packet_len - 7, file);
		(*expected)++;
		return 1;
	}
	else if (*expected < *highest && !isPacketValid(*expected)) {
		expected_netord = htonl(*expected);

		// SREJ is expected
		send_packet_len = createPDU(send_PDU, (uint8_t *) &expected_netord, 4, 0, 6);
		sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));		

		// Change to RR and send
		send_packet_len = createPDU(send_PDU, (uint8_t *) &expected_netord, 4, 0, 5);
		sendtoErr(socketNum, send_PDU, send_packet_len, 0, (struct sockaddr *) cinfo.serverAddress, sizeof(struct sockaddr_in6));

		return 4;
	}

	return 0;


}