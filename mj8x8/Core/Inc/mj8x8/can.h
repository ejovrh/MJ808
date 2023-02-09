#ifndef CORE_INC_MJ8X8_CAN_H_
#define CORE_INC_MJ8X8_CAN_H_

#include <inttypes.h>
#include "can_msg.h"

typedef struct can_t	// can_t struct describing the CAN device as a whole
{
	uint8_t own_sidh;  // SIDH for device
	uint8_t own_sidl;  // SIDL for device

	void (*const Sleep)(const uint8_t in_val);	// puts the MCP2515 to sleep (and wakes it up)
	void (*const RequestToSend)(can_msg_t *const msg);  // requests message to be sent to the CAN bus
	void (*const FetchMessage)(can_msg_t *msg);  // fetches received message from some RX buffer
} can_t __attribute__((aligned(8)));

can_t* can_ctor();	// CAN object constructor - does function pointer & hardware initialisation

#endif /* CORE_INC_MJ8X8_CAN_H_ */
