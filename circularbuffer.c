/**
 * Circular Buffer
 * File: circularbuffer.c
 * 
 * Program Description: This file contains functions to initialize, enqueue, dequeue, 
 * and display elements in a circular buffer. The buffer follows 
 * a FIFO structure, ensuring efficient PDU management.
 * 
 * Author: Elizabeth Acevedo
 * Date created: 03/01/2025
 * 
 */

#include "circularbuffer.h"

void initBuffer(CircularBuffer *cb, int capacity) {
    cb->window = (Buffer *)malloc(capacity * sizeof(Buffer));
    if (!cb->window) {
        perror("Malloc failed");
        exit(EXIT_FAILURE);
    }
    cb->front = -1;     // -1, Indicates that buffer is empty
    cb->rear = -1;
    cb->size = 0;
    cb->capacity = capacity;
}

int isFull(CircularBuffer *cb) {
    return cb->size == cb->capacity;
}

int isEmpty(CircularBuffer *cb) {
    return cb->size == 0;
}

void enqueue(CircularBuffer *cb, uint8_t *data, int data_len) {
    /* When enqueueing, rear is incremented (circularly) to point to the 
    next available slot, while front stays at the first element */
    //printf("[Before Enqueue] Size: %d, Front: %d, Rear: %d, Window Index: %d\n", cb->size, cb->front, cb->rear, cb->window_indx);
    if (isFull(cb)) {
        printf("Buffer is full!\n");
        return;
    }
    if (isEmpty(cb)) {
        cb->front = 0;  // Reset front index if buffer was empty
    }
    /* Move rear forward & assign window index */
    cb->rear = (cb->rear + 1) % cb->capacity;   // Update rear index
    cb->window_indx = cb->rear; // Update window index
    
    /* Store data in buffer */
    memcpy(cb->window[cb->window_indx].buff, data, data_len);   // Copy data to buffer
    cb->size++; // Increment buffer size

    printf("Inserted PDU at index %d, Buffer size: %d\n", cb->rear, cb->size);
    //printf("[After Enqueue] Size: %d, Front: %d, Rear: %d, Window Index: %d\n\n\n", cb->size, cb->front, cb->rear, cb->window_indx);
}

uint8_t *dequeue(CircularBuffer *cb) {
    /* When dequeueing, the front pointer is moved forward (circularly), 
    and if there are no more elements, front and rear are both reset to -1, 
    marking the buffer as empty again. */
    //printf("[Before Dequeue] Size: %d, Front: %d, Rear: %d, Window Index: %d\n", cb->size, cb->front, cb->rear, cb->window_indx);
    if (isEmpty(cb)) {
        printf("Buffer is empty!\n");
        return NULL;
    }
    uint8_t *data = cb->window[cb->front].buff; // Get data from buffer
    if (cb->front == cb->rear) { // If only one element was in buffer
        cb->front = -1;
        cb->rear = -1;
    }
    else {
        cb->front = (cb->front + 1) % cb->capacity; // Update front index
    }

    if (cb->size > 0){  // Ensure buffer size is not negative
        cb->size--; // Decrement buffer size
    }
    printf("Dequeued PDU, Buffer size: %d\n", cb->size);
    //printf("[After Dequeue] Size: %d, Front: %d, Rear: %d, Window Index: %d\n\n\n", cb->size, cb->front, cb->rear, cb->window_indx);
    return data;
}

// Display buffer status (for debugging)
void display(CircularBuffer *cb) {
    if (isEmpty(cb)) {
        printf("Buffer is empty!\n");
        return;
    }
    printf("\nBuffer size: %d, Front index: %d, Rear index: %d\n", cb->size, cb->front, cb->rear);

    printf("Buffer indexes: ");
    int i = cb->front;
    for (int count = 0; count < cb->size; count++) {
        printf("%d ", i);
        i = (i + 1) % cb->capacity;
    }
    printf("\n\n");
}

// Free allocated buffer memory
void freeBuffer(CircularBuffer *cb) {
    free(cb->window);
}

int main() {
    int window_size = 5;
    CircularBuffer cb;
    initBuffer(&cb, window_size);

    // Sample PDUs to store
    uint8_t sample_pdu1[MAX_PDU] = {1, 2, 3, 4, 5};
    uint8_t sample_pdu2[MAX_PDU] = {6, 7, 8, 9, 10};
    uint8_t sample_pdu3[MAX_PDU] = {11, 12, 13, 14, 15};
    uint8_t sample_pdu4[MAX_PDU] = {16, 17, 18, 19, 20};
    uint8_t sample_pdu5[MAX_PDU] = {21, 22, 23, 24, 25};
    uint8_t sample_pdu6[MAX_PDU] = {26, 27, 28, 29, 30};

    printf("Enqueueing PDUs:\n");
    enqueue(&cb, sample_pdu1, 5);
    enqueue(&cb, sample_pdu2, 5);
    enqueue(&cb, sample_pdu3, 5);
    enqueue(&cb, sample_pdu4, 5);
    enqueue(&cb, sample_pdu5, 5);

    display(&cb);

    printf("\nAttempting to enqueue when buffer is full:\n");
    enqueue(&cb, sample_pdu6, 5); // Should print "Buffer is full!"
    display(&cb);

    uint8_t *pdu = dequeue(&cb);
    printf("\nDequeued PDU first byte: %d\n", pdu[0]);

    display(&cb);

    freeBuffer(&cb);
    return 0;
}
