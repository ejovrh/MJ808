#include <inttypes.h>

#include "message\message.h"
#include "mj8x8\mj8x8.h"

typedef struct															// message_handler_t actual
{
	message_handler_t public;											// public struct
	// TODO - convert to pointer
	can_msg_t __msg;													// private - CAN message object
	can_t *__can;														// private - pointer to can_t struct
} __message_handler_t;

extern __message_handler_t __MsgHandler;								// declare message_handler_t actual

// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	__MsgHandler.__msg.sidh = __MsgHandler.__can->own_sidh;
	__MsgHandler.__msg.sidl = __MsgHandler.__can->own_sidl;

	if (in_command == CMND_ANNOUNCE)									// if we have the broadcast command
		__MsgHandler.__msg.sidh |= BROADCAST;							//	then set the broadcast flag

	__MsgHandler.__msg.COMMAND = in_command;							// set command into message
	__MsgHandler.__msg.ARGUMENT = in_argument;							// set argument into message
	__MsgHandler.__msg.dlc = in_len;									// set DLC

	// TODO - move sleep wakeup into _mcp2515_can_msg_send()
	__MsgHandler.__can->Sleep(0);										// attempt to wake it up
	__MsgHandler.__can->RequestToSend(&__MsgHandler.__msg);				// load message into TX buffer and request to send
};

// fetches message received from CAN bus by local CAN IC, populates known hosts and returns message handler object
volatile can_msg_t *_ReceiveMessage(void)
{
	__MsgHandler.__can->FetchMessage(&__MsgHandler.__msg);				// fetch the message from some RX buffer into RAM
	// FIXME - 1st LU doesn't always get listed in devices.All -- very likely the root cause in the quick'n'dirty arduino LU
	__MsgHandler.public.Devices |= ( 1 << ( (__MsgHandler.__msg.sidh >> 2) & 0x0F ) );// populate devices in canbus_t struct so that we know who else is on the bus

	return &__MsgHandler.__msg;											// return pointer to it to someone who will make use of it
};

__message_handler_t __MsgHandler =										// instantiate message_handler_t actual and set function pointers
{
	.public.SendMessage = &_SendMessage,								// set up function pointer
	.public.ReceiveMessage = &_ReceiveMessage							//	ditto
};

void message_handler_ctor(can_t * const in_can)
{
	__MsgHandler.__can = in_can;										// save address of can_t struct in private data member
	__MsgHandler.public.Devices = 0x0000;								// default state is empty bus
};

message_handler_t * const MsgHandler = &__MsgHandler.public ;			// set pointer to MsgHandler public part