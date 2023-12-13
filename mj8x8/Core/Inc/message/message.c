#include "mj8x8\mj8x8.h"
#include "message\message.h"
#include "try/try.h"

extern activity_t *_BusActivityArray[16];
static device_t _devices;  // devices discovered on the bus

typedef struct	// message_handler_t actual
{
	message_handler_t public;  // public struct
	can_msg_t *__rx_msg;  // private - reference to CAN message object
	can_msg_t *__tx_msg;  // private - reference to CAN message object
	can_t *__can;  // private - pointer to can_t struct
} __message_handler_t;

extern __message_handler_t __MsgHandler;  // declare message_handler_t actual
can_msg_t _rx_can_message;  // object for storing a RX CAN message
can_msg_t _tx_can_message;  // object for storing a TX CAN message

// loads outbound CAN message into local CAN IC and asks it to transmit it onto the bus
void _SendMessage(const mj8x8_Devices_t in_rcpt, const uint8_t in_command, const uint8_t *in_payload, const uint8_t in_len)
{
	// TODO - SendMessage() - devise a way to send arbitrary message lengths >> 8 Bytes, i.e. 20k for FW updates over CAN

	// first, format the sender into the empty TX CAN identifier
	__MsgHandler.__tx_msg->sid = __MsgHandler.__can->own_sid;
	__MsgHandler.__tx_msg->sid |= PRIORITY_LOW;  // set to low priority (default)

	// second, format-in the recipient, if exists...
	if(in_rcpt == 0)  // if no recipient ...
		__MsgHandler.__tx_msg->sid |= BROADCAST;  //	... then set the broadcast flag (the default is unicast)
	else
		// there is a recipient...: unicast  - the default
		__MsgHandler.__tx_msg->sid |= in_rcpt;  // copy the sender ID bits [3:0] into the SID

	__MsgHandler.__tx_msg->data[0] = in_command;

	for(uint8_t i = 0; i < in_len - 1; ++i)
		__MsgHandler.__tx_msg->data[i + 1] = *(in_payload + i);  // deep copy

	__MsgHandler.__tx_msg->dlc = in_len;  // set DLC (data plus command byte)

	__MsgHandler.__can->RequestToSend(__MsgHandler.__tx_msg);  // load message into TX buffer and request to send
}

// convert the sender CAN SID to a zero-indexed device number
static inline uint8_t SenderIDtoNum(const uint16_t in_sid)
{
	return ((in_sid & 0xF0) >> 4);  // mask relevant bits and rsh and return the result
}

// RX - upload CAN message into self at physical reception from FIFO
static void _SetMessage(const can_msg_t *const in_msg)
{
	uint8_t i;	// iterator for data deep copy
	uint8_t num;	// numerical value derived from CAN SID of sender

	__MsgHandler.__rx_msg->sid = in_msg->sid;  // deep copy
	__MsgHandler.__rx_msg->dlc = in_msg->dlc;

	num = SenderIDtoNum(__MsgHandler.__rx_msg->sid);  // convert sender CAN SID to zero-indexed device number

	for(i = 0; i < __MsgHandler.__rx_msg->dlc; ++i)  // more deep copy
		__MsgHandler.__rx_msg->data[i] = in_msg->data[i];

	__MsgHandler.public.Devices->byte |= _BV(num);  // populate devices in canbus_t struct so that we know who else is on the bus

	if(__MsgHandler.__rx_msg->COMMAND== CMND_ANNOUNCE)  // if it is a heartbeat message
		{
			if(num != ALL)
				_BusActivityArray[num]->byte = __MsgHandler.__rx_msg->ARGUMENT;  // store the transmitted remote device status
			else
				return;
		}
}

// RX - returns reference to received message stored in message handler object
static inline can_msg_t* _GetMessage(void)
{
	return __MsgHandler.__rx_msg;  // return pointer to it to someone who will make use of it
}

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
