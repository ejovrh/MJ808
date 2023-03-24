#include "mj8x8\mj8x8.h"
#include "message\message.h"

static device_t _devices;  // devices discovered on the bus

typedef struct	// message_handler_t actual
{
	message_handler_t public;  // public struct
	can_msg_t *__rx_msg;  // private - reference to CAN message object
	can_msg_t *__tx_msg;  // private - reference to CAN message object
	can_t *__can;  // private - pointer to can_t struct
} __message_handler_t;

extern __message_handler_t __MsgHandler;  // declare message_handler_t actual
// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(const uint8_t in_command, const uint8_t in_argument, const uint8_t in_len)
{
	__MsgHandler.__tx_msg->sidh = __MsgHandler.__can->own_sidh;
	__MsgHandler.__tx_msg->sidl = __MsgHandler.__can->own_sidl;

	if(in_command == CMND_ANNOUNCE)  // if we have the broadcast command
		__MsgHandler.__tx_msg->sidh |= BROADCAST;  //	then set the broadcast flag

	__MsgHandler.__tx_msg->COMMAND= in_command;  // set command into message
	__MsgHandler.__tx_msg->ARGUMENT= in_argument;  // set argument into message
	__MsgHandler.__tx_msg->dlc = in_len;  // set DLC

	__MsgHandler.__can->RequestToSend(__MsgHandler.__tx_msg);  // load message into TX buffer and request to send
}

// upload CAN message into self at physical reception from FIFO
static void _SetMessage(const can_msg_t *const in_msg)
{
	uint8_t i;	// iterator for data deep copy

	__MsgHandler.__rx_msg->sidh = in_msg->sidh;  // deep copy
	__MsgHandler.__rx_msg->sidl = in_msg->sidl;
	__MsgHandler.__rx_msg->dlc = in_msg->dlc;
	__MsgHandler.__rx_msg->eid0 = in_msg->eid0;
	__MsgHandler.__rx_msg->eid8 = in_msg->eid8;
	__MsgHandler.__rx_msg->rx_status = in_msg->rx_status;

	for(i = 0; i < CAN_MAX_MSG_LEN; ++i)	// more deep copy
		__MsgHandler.__rx_msg->data[i] = in_msg->data[i];

	__MsgHandler.public.Devices->byte |= (1 << ((__MsgHandler.__rx_msg->sidh >> 2) & 0x0F));  // populate devices in canbus_t struct so that we know who else is on the bus
}

// fetches message received from CAN bus by local CAN IC, populates known hosts and returns message handler object
static inline can_msg_t* _GetMessage(void)
{
	return __MsgHandler.__rx_msg;  // return pointer to it to someone who will make use of it
}

can_msg_t _rx_can_message;  // object for storing a RX CAN message
can_msg_t _tx_can_message;  // object for storing a TX CAN message

__message_handler_t __MsgHandler =  // instantiate message_handler_t actual and set function pointers
	{  //
	.public.SendMessage = &_SendMessage,  // set up function pointer
	.public.GetMessage = &_GetMessage,  //	ditto
	.public.SetMessage = &_SetMessage,  //	ditto
	.public.Devices = &_devices  // tie in actual object to reference
	};

void message_handler_ctor(can_t *const in_can)
{
	__MsgHandler.__can = in_can;	// save address of can_t struct in private data member
	__MsgHandler.__rx_msg = &_rx_can_message;  // tie in message container
	__MsgHandler.__tx_msg = &_tx_can_message;  // ditto
	__MsgHandler.public.Devices->byte = 0x0000;  // default state is empty bus
}

message_handler_t *const MsgHandler = &__MsgHandler.public;  // set pointer to MsgHandler public part
