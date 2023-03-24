#ifndef CORE_INC_MESSAGE_MESSAGE_H_
#define CORE_INC_MESSAGE_MESSAGE_H_

#include "mj8x8/mj8x8_commands.h"	// device_t is defined here, amongst other things

typedef struct message_handler_t	// sends and receives (stores) CAN messages, keeps track of bus status
{  //
	device_t *Devices;  // pointer to indicator of devices discovered, see device_t definition

	void (*const SendMessage)(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len);  // sends message out to CAN bus
	can_msg_t* (*const GetMessage)(void);  // downloads received message by local CAN IC
	void (*const SetMessage)(const can_msg_t *const msg);  // downloads received message by local CAN IC
} message_handler_t;

void message_handler_ctor(can_t *const in_can);  // the message handler constructor

extern message_handler_t *const MsgHandler;  // declare pointer to public struct part

#endif /* CORE_INC_MESSAGE_MESSAGE_H_ */
