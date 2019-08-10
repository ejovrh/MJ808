#include <inttypes.h>

#include "message.h"

// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(volatile message_handler_t *self, const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	// TODO - eventually consolidate two CAN message objects (in and out) into one and make sure own SID is set in outbound messages:
	//			msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A);		// high byte
	//			msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);																		// low byte

	if (in_command == CMND_ANNOUNCE)									// if we have the broadcast command
		self->out->sidh |= BROADCAST;									//	then set the broadcast flag

	self->out->COMMAND = in_command;									// set command into message
	self->out->ARGUMENT = in_argument;									// set argument into message
	self->out->dlc = in_len;											// set DLC

	self->can->RequestToSend(self->out);								// load message into TX buffer and request to send
};

// fetches message received from CAN bus by local CAN IC, populates known hosts and returns message handler object
volatile can_msg_t *_ReceiveMessage(volatile struct message_handler_t *self)
{
	self->can->FetchMessage(self->in);									// fetch the message from some RX buffer
	self->bus->devices.uint16_val |= ( 1 << ( (self->in->sidh >> 2) & 0x0F ) );	// populate devices in canbus_t struct so that we know who else is on the bus

	return self->in;													// return it to someone who will make use of it
};

void message_handler_ctor(volatile message_handler_t *self, volatile can_t *in_can, volatile canbus_t *in_bus, volatile can_msg_t *in_msg, volatile can_msg_t *out_msg)
{
	self->can = in_can;													// set address of can object
	self->bus = in_bus;													// set address of bus object

	self->in = in_msg;													// set address of inbound message struct
	self->out = out_msg;												// set address of outbound message struct

	// TODO - eventually get rid of union
	self->bus->devices.uint16_val = 0x0000;
	self->bus->NumericalCAN_ID = 0;
	self->bus->FlagDoHeartbeat = 1;										// start with discovery mode
	self->bus->FlagDoDefaultOperation = 0;

	self->ReceiveMessage = &_ReceiveMessage;							// set up function pointer
	self->SendMessage = &_SendMessage;									//	ditto
};

volatile canbus_t BUS __attribute__ ((section (".data")));				// define BUS object and put it into .data
volatile message_handler_t MsgHandler __attribute__ ((section (".data")));		// define MsgHandler object and put it into .data

volatile can_msg_t msg_out;												// define msg_out
volatile can_msg_t msg_in;												// define msg_in