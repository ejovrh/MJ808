#include <inttypes.h>

#include "message.h"
#include "mj8x8.h"

static can_msg_t __msg __attribute__ ((section (".data")));				// define private CAN message object and put it into .data
static volatile message_handler_t *__self;								// private pointer to self
static volatile can_t *__can;											// private pointer to can_t struct

// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	if (__can->in_sleep)												// if the CAN infra. is sleeping
		__can->Sleep(__can, 0);											// wake it up

	__msg.sidh = __can->own_sidh;
	__msg.sidl = __can->own_sidl;

	if (in_command == CMND_ANNOUNCE)									// if we have the broadcast command
		__msg.sidh |= BROADCAST;										//	then set the broadcast flag

	__msg.COMMAND = in_command;											// set command into message
	__msg.ARGUMENT = in_argument;										// set argument into message
	__msg.dlc = in_len;													// set DLC

	__can->RequestToSend(&__msg);										// load message into TX buffer and request to send
};

// fetches message received from CAN bus by local CAN IC, populates known hosts and returns message handler object
volatile can_msg_t *_ReceiveMessage(void)
{
	__can->FetchMessage(&__msg);										// fetch the message from some RX buffer into RAM
	// FIXME - 1st LU doesn't always get listed in devices.All -- very likely the root cause in the quick'n'dirty arduino LU
	if (__msg.sidh & BROADCAST)											// if we get a broadcast message (aka. heartbeat)
		__self->bus->devices.All |= ( 1 << ( (__msg.sidh >> 2) & 0x0F ) );// populate devices in canbus_t struct so that we know who else is on the bus

	return &__msg;														// return pointer to it to someone who will make use of it
};

void message_handler_ctor(volatile message_handler_t *self, volatile can_t *in_can)
{
	static canbus_t BUS __attribute__ ((section (".data")));			// define BUS object and put it into .data

	__self = self;														// save address of self in private data member
	__can = in_can;														// save address of can_t struct in private data member

	__self->bus = &BUS;													// set address of bus object

	// TODO - eventually get rid of union
	__self->bus->devices.All = 0x0000;
	__self->bus->NumericalCAN_ID = (uint8_t) ( (__can->own_sidh >>2 ) & 0x0F );
	__self->bus->FlagDoHeartbeat = 1;									// start with discovery mode
	__self->bus->FlagDoDefaultOperation = 0;

	__self->ReceiveMessage = &_ReceiveMessage;							// set up function pointer
	__self->SendMessage = &_SendMessage;								//	ditto
};

volatile message_handler_t MsgHandler __attribute__ ((section (".data")));		// define MsgHandler object and put it into .data