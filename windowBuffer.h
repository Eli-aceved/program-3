/**
 * Circular Buffer Header File
 * File: circularbuffer.h
 *
 * Program Description: This header defines the circular buffer structure and function prototypes 
 * for enqueueing, dequeueing, and managing protocol data units (PDUs).
 * 
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/01/2025
 * 
 */

#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

/* Includes */
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Definitions */
#define MAX_PDU 1407    // Max size for a PDU (header(7) + payload(1400))

/* Circular buffer structure for storing PDUs */
typedef struct {
    uint8_t buff[MAX_PDU];
} Buffer;

/* Circular buffer structure for managing PDUs */
typedef struct {
    Buffer *window;
    int front;
    int rear;
    int size;
    int capacity;
    int window_indx; 
} CircularBuffer;

/* Function Prototypes */
void initBuffer(CircularBuffer *cb, int capacity);
int isFull(CircularBuffer *cb);
int isEmpty(CircularBuffer *cb);
void enqueue(CircularBuffer *cb, uint8_t *data, int data_len);
uint8_t *dequeue(CircularBuffer *cb);
void display(CircularBuffer *cb);
void freeBuffer(CircularBuffer *cb);

#endif 
