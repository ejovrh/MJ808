#ifndef CORE_INC_MJ8X8_CAN_MSG_H_
#define CORE_INC_MJ8X8_CAN_MSG_H_

#pragma once

#define CAN_MAX_MSG_LEN 8	// 8 bytes are the max. length of a CAN message
#define COMMAND data[0]
#define ARGUMENT data[1]

typedef struct	// can_msg_t struct describing a generic CAN message
{
	uint16_t sid;  // Standard Identifier word
	uint8_t dlc;	// Data Length Code
	uint8_t data[CAN_MAX_MSG_LEN];	// Data, length identified by DLC
} can_msg_t __attribute__((aligned(8)));

#endif /* CORE_INC_MJ8X8_CAN_MSG_H_ */
