#include <inttypes.h>

#include "message.h"
#include "mj8x8.h"

static can_msg_t __msg __attribute__ ((section (".data")));				// define private CAN message object and put it into .data

// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(volatile message_handler_t * const self, const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	if (self->can->in_sleep)											// if the CAN infra. is sleeping
		self->can->Sleep(self->can, 0);									// wake it up

	__msg.sidh = self->can->own_sidh;
	__msg.sidl = self->can->own_sidl;

	if (in_command == CMND_ANNOUNCE)									// if we have the broadcast command
		__msg.sidh |= BROADCAST;										//	then set the broadcast flag

	__msg.COMMAND = in_command;											// set command into message
	__msg.ARGUMENT = in_argument;										// set argument into message
	__msg.dlc = in_len;													// set DLC

	self->can->RequestToSend(&__msg);									// load message into TX buffer and request to send
};

// fetches message received from CAN bus by local CAN IC, populates known hosts and returns message handler object
volatile can_msg_t *_ReceiveMessage(volatile struct message_handler_t * const self)
{
	self->can->FetchMessage(&__msg);									// fetch the message from some RX buffer into RAM
	// FIXME - 1st LU doesn't always get listed in devices.All -- very likely the root cause in the quick'n'dirty arduino LU
	if (__msg.sidh & BROADCAST)											// if we get a broadcast message (aka. heartbeat)
		self->bus->devices.All |= ( 1 << ( (__msg.sidh >> 2) & 0x0F ) );// populate devices in canbus_t struct so that we know who else is on the bus

	return &__msg;														// return pointer to it to someone who will make use of it
};

void message_handler_ctor(volatile message_handler_t *self, volatile can_t *in_can)
{
	static canbus_t BUS __attribute__ ((section (".data")));				// define BUS object and put it into .data

	self->can = in_can;													// set address of can object
	self->bus = &BUS;													// set address of bus object

	// TODO - eventually get rid of union
	self->bus->devices.All = 0x0000;
	self->bus->NumericalCAN_ID = (uint8_t) ( (self->can->own_sidh >>2 ) & 0x0F );
	self->bus->FlagDoHeartbeat = 1;										// start with discovery mode
	self->bus->FlagDoDefaultOperation = 0;

	self->ReceiveMessage = &_ReceiveMessage;							// set up function pointer
	self->SendMessage = &_SendMessage;									//	ditto
};

volatile message_handler_t MsgHandler __attribute__ ((section (".data")));		// define MsgHandler object and put it into .data