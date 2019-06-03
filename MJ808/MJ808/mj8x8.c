#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "gpio.h"
#include "mj8x8.h"

#if defined(MJ808_) || defined (MJ828_)									// button debouncer for devices with buttons
void button_debounce(volatile button_t *in_button)						// marks a button as pressed if it was pressed for the duration of 2X ISR iterations
	{
	inline void local_advance_counter(void)								// local helper function which advances the debounce "timer"
	{
		++in_button->hold_counter;										// start to count (used to determine long button press; not used for debouncing)

																		// debouncing by means of bit shifting
																		// CHECKME - XOR has no assignment operator
		in_button->state ^ 0x03;										// XOR what we currently have in state
		in_button->state <<= 1;											// left shift so that if we have 0x01 this becomes 0x02
		in_button->state |= 0x01;										// OR what we have with 0x01 so that 0x02 becomes 0x03
	}

	/*	rationale of debouncing:
	 *		- the ISR fires every 25ms, which means the sample rate of a button press is once per 25ms
	 *		- if the button is held for only one iteration (bounce state) we have 0x01 and on the next iteration state is reset to 0x00 [1st 25ms]
	 *		- if the button is held for two iterations, state is on the first iteration 0x01 and 0x03 on the second [2nd 25ms]
	 *		- after 2 iterations 50ms have passed -> stable state
	 *
	 *		the debouncer does only that - it debounces
	 *
	 *		the state marker marks the button state for external code to make sense of it
	 *		valid states:
	 *			- "is_pressed" - key is pressed (and held)
	 *			- "toggle" - toggled on/off state, once per key press
	 *			- "hold_temp" - held for e.g 1s to turn something on/off
	 *			- "hold_constant" - held constantly (e.g. by error) - to be ignored
	 */
																		// CHECKME - random spikes (not true button press events) every 25ms might be an issue
	#if defined(MJ828_)													// inverted
	if (	!	( (volatile) *(in_button->PIN) & (1<<in_button->pin_number))		)	// if in the given PIN register the given button is pressed
	#endif
	#if defined(MJ808_)													// non-inverted
	if (		( (volatile) *(in_button->PIN) & (1<<in_button->pin_number))		)	// if in the given PIN register the given button is pressed
	#endif
	{												// button is pressed
		local_advance_counter();										// debouncing happens here

		if (in_button->hold_counter >= BUTTON_MAX_PRESS_TIME)			// too long button press -> error state
		{																// turn everything off
			in_button->state = 0;										// reset state
			in_button->is_pressed = 0;									// mark as currently not pressed
			in_button->was_pressed = 1;									// mark button as "was pressed" - this is the previous state in the next iteration
			in_button->hold_error = 1;									// mark as error state

			in_button->toggle = 0;										// toggled due to error state -> reset to default value
			in_button->hold_temp = 0;									// mark as hold_temp off
			return;														// get out
		}
	}
	else											// button is released
	{
		if (in_button->is_at_default)									// if we are in zero state
			return;														// no need to do anything

																		// button is released
																		// button is not pressed - reset everything to default values
		in_button->state = 0;											// reset state
		in_button->is_pressed = 0;										// mark as currently not pressed

		in_button->was_pressed = 1;										// mark button as "was pressed" - this is the previous state in the next iteration
		in_button->hold_error = 0;										// release error flag, since button is now released
		in_button->hold_counter = 0;									// set counter back to 0
		in_button->is_pressed = 0;										// mark as currently not pressed
		in_button->is_at_default = 1;
		return;															// finish
	}

	// state markers
	if (in_button->state == 0x03 && !in_button->hold_error)				// if we have a non-error steady state
	{
		if (in_button->hold_counter >= BUTTON_MIN_PRESS_TIME)			// pressed for a valid amount of time
		{
			if (!in_button->was_pressed)								// previous state (prevent flapping on/off)
				in_button->hold_temp = !in_button->hold_temp;				// set "hold_temp" state

			in_button->was_pressed = 1;									// mark the button as being pressed
			return;
		}

		if (in_button->was_pressed)										// previous state (prevent flapping on/off)
			in_button->toggle = !in_button->toggle;						// set "toggle" state

		in_button->is_pressed = 1;										// set "is_pressed" state
		in_button->was_pressed = 0;										// mark the button as being pressed
		in_button->is_at_default = 0;
	}
};
#endif

// provides a periodic heartbeat based on the watchdog timer interrupt
void Heartbeat(volatile message_handler_t *msg)
{
	if (msg->bus->FlagDoHeartbeat)										// if we are in heartbeat mode
	{
		if (msg->bus->BeatIterationCount == msg->bus->NumericalCAN_ID)	// see if this counter iteration is our turn
		{
			msg->SendMessage(msg, CMND_ANNOUNCE, 0x00, 1);				// broadcast CAN heartbeat message

			msg->bus->FlagDoHeartbeat = 0;								// heartbeat mode of for the remaining counter iterations
			WDTCR |= (_BV(WDCE) | _BV(WDE));							// WDT change enable sequence
			WDTCR = ( _BV(WDIE) | _BV(WDP2) | _BV(WDP1) );				// set watchdog timer set to 1s

			#if defined(MJ808_)
			util_led(UTIL_LED_RED_BLINK_1X);
			#endif
		}
	}

	if ( (! msg->bus->BeatIterationCount) && (! msg->bus->FlagDoHeartbeat) )		// counter roll-over, change from slow to fast
	{
		msg->bus->FlagDoHeartbeat = 1;									// set heartbeat mode on
		++msg->bus->FlagDoDefaultOperation;								// essentially count how many times we are in non-heartbeat count mode

		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = ( _BV(WDIE) | _BV(WDP2)  );								// watchdog timer set to 0.25
	}

	++msg->bus->BeatIterationCount;										// increment the iteration counter
};


volatile mj8x8_t * mj8x8_ctor(volatile mj8x8_t *self, volatile can_t *can, volatile attiny4313_t *mcu)
{
	// GPIO state definitions
	{
	// state initialization of device-unspecific pins
	gpio_conf(MCP2515_INT_pin, INPUT, HIGH);							// INT1, active low

	gpio_conf(ICSP_DO_MOSI, OUTPUT, LOW);								// data out - output pin
	gpio_conf(ICSP_DI_MISO, INPUT, LOW);								// data in - input pin
	gpio_conf(SPI_SCK_pin, OUTPUT, LOW);								// low for proper CPOL = 0 waveform
	gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);						// //high (device inert), low (device selected)
	// state initialization of device-unspecific pins
	}

	self->HeartBeat = &Heartbeat;
	self->mcu = attiny_ctor(mcu);										// pass MCU address into constructor
	self->can = can_ctor(can);											// pass CAN address into constructor

	return self;
};