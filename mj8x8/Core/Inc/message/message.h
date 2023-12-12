#ifndef CORE_INC_MESSAGE_MESSAGE_H_
#define CORE_INC_MESSAGE_MESSAGE_H_

#include "mj8x8/mj8x8_commands.h"	// device_t is defined here, amongst other things

typedef struct message_handler_t	// sends and receives (stores) CAN messages, keeps track of bus status
{  //
	device_t *Devices;  // pointer to indicator of devices discovered, see device_t definition

	void (*const SendMessage)(const mj8x8_Devices_t in_rcpt, const uint8_t in_command, const uint8_t *in_payload, const uint8_t in_len);  // loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
	can_msg_t* (*const GetMessage)(void);  // RX - returns reference to received message stored in message handler object
	void (*const SetMessage)(const can_msg_t *const msg);  // RX - upload CAN message into self at physical reception from FIFO
} message_handler_t;

void message_handler_ctor(can_t *const in_can);  // the message handler constructor

extern message_handler_t *const MsgHandler;  // declare pointer to public struct part

#endif /* CORE_INC_MESSAGE_MESSAGE_H_ */
