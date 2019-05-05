#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mcp2515.h"
#include "mj8x8.h"

#if defined(MJ808_) || defined(MJ818_) // private function - fades *ocr to value (or ocr_max) - up to OCR_MAX or down to 0x00
void fade(uint8_t value, volatile uint8_t *ocr, uint8_t ocr_max)
{
	if (value > *ocr) // we need to get brighter
	{
		while (*ocr < value) // loop until we match the OCR with the requested value
		{
			if (*ocr >= ocr_max) // safeguard against too high a CAN command argument (OCR_MAX is a function of schematic & PCB design)
			{
				#if defined(MJ808_)
				util_led(UTIL_LED_RED_BLINK_2X); // CHECKME: something drives this OCR dangerously up
				#endif
				break;
			}

			++*ocr; // increment the OCR
			_delay_ms(5); // delay it a bit for visual stimulus ...
		}
		return;
	}

	if (value < *ocr) // we need to get dimmer
	{
		while (*ocr > value) // loop until we match the OCR with the requested value
		{
			--*ocr; // decrement the OCR
			_delay_ms(2.5); // delay it a bit for visual stimulus ...
		}
		return;
	}
}

#endif

#if defined(MJ808_) // interprets CMND_UTIL_LED command - utility LED (red, green, on, off, blink)
void util_led(uint8_t in_val)
{
	uint8_t led = 0; // holds the pin of the LED: D0 - green (default), D1 - red

	if (in_val & _BV(B3)) // determine B3 value: red or green (default)
	led = 1; // red

	in_val &= 7; // clear everything except B2:0, which is the blink count (1-6)

	if (in_val == 0x00) // B3:B0 is 0 - turn off
	{
		PORTD |= (1<<led); // clear bit
		return;
	}

	if (in_val == 0x07) // B3:B0 is 7 - turn on
	{
		PORTD &= ~(1<<led); // set bit
		return;
	}

	while (in_val--) // blink loop
	{
		_delay_ms(BLINK_DELAY); // waste a few cycles (non-blocking)
		PORTD ^= (1<<led); // toggle the led pin
		_delay_ms(BLINK_DELAY); // waste a few cycles (non-blocking)
		PORTD ^= (1<<led); // toggle the led pin
	}
}
#endif

#if defined(SENSOR) // interprets CMND_DEVICE-DEV_SENSOR command - TODO - sensor related stuff
void dev_sensor(can_message_t *msg)
{
	util_led(UTIL_LED_GREEN_BLINK_4X); // debug indicator
	return;
}
#endif

#if defined(PWR_SRC) // interprets CMND_DEVICE-DEV_PWR_SRC command - TODO - power source related stuff
void dev_pwr_src(can_message_t *msg)
{
	util_led(UTIL_LED_GREEN_BLINK_2X); // debug indicator
	return;
}
#endif

#if defined(LOGIC_UNIT) // interprets CMND_DEVICE-DEV_LU command - TODO - logic unit related stuff
void dev_logic_unit(can_message_t *msg)
{
	util_led(UTIL_LED_GREEN_BLINK_1X); // debug indicator
	return;
}
#endif

#if defined(MJ808_) || defined (MJ818_) // interprets CMND_DEVICE-DEV_LIGHT command - positional light control
void dev_light(can_message_t *msg)
{
	#if defined (MJ808_)
	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT) ) // front positional light - low beam
	{
		fade(msg->ARGUMENT, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT); // fade front light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) ) // front positional light - high beam
	{
		if (msg->ARGUMENT > OCR_MAX_FRONT_LIGHT)
		{
			OCR_FRONT_LIGHT = OCR_MAX_FRONT_LIGHT;
			util_led(UTIL_LED_RED_BLINK_2X); // CHECKME: something drives this OCR dangerously up
		}
		else
			OCR_FRONT_LIGHT = msg->ARGUMENT;

		return;
	}
	#endif

	#if defined (MJ818_)
	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | REAR_LIGHT) ) // rear positional light
	{
		fade(msg->ARGUMENT, &OCR_REAR_LIGHT, OCR_MAX_REAR_LIGHT);  // fade rear light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT) ) // brake light
	{
		if (msg->ARGUMENT > OCR_MAX_BRAKE_LIGHT)
			OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
		else
			OCR_BRAKE_LIGHT = msg->ARGUMENT;
	}
	#endif
}
#endif

// conveys button press event to the CAN bus
void msg_button(can_message_t *msg, uint8_t button)
{
	msg->COMMAND = (MSG_BUTTON_EVENT | button);
	msg->dlc = 1;
	mcp2515_can_msg_send(msg);
}

void discovery_announce(volatile canbus *canbus_status, can_message_t *msg) // announce ourselves to the bus - once on power-up and then 2s-periodic
{
	/* time-slot based broadcast of self; timer is the watchdog, set to 250ms on inital init in main()
	 * after 2s this loop is done and everyone in the bus (if alive) has broadcasted their SID
	 *	in its time slot (provided power up was at the same time)
	 *	after that time, canbus_status->devices	holds a bitwise representation of what is alive on the bus
	 */

	if (canbus_status->broadcast_iteration_count == canbus_status->numerical_self_id) // compare device counter with decimal self-id
	{	// broadcast self-id to everyone
		msg->sidh |= BROADCAST; // set the broadcast flag
		msg->COMMAND = (CMND_ANNOUNCE); // "mark" it as an announce command (doesn't really do a thing since the announce is 0x00)
		msg->dlc = 1;
		mcp2515_can_msg_send(msg);
		msg->sidh &= ~BROADCAST; // unset the broadcast flag
		canbus_status->status &= ~0x80; // unset the "i-was-here" bit
		canbus_status->status |= 0x40; // FIXME - what is this flag used for??
		canbus_status->sleep_iteration = 0;	// reset the cycle and start over
		canbus_status->broadcast_iteration_count = 0;	// reset the cycle and start over

		WDTCR |= (_BV(WDCE) | _BV(WDE)); // WDT change enable sequence
		WDTCR = ( _BV(WDIE) | _BV(WDP2) | _BV(WDP1) | _BV(WDP0)); // set watchdog timer set to 2s

		// from here on we act upon the bus status directly in this iteration of the ISR
	}

	++canbus_status->broadcast_iteration_count; // counter, incremented by the WDT ISR; counts from the 0th up to the 15th device
}

// discover what lives on the CAN bus and act upon it
void discovery_behave(volatile canbus *canbus_status)
{
		// front light present
		#if defined(MJ818_) // rear light behavior
		if (canbus_status->devices._MJ808) // the front light is on the bus
		{
			return; // do nothing, the front light will tell me what to do
		}
		#endif

		// logic unit present
		if (canbus_status->devices._LU) // the logic unit is on the bus
		{
			return;
		}

		// rear light present
		if (canbus_status->devices._MJ818)
		{
			#if defined(MJ808_) // indicate through utility LED
			//util_led(UTIL_LED_RED_BLINK_2X);
			#endif
			return;
		}

		if (canbus_status->devices.all == 0x00) // we are alone on the bus -> go dumb, glow, rescan periodically and be happy...
		{
			#if defined(MJ808_) // setup of front light PWM - permanent on
			util_led(UTIL_LED_RED_BLINK_1X); // indicate bus empty
			#endif

			#if defined(MJ818_) // setup of rear light PWM - permanent on
			if (OCR_REAR_LIGHT == 0x00) // run once
			fade(0x10, &OCR_REAR_LIGHT, OCR_MAX_REAR_LIGHT);
			#endif

			//mcp2515_opcode_bit_modify(CANCTRL, 0x20, 0x20); // sleep - put into sleep mode
			//gpio_set(MCP2561_standby_pin); // sleep of mcp2561

			if (canbus_status->sleep_iteration > REDISCOVER_ITERATION) // initiate re-broadcast
			{
				WDTCR |= (_BV(WDCE) | _BV(WDE)); // WDT change enable sequence
				WDTCR = ( _BV(WDIE) | _BV(WDP2)  ); // watchdog timer set to 0.25

				canbus_status->broadcast_iteration_count = 0; // reset the device counter
				canbus_status->status |= 0x80; // unset i-was-here bit and let it rescan again
			}

			return;
		}
}

#if defined(MJ808_) || defined (MJ828_)
void button_debounce(volatile button_t *in_button)	// marks a button as pressed if it was pressed for the duration of 2X ISR iterations
	{
	inline void local_advance_counter(void)	// local helper function which advances the debounce "timer"
	{
		++in_button->hold_counter;	// start to count (used to determine long button press; not used for debouncing)

		// debouncing by means of bit shifting
		in_button->state ^ 0x03;	// XOR what we currently have in state
		in_button->state <<= 1;		// left shift so that if we have 0x01 this becomes 0x02
		in_button->state |= 0x01;	// OR what we have with 0x01 so that 0x02 becomes 0x03
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
	#if defined(MJ828_) // inverted
	if (	!	( *(in_button->PIN) & (1<<in_button->pin_number))		)	// if in the given PIN register the given button is pressed
	#endif
	#if defined(MJ808_)	// non-inverted
	if (		( *(in_button->PIN) & (1<<in_button->pin_number))		)	// if in the given PIN register the given button is pressed
	#endif
	{									// button is pressed
		local_advance_counter();		// debouncing happens here

		if (in_button->hold_counter >= BUTTON_MAX_PRESS_TIME)	// too long button press -> error state
		{										// turn everything off
			in_button->state = 0;				// reset state
			in_button->is_pressed = 0;			// mark as currently not pressed
			in_button->was_pressed = 1;			// mark button as "was pressed" - this is the previous state in the next iteration
			in_button->hold_error = 1;			// mark as error state

			in_button->toggle = 0;				// toggled due to error state -> reset to default value
			in_button->hold_temp = 0;			// mark as hold_temp off
			return;								// get out
		}
	}
	else
	{
		if (in_button->is_at_default)	// if we are in zero state
			return;							// no need to do anything

									// button is released
										// button is not pressed - reset everything to default values
		in_button->state = 0;			// reset state
		in_button->is_pressed = 0;		// mark as currently not pressed

		in_button->was_pressed = 1;		// mark button as "was pressed" - this is the previous state in the next iteration
		in_button->hold_error = 0;		// release error flag, since button is now released
		in_button->hold_counter = 0;	// set counter back to 0
		in_button->is_pressed = 0;		// mark as currently not pressed
		in_button->is_at_default = 1;
		return;							// finish
	}

	// state markers
	if (in_button->state == 0x03 && !in_button->hold_error)		// if we have a non-error steady state
	{
		if (in_button->hold_counter >= BUTTON_MIN_PRESS_TIME)		// pressed for a valid amount of time
		{
			if (!in_button->was_pressed)								// previous state (prevent flapping on/off)
				in_button->hold_temp = !in_button->hold_temp;				// set "hold_temp" state

			in_button->was_pressed = 1;									// mark the button as being pressed
			return;
		}

		if (in_button->was_pressed)								// previous state (prevent flapping on/off)
			in_button->toggle = !in_button->toggle;					// set "toggle" state

		in_button->is_pressed = 1;								// set "is_pressed" state
		in_button->was_pressed = 0;								// mark the button as being pressed
		in_button->is_at_default = 0;
	}
}
#endif

#if defined(MJ828_)
static void glow(uint8_t led, uint8_t state, uint8_t blink) // private function, used only by the charlieplexing_handler() function
{
	// FIXME - blinking is passed by value, hence never decremented where it should be: in the struct
	//	task: put code in glow() into the charlieplex-handler

	// set LED pins to initial state
	gpio_conf(LED_CP1_pin, INPUT, LOW);				// Charlie-plexed pin1
	gpio_conf(LED_CP2_pin, INPUT, LOW);				// Charlie-plexed pin2
	gpio_conf(LED_CP3_pin, INPUT, LOW);				// Charlie-plexed pin3
	gpio_conf(LED_CP4_pin, INPUT, LOW);				// Charlie-plexed pin4

	static uint8_t counter;
	counter++;

	if (! (state || blink) )	// if we get 0x00 (off argument) - do nothing and get out
		return;



	switch (led)
	{
		case 0x00:	//red led
			gpio_conf(LED_CP2_pin, OUTPUT, HIGH); // b1 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP1_pin, OUTPUT, LOW); // b2 - cathode
			else
				gpio_conf(LED_CP1_pin, OUTPUT, HIGH); // b2 - cathode
		break;

		case 0x01:	// green led
			gpio_conf(LED_CP1_pin, OUTPUT, HIGH); // b2 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP2_pin, OUTPUT, LOW); // b1 - cathode
			else
				gpio_conf(LED_CP2_pin, OUTPUT, HIGH); // b1 - cathode
		break;

		case 0x02:	// blue1 led
			gpio_conf(LED_CP2_pin, OUTPUT, HIGH); // b1 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP3_pin, OUTPUT, LOW); // b0 - cathode
			else
				gpio_conf(LED_CP3_pin, OUTPUT, HIGH); // b0 - cathode
		break;

		case 0x03:	// yellow led
			gpio_conf(LED_CP3_pin, OUTPUT, HIGH); // b2 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP2_pin, OUTPUT, LOW); // b1 - cathode
			else
				gpio_conf(LED_CP2_pin, OUTPUT, HIGH); // b1 - cathode
		break;

		case 0x04:	// blue2 LED (battery indicator 1)
			gpio_conf(LED_CP4_pin, OUTPUT, HIGH); // d6 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP3_pin, OUTPUT, LOW); // b0 - cathode
			else
				gpio_conf(LED_CP3_pin, OUTPUT, HIGH); // b0 - cathode
		break;

		case 0x05:	// blue3 LED (battery indicator 2)
			gpio_conf(LED_CP3_pin, OUTPUT, HIGH); // b0 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP4_pin, OUTPUT, LOW); // d6 - cathode
			else
				gpio_conf(LED_CP4_pin, OUTPUT, HIGH); // d6 - cathode
		break;

		case 0x06:	// blue4 LED (battery indicator 3)
			gpio_conf(LED_CP1_pin, OUTPUT, HIGH); // b2 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP4_pin, OUTPUT, LOW); // d6 - cathode
			else
				gpio_conf(LED_CP4_pin, OUTPUT, HIGH); // d6 - cathode
		break;

		case 0x07:	// blue5 LED (battery indicator 4)
			gpio_conf(LED_CP4_pin, OUTPUT, HIGH); // d6 - anode

			if ( (state) || (blink && counter <= 128) )  // on
				gpio_conf(LED_CP1_pin, OUTPUT, LOW); // b2 - cathode
			else
				gpio_conf(LED_CP1_pin, OUTPUT, HIGH); // b2 - cathode
		break;
	}

};

void charlieplexing_handler(volatile leds_t *in_led)
{
	static uint8_t i = 0;	// iterator to loop over all LEDs on device

	glow(i, in_led->leds[i].on, in_led->leds[i].blink_count); // e.g. command = 0x00 (red), arg = 0x01 (on)

	// !!!!
	(i == in_led->led_count) ? i = 0 : ++i; // count up to led_count and then start from zero
}
#endif