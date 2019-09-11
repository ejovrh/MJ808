#ifndef MCP2515_H_
#define MCP2515_H_

#include "can_msg.h"
#include "mcp2515_hw_defines.h"

typedef struct can_t													// can_t struct describing the CAN device as a whole
{
	uint8_t own_sidh;													// SIDH for device
	uint8_t own_sidl;													// SIDL for device

// public methods

	void (* const Sleep)(const uint8_t in_val);							// puts the MCP2515 to sleep (and wakes it up)
	void (* const RequestToSend)(volatile can_msg_t * const msg);		// requests message to be sent to the CAN bus
	void (* const FetchMessage)(volatile can_msg_t *msg);				// fetches received message from some RX buffer
	void (* const ChangeOpMode)(const uint8_t mode);					// changes the operational mode of the MCP2515
	void (* const ReadBytes)(const uint8_t addr, volatile uint8_t *data, const uint8_t len);		// reads len bytes from some register in the MCP2515
	void (* const BitModify)(const uint8_t addr, const uint8_t mask, const uint8_t byte);			// modifies bit identified by "byte" according to "mask" in some register
} can_t __attribute__((aligned(8)));

can_t *can_ctor();														// CAN object constructor - does function pointer & hardware initialization

#endif /* MCP2515_H_ */