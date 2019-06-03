#ifndef MESSAGE_H_
#define MESSAGE_H_

#include "mj8x8.h"
#include "mcp2515.h"

//typedef struct message_handler_t
//{
	//volatile can_msg_t *in;
	//volatile can_msg_t *out;
	//volatile can_t *can;
	//volatile canbus_t *bus;
//
	//void (*SendMessage)(volatile struct message_handler_t *self, const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len);
	//void (*ReceiveMessage)(volatile struct message_handler_t *self);
//} message_handler_t;

void message_handler_ctor(volatile message_handler_t *self, volatile can_t *in_can, volatile canbus_t *in_bus, volatile can_msg_t *in_msg, volatile can_msg_t *out_msg);

#endif /* MESSAGE_H_ */