#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "mj8x8.h"

// TODO - get rid of unions
typedef union															// u_devices union of bit fields and uint16_t - representation discovered devices on bus
{
	struct																// bit fields - one bit for each device on the bus
	{
		uint8_t _LU :1;													// 1 indicates device present, 0 otherwise
		uint8_t _DEV_0B :1;												//	ditto
		uint8_t _DEV_0C :1;												//	ditto
		uint8_t _MJ828 :1;												//	ditto
		uint8_t _DEV_1A :1;												//	ditto
		uint8_t _DEV_1B :1;												//	ditto
		uint8_t _DEV_1C :1;												//	ditto
		uint8_t _DEV_1D :1;												//	ditto
		uint8_t _MJ808 :1;												//	ditto
		uint8_t _MJ818 :1;												//	ditto
		uint8_t _DEV_2C :1;												//	ditto
		uint8_t _DEV_2D :1;												//	ditto
		uint8_t _DEV_3A :1;												//	ditto
		uint8_t _DEV_3B :1;												//	ditto
		uint8_t _DEV_3C :1;												//	ditto
		uint8_t _DEV_3D :1;												//	ditto
	};
	uint16_t All;														// the bit field as one uint16_t
} u_devices;

typedef struct															// canbus_t struct describing the CAN bus state
{
	u_devices devices;													// indicator of devices discovered, 16 in total; B0 - 1st device (0A), B1 - 2nd device (0B), ..., B15 - 16th device (3D)
	uint8_t NumericalCAN_ID ;											// ordered device number - A0 (0th device) until 3C (15th device), used in Heartbeat()
	uint8_t BeatIterationCount : 4;										// how many times did we wakeup, sleep and wakeup again
	uint8_t FlagDoHeartbeat : 1;
	uint8_t FlagDoDefaultOperation : 2;
} canbus_t;

typedef struct message_handler_t										// sends and receives (stores) CAN messages, keeps track of bus status
{																		//	is message-agnostic -> the actual device has to know what to do with a message (via PopulatedBusOperation() )
	volatile can_msg_t *msg;											// container for outbound messages
	volatile can_t *can;												// pointer to CAN infrastructure
	volatile canbus_t *bus;												// prime candidate for a private data member

	void (*SendMessage)(volatile struct message_handler_t *self, const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len);	// sends message out to CAN bus
	volatile can_msg_t* (*ReceiveMessage)(volatile struct message_handler_t *self);		// downloads received message by local CAN IC
} message_handler_t;

void message_handler_ctor(volatile message_handler_t *self, volatile can_t *in_can, volatile canbus_t *in_bus, volatile can_msg_t *msg);

extern volatile canbus_t BUS;											// declare canbus_t object
extern volatile message_handler_t MsgHandler;							// declare message handler object

extern volatile can_msg_t msg;											// declare message object for inbound messages

#endif /* MESSAGE_H_ */