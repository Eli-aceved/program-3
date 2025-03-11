/**
 * Helper functions for rcopy and server packet creation header
 * File: helperfuncs.h
 * 
 * Program Description: This file is the header file for helperfuncs.c
 * Author: Elizabeth Acevedo
 * Date created: 03/11/2025
 * 
 */

#ifndef HELPERFUNCS_H
#define HELPERFUNCS_H

/* Includes */
#include <stdint.h>
#include <arpa/inet.h> // For htonl() and ntohl() conversions
#include <string.h>
#include <stdio.h>

#define PACKETNUM_BYTES 4   // Number of bytes for packet number in PDU

/* Function Prototypes */
void createPDU(uint8_t *pdu, uint8_t *data_chunk, uint16_t data_size, uint32_t packet_num, uint8_t flag);


#endif