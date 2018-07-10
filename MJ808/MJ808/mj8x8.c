#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mcp2515.h"
#include "mj8x8.h"

#if defined(MJ808_) || defined(MJ818_) // private function - fades *ocr to value (or ocr_max) - up to OCR_MAX or down to 0x00

#if defined(MJ808_)
void fade(uint8_t value, volatile uint16_t *ocr, uint8_t ocr_max)
#endif

#if defined(MJ818_)
void fade(uint8_t value, volatile uint8_t *ocr, uint8_t ocr_max)
#endif

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
void util_led(uint8_t in_val) //TODO - convert to pointer-type argument
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
		_delay_ms(BLINK_DELAY); //TODO: replace with timer
		PORTD ^= (1<<led); // toggle the led pin
		_delay_ms(BLINK_DELAY); //TODO: replace with timer
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
		{
			OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
			#if defined (MJ808_)
			util_led(UTIL_LED_GREEN_BLINK_2X); // CHECKME: something drives this OCR dangerously up
			#endif
		}
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

void discovery_announce(volatile canbus *canbus_status, can_message_t *msg)
{
	/* time-slot based broadcast of self; timer is the watchdog, set to 500ms on inital init in main()
	 * after 2s this loop is done and everyone in the bus (if alive) has broadcasted their SID
	 *	in its time slot (provided power up was at the same time)
	 *	after that time, canbus_status->devices	holds a bitwise representation of what is alive on the bus
	 */

	if (canbus_status->count == canbus_status->n) // compare counter with decimal self-id, if we match
	{	// broadcast self-id to everyone
		msg->sidh |= BROADCAST; // set the broadcast flag
		msg->COMMAND = (CMND_ANNOUNCE); // "mark" it as an announce command (doesn't really do a thing since the announce is 0x00)
		msg->dlc = 1;
		mcp2515_can_msg_send(msg);
		msg->sidh &= ~BROADCAST; // unset the broadcast flag
		canbus_status->status &= ~0x80; // unset the "i-was-here" bit
		canbus_status->status |= 0x40;
		canbus_status->sleep_iteration = 0;

		WDTCR |= (_BV(WDCE) | _BV(WDE)); // WDT change enable sequence
		WDTCR |= ( _BV(WDIE) | _BV(WDP3)); // set watchdog timer set to 8s

		// from here on we act upon the bus status directly in this iteration of the ISR
	}

	++canbus_status->count; // counter, incremented by the WDT ISR; counts from the 0th up to the 15th device
}

// discover what lives on the CAN bus and act upon it
void discovery_behave(volatile canbus *canbus_status)
{
		// front light present
		#if defined(MJ818_) // rear light behavior
		if (canbus_status->devices & _BV(MJ808)) // the front light is on the bus
		{
			return; // do nothing, the front light will tell me what to do
		}
		#endif

		// logic unit present
		if (canbus_status->devices & _BV(LU)) // the logic unit is on the bus
		{
			#if defined(MJ808_) // indicate through utility LED
			util_led(UTIL_LED_GREEN_BLINK_1X);
			#endif
			return; // do nothing, the logic unit will tell me what to do
		}

		// rear light present
		if (canbus_status->devices & _BV(MJ818))
		{
			#if defined(MJ808_) // indicate through utility LED
			util_led(UTIL_LED_RED_BLINK_2X);
			#endif
			return;
		}

		if (canbus_status->devices == 0x00) // we are alone on the bus -> go dumb, glow, rescan periodically and be happy...
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

			//if (canbus_status->sleep_iteration > 3)
			//{
			//WDTCR |= (_BV(WDCE) | _BV(WDE)); // WDT change enable sequence
			//WDTCR |= ( _BV(WDIE) | _BV(WDP2) | _BV(WDP0)); // watchdog timer set to 0.5
			//canbus_status->count = 0;
			//canbus_status->status &= ~0x80; // unset i-was-here bit and let it rescan again
			//}

			return;
		}
}