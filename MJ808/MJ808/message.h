#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "mcp2515.h"

typedef struct															// canbus_t struct describing the CAN bus state
{
	uint8_t NumericalCAN_ID ;											// ordered device number - A0 (0th device) until 3C (15th device), used in Heartbeat()
	uint8_t BeatIterationCount : 4;										// how many times did we wakeup, sleep and wakeup again
	uint8_t FlagDoHeartbeat : 1;										// shall the heartbeat be initiated?
	uint8_t FlagDoDefaultOperation : 2;									// we are alone on the bus - shall we do our device-specific default operation?
	uint8_t placeholder : 1;											// empty space
} canbus_t;

typedef struct message_handler_t										// sends and receives (stores) CAN messages, keeps track of bus status
{																		//	is message-agnostic -> the actual device has to know what to do with a message (via PopulatedBusOperation() )
	volatile canbus_t *bus;												// prime candidate for a private data member
	uint16_t (* const GetDevices)(void);								// returns variable with bit-wise representation of devices on bus
	void (* const SendMessage)(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len);	// sends message out to CAN bus
	volatile can_msg_t* (* const ReceiveMessage)(void);					// downloads received message by local CAN IC
} message_handler_t;

void message_handler_ctor(can_t * const in_can);

extern message_handler_t * const MsgHandler;							// declare pointer to public struct part

#endif /* MESSAGE_H_ */