#include "main.h"
#include "mj818\mj818.h"
#include "mj818\mj818_led.c"											// concrete device-specific LED functions

typedef struct															// mj818_t actual
{
	mj818_t public;														// public struct
} __mj818_t;

static __mj818_t       __Device       __attribute__ ((section (".data")));	// preallocate __Device object in .data

// defines device operation on empty bus
void _EmptyBusOperationMJ818(void)
{
	// PRT - 	if(OCR_REAR_LIGHT == 0x00)											// run once
	__Device.public.led->Shine(0x10);								// operate on component part
}

// dispatches CAN messages to appropriate sub-component on device
void _PopulatedBusOperationMJ818(message_handler_t *const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();			// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if(msg->COMMAND== (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT))  // rear positional light
		{
			__Device.public.led->led[Rear].Shine(msg->ARGUMENT);	// fade rear light to CAN msg. argument value
			return;
		}

	if(msg->COMMAND== (CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT))		// brake light
		{
			if(msg->ARGUMENT > OCR_MAX_BRAKE_LIGHT)
			// PRT - 				OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
			;
			else
			// PRT - 				OCR_BRAKE_LIGHT = msg->ARGUMENT;
			;
		}
}

void mj818_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B));	// call base class constructor & initialize own SID

	// GPIO state definitions
		{
			// state initialization of device-specific pins
			// PRT - state initialization of device-specific pins
			// state initialization of device-specific pins
		}

	// hardware initialization
		{
			// PRT -			cli();

			// OCR init for rear lights - have lights off
			// PRT -OCR init for rear lights - have lights off

			// timer/counter1 - 16bit - brake light PWM
			// PRT - timer brake light PWM

			// timer/counter0 - 8bit - rear light PWM
			// PRT - timer rear light PWM

			// PRT -			sei();
		}

	__Device.public.led = _virtual_led_ctorMJ818();  // call virtual constructor & tie in object addresses

	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperationMJ818;	// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ818;	// implements device-specific operation depending on bus activity
}

#if defined(MJ818_)														// all devices have the object name "Device", hence the preprocessor macro
mj818_t *const Device = &__Device.public;	// set pointer to MsgHandler public part
#endif
