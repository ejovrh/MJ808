#include <inttypes.h>

#include "message.h"
//#include "mj8x8.h"

 //static volatile canmsg_handler_t *self = &message;

void _SendMessage(volatile message_handler_t *self, const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	if (in_command == CMND_ANNOUNCE)									// if we have the broadcast command
		self->out->sidh |= BROADCAST;									//	then set the broadcast flag

	self->out->COMMAND = in_command;									// set command into message
	self->out->ARGUMENT = in_argument;									// set argument into message
	self->out->dlc = in_len;											// set DLC

	self->can->RequestToSend(self->out);								// load message into TX buffer and request to send
};

void _ReceiveMessage(volatile struct message_handler_t *self)
{
	self->can->FetchMessage(self->in);
	self->bus->devices.uint16_val |= ( 1 << ( (self->in->sidh >> 2) & 0x0F ) );
};

void message_handler_ctor(volatile message_handler_t *self, volatile can_t *in_can, volatile canbus_t *in_bus, volatile can_msg_t *in_msg, volatile can_msg_t *out_msg)
{
	self->can = in_can;
	self->bus = in_bus;

	self->in = in_msg;
	self->out = out_msg;

	self->ReceiveMessage = &_ReceiveMessage;
	self->SendMessage = &_SendMessage;

	return self;
};