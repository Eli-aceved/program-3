#ifndef BUFFMGMNT_H
#define BUFFMGMNT_H

/* Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "networks.h"
#include "cpe464.h"
#include "rcopyCBuffer.h"
#include "helperfuncs.h"
#include "safeUtil.h"

/* Defines */
#define MAX_PDU 1407

/* Function Prototypes */
int dataMngmntFSM(int socketNum, FILE *file);
int in_order_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest);
int buff_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest);
int flush_state(int socketNum, FILE *file, uint32_t *expected, uint32_t *highest);

#endif