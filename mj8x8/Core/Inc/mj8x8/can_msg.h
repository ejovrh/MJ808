#ifndef CORE_INC_MJ8X8_CAN_MSG_H_
#define CORE_INC_MJ8X8_CAN_MSG_H_

#include <inttypes.h>

#pragma once

#define CAN_MAX_MSG_LEN 8	// 8 bytes are the max. length of a CAN message
#define COMMAND data[0]
#define ARGUMENT data[1]

typedef struct	// can_msg_t struct describing a generic CAN message
{
	// preserve byte order for sequential reads/writes
	uint8_t sidh;  // Standard Identifier High Byte
	uint8_t sidl;  // Standard Identifier Low Byte
	uint8_t eid8;  // Extended Identifier - not used here; just for padding so that read_rx_buffer() can read 13 bytes in one row
	uint8_t eid0;  // Extended Identifier - not used here
	uint8_t dlc;	// Data Length Code
	uint8_t data[CAN_MAX_MSG_LEN];	// Data, length identified by DLC
	uint8_t rx_status;	// holds RX and TX status metadata
// preserve byte order for sequential reads/writes
} can_msg_t __attribute__((aligned(8)));

#endif /* CORE_INC_MJ8X8_CAN_MSG_H_ */
