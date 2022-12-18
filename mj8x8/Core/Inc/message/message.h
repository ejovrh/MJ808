#ifndef CORE_INC_MESSAGE_MESSAGE_H_
#define CORE_INC_MESSAGE_MESSAGE_H_

#include "mj8x8\can.h"

typedef struct message_handler_t	// sends and receives (stores) CAN messages, keeps track of bus status
{  //	is message-agnostic -> the actual device has to know what to do with a message (via PopulatedBusOperation() )
	uint16_t Devices;  // indicator of devices discovered, 16 in total; B0 - 1st device (0A), B1 - 2nd device (0B), ..., B15 - 16th device (3D)

	void (*const SendMessage)(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len);  // sends message out to CAN bus
	volatile can_msg_t* (*const ReceiveMessage)(void);	// downloads received message by local CAN IC
} message_handler_t;

void message_handler_ctor(can_t *const in_can);  // the message handler constructor

extern message_handler_t *const MsgHandler;  // declare pointer to public struct part

#endif /* CORE_INC_MESSAGE_MESSAGE_H_ */
