/**
 * Buffer Management Finite State Machine for rcopy (client)
 * File: buffmgmnt.c
 * 
 * Program Description: 
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/05/2025
 * 
 */

/* Includes */
#include "buffmgmnt.h"
#include ""

/* Buffer FSM Initialization */
enum BuffManagementState{
    CORRECT = 1,    // rcopy receives correct/in order PDU
    BUFFER = 2,     // PDU received is greater than expected
    FLUSH = 3       // Flush buffer (write to disk) when you receive the expected PDU that was missing
};

enum BuffManagementState state = CORRECT;

/* Global Variables */

/* Prototypes */
void bufferMngmntFSM();

/* FSM */
void bufferMngmntFSM() {
    switch (state) {
        case CORRECT:
            if (recvdPDU == expectedPDU) {
                // Write to disk
                writePDUtoDisk(recvdPDU);
                expectedPDU++;
                // RR expected
            }
            else if (recvdPDU > expectedPDU) {  // If recv an out of order packet
                // SREJ expected
                storetoBuffer(recvdPDU);
                highest_recv = recvdPDU;
                state = BUFFER;
            }
            break;

        case BUFFER:
            if (recvdPDU > expectedPDU) {   // Stays at state until recvd lost packet
                storetoBuffer(recvdPDU);
                buffer[seqNum % window_size].validFlag = 1;
                highest_recv = recvdPDU;
            }
            else if (recvdPDU == expectedPDU) {
                // Write to disk
                writePDUtoDisk(recvdPDU);
                buffer[seqNum % window_size].validFlag = 0;
                expectedPDU++;
                state = FLUSH;
            }
            break;

        case FLUSH:
            while (packetNum == expectedPDU && valid) {  // expected & valid in the buffer
                // Write to disk
                writePDUtoDisk(packetNum);
                expectedPDU++;
            }
            if (expectedPDU < highest_recv && !valid) {
                // SREJ expected
                // RR expected
                state = BUFFER;
            }
            else if (expectedPDU == highest_recv) { // When buffer is empty
                // Write to disk
                writePDUtoDisk(expectedPDU);
                expectedPDU++;
                // RR expected
                state = CORRECT;
            }
            break;

        default:
            break;
            
    }
}
