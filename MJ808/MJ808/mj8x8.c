#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "mj8x8.h"

// provides a periodic heartbeat based on the watchdog timer interrupt
void Heartbeat(volatile void *msg)
{
	message_handler_t *ptr = (message_handler_t *) msg;

	if (ptr->bus->FlagDoHeartbeat)										// if we are in heartbeat mode
	{
		if (ptr->bus->BeatIterationCount == ptr->bus->NumericalCAN_ID)	// see if this counter iteration is our turn
		{
			ptr->SendMessage(ptr, CMND_ANNOUNCE, 0x00, 1);				// broadcast CAN heartbeat message

			ptr->bus->FlagDoHeartbeat = 0;								// heartbeat mode of for the remaining counter iterations
			WDTCR |= (_BV(WDCE) | _BV(WDE));							// WDT change enable sequence
			WDTCR = ( _BV(WDIE) | _BV(WDP2) | _BV(WDP1) );				// set watchdog timer set to 1s

			#if defined(MJ808_)
			// TODO - access via object
			_util_led_mj808(UTIL_LED_RED_BLINK_1X);
			#endif
		}
	}

	if ((!ptr->bus->BeatIterationCount) && (!ptr->bus->FlagDoHeartbeat))// counter roll-over, change from slow to fast
	{
		ptr->bus->FlagDoHeartbeat = 1;									// set heartbeat mode on
		++ptr->bus->FlagDoDefaultOperation;								// essentially count how many times we are in non-heartbeat count mode

		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = ( _BV(WDIE) | _BV(WDP2)  );								// watchdog timer set to 0.25
	}

	++ptr->bus->BeatIterationCount;										// increment the iteration counter
};

volatile mj8x8_t * mj8x8_ctor(volatile mj8x8_t *self, volatile can_t *can, volatile ATtiny4313_t *mcu)
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

	self->HeartBeat = &Heartbeat;
	self->mcu = attiny_ctor(mcu);										// pass MCU address into constructor
	self->can = can_ctor(can);											// pass CAN address into constructor

	return self;
};

volatile mj8x8_t MJ8x8 __attribute__ ((section (".data")));	// define MJ8X8 object and put it into .data