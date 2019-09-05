#include <inttypes.h>

#include "message.h"
#include "mj8x8.h"

typedef struct															// message_handler_t actual
{
	message_handler_t public;											// public struct
	can_msg_t __msg;													// private - CAN message object
	can_t *__can;														// private - pointer to can_t struct
} __message_handler_t;

static __message_handler_t __MsgHandler __attribute__ ((section (".data")));

// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	if (__MsgHandler.__can->in_sleep)									// if the CAN infra. is sleeping
		__MsgHandler.__can->Sleep(__MsgHandler.__can, 0);				// wake it up

	__MsgHandler.__msg.sidh = __MsgHandler.__can->own_sidh;
	__MsgHandler.__msg.sidl = __MsgHandler.__can->own_sidl;

	if (in_command == CMND_ANNOUNCE)									// if we have the broadcast command
		__MsgHandler.__msg.sidh |= BROADCAST;							//	then set the broadcast flag

	__MsgHandler.__msg.COMMAND = in_command;							// set command into message
	__MsgHandler.__msg.ARGUMENT = in_argument;							// set argument into message
	__MsgHandler.__msg.dlc = in_len;									// set DLC

	__MsgHandler.__can->RequestToSend(&__MsgHandler.__msg);				// load message into TX buffer and request to send
};

// fetches message received from CAN bus by local CAN IC, populates known hosts and returns message handler object
volatile can_msg_t *_ReceiveMessage(void)
{
	__MsgHandler.__can->FetchMessage(&__MsgHandler.__msg);				// fetch the message from some RX buffer into RAM
	// FIXME - 1st LU doesn't always get listed in devices.All -- very likely the root cause in the quick'n'dirty arduino LU
	if (__MsgHandler.__msg.sidh & BROADCAST)							// if we get a broadcast message (aka. heartbeat)
		__MsgHandler.public.bus->devices.All |= ( 1 << ( (__MsgHandler.__msg.sidh >> 2) & 0x0F ) );// populate devices in canbus_t struct so that we know who else is on the bus

	return &__MsgHandler.__msg;											// return pointer to it to someone who will make use of it
};

void message_handler_ctor(can_t * const in_can)
{
	static canbus_t BUS __attribute__ ((section (".data")));			// define BUS object and put it into .data

	__MsgHandler.__can = in_can;										// save address of can_t struct in private data member

	__MsgHandler.public.bus = &BUS;										// set address of bus object

	// TODO - eventually get rid of union
	__MsgHandler.public.bus->devices.All = 0x0000;
	__MsgHandler.public.bus->NumericalCAN_ID = (uint8_t) ( (__MsgHandler.__can->own_sidh >>2 ) & 0x0F );
	__MsgHandler.public.bus->FlagDoHeartbeat = 1;						// start with discovery mode
	__MsgHandler.public.bus->FlagDoDefaultOperation = 0;

	__MsgHandler.public.ReceiveMessage = &_ReceiveMessage;				// set up function pointer
	__MsgHandler.public.SendMessage = &_SendMessage;					//	ditto
};

message_handler_t * const MsgHandler = &__MsgHandler.public ;			// set pointer to MsgHandler public part