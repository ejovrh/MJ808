#ifndef CORE_INC_MJ8X8_CAN_H_
#define CORE_INC_MJ8X8_CAN_H_

#include <inttypes.h>
#include "can_msg.h"

typedef union  // union for core activity indication (is exposed to end device via mj8x8_t)
{
	// will be type-redefined in the end device (e.g. mj808_activity_t)
	struct
	{
		// CAN has to be active - 0x0F - lower nibble
		uint8_t _0 :1;  // bit 0
		uint8_t _1 :1;  // bit 1
		uint8_t _2 :1;  // bit 2
		uint8_t _3 :1;  // bit 3

		// CAN can be in standby mode - 0xF0 - upper nibble
		uint8_t CANActive :1;  // bit 4
		uint8_t _5 :1;	// bit 5
		uint8_t _6 :1;  // bit 6
		uint8_t _7 :1;  // bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} activity_t;

typedef struct can_t	// can_t struct describing the CAN device as a whole
{
	uint8_t own_sidh;  // SIDH for device
	uint8_t own_sidl;  // SIDL for device
	activity_t *activity;  // device activity indicator

	void (*const BusActive)(const uint8_t in_val);	// puts the CAN infrastructure to sleep (and wakes it up)
	void (*const RequestToSend)(can_msg_t *const msg);  // requests message to be sent to the CAN bus
	void (*const FetchMessage)(can_msg_t *msg);  // fetches received message from some RX buffer
} can_t __attribute__((aligned(8)));

can_t* can_ctor();	// CAN object constructor - does function pointer & hardware initialisation

#endif /* CORE_INC_MJ8X8_CAN_H_ */
