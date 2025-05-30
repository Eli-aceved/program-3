#ifndef BUFFMGMNT_H
#define BUFFMGMNT_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "networks.h"

/* Defines */
#define MAX_PDU 1407

/* Function Prototypes */
void dataMngmntFSM(int socketNum, FILE *file);
int in_order_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest);


#endif