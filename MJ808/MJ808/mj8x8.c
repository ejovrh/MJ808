#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "mj8x8.h"

extern void DoNothing(void);

typedef struct															// mj8x8_t actual
{
	mj8x8_t public;														// public struct
//	uint8_t foo_private;												// private - some data member
} __mj8x8_t;

extern __mj8x8_t __MJ8x8;												// declare mj8x8_t actual

// provides a periodic heartbeat based on the watchdog timer interrupt
static void _Heartbeat(message_handler_t * const msg)
{
	if (msg->bus->FlagDoHeartbeat)										// if we are in heartbeat mode
	{
		if (msg->bus->BeatIterationCount == msg->bus->NumericalCAN_ID)	// see if this counter iteration is our turn
		{
			msg->SendMessage(CMND_ANNOUNCE, 0x00, 1);					// broadcast CAN heartbeat message

			msg->bus->FlagDoHeartbeat = 0;								// heartbeat mode of for the remaining counter iterations
			WDTCR |= (_BV(WDCE) | _BV(WDE));							// WDT change enable sequence
			WDTCR = ( _BV(WDIE) | _BV(WDP2) | _BV(WDP1) );				// set watchdog timer set to 1s

			#if defined(MJ808_)
			// TODO - access via object
			_util_led_mj808(UTIL_LED_RED_BLINK_1X);
			#endif
		}
	}

	if ((!msg->bus->BeatIterationCount) && (!msg->bus->FlagDoHeartbeat))// counter roll-over, change from slow to fast
	{
		msg->bus->FlagDoHeartbeat = 1;									// set heartbeat mode on
		++msg->bus->FlagDoDefaultOperation;								// essentially count how many times we are in non-heartbeat count mode

		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = ( _BV(WDIE) | _BV(WDP2)  );								// watchdog timer set to 0.25
	}
	++msg->bus->BeatIterationCount;										// increment the iteration counter
};

__mj8x8_t __MJ8x8 =														// instantiate mj8x8_t actual and set function pointers
{
	.public.HeartBeat = &_Heartbeat,									// implement device-agnostic default behavior - heartbeat
	.public.EmptyBusOperation = &DoNothing								// implement device-agnostic default behavior - do nothing, usually an override happens
};

mj8x8_t * mj8x8_ctor()
{
	// GPIO state definitions
	{
	// state initialization of device-unspecific pins
	gpio_conf(MCP2515_INT_pin, INPUT, HIGH);							// INT1, active low

	gpio_conf(ICSP_DO_MOSI, OUTPUT, LOW);								// data out - output pin
	gpio_conf(ICSP_DI_MISO, INPUT, LOW);								// data in - input pin
	gpio_conf(SPI_SCK_pin, OUTPUT, LOW);								// low for proper CPOL = 0 waveform
	gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);						// high (device inert), low (device selected)
	// state initialization of device-unspecific pins
	}

	__MJ8x8.public.can = can_ctor();									// pass on CAN public part
	__MJ8x8.public.mcu = attiny_ctor();									// pass on MCU public part

	return &__MJ8x8.public;
};