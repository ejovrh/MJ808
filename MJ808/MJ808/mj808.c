#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mcp2515.h"
#include "mj808.h"

// private function - fades *ocr to value (or ocr_max) - up to OCR_MAX or down to 0x00
void fade(uint8_t value, volatile uint16_t *ocr, uint8_t ocr_max)
{
	if (value > *ocr) // we need to get brighter
	{
		while (*ocr < value) // loop until we match the OCR with the requested value
		{
			if (*ocr >= ocr_max) // safeguard against too high a CAN command argument (OCR_MAX is a function of schematic & PCB design)
			{
				util_led(UTIL_LED_RED_BLINK_2X); // CHECKME: something drives this OCR dangerously up
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


// interprets CMND_UTIL_LED command - utility LED (red, green, on, off, blink)
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

// interprets CMND_DEVICE-DEV_SENSOR command - TODO - sensor related stuff
void dev_sensor(can_message_t *msg)
{
	util_led(UTIL_LED_GREEN_BLINK_4X); // debug indicator
	return;
}

// interprets CMND_DEVICE-DEV_PWR_SRC command - TODO - power source related stuff
void dev_pwr_src(can_message_t *msg)
{
	util_led(UTIL_LED_GREEN_BLINK_2X); // debug indicator
	return;
}

// interprets CMND_DEVICE-DEV_LU command - TODO - logic unit related stuff
void dev_logic_unit(can_message_t *msg)
{
	util_led(UTIL_LED_GREEN_BLINK_1X); // debug indicator
	return;
}

// interprets CMND_DEVICE-DEV_LIGHT command - positional light control
void dev_light(can_message_t *msg)
{
	//util_led(UTIL_LED_GREEN_BLINK_3X); // debug indicator
	msg->COMMAND &= 0x03; // clear everything except B1:0, which is the led indicator

	if (msg->COMMAND == 0x00) // front positional light
	{
		fade(msg->ARGUMENT, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT); // fade front light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == 0x01) // yet undefined light
	{ // a Christmas light, maybe...
		util_led(UTIL_LED_GREEN_BLINK_2X); // debug indicator
		return;
	}

	if (msg->COMMAND == 0x02) // rear positional light
	{
		fade(msg->ARGUMENT, &OCR_REAR_LIGHT, OCR_MAX_REAR_LIGHT);  // fade rear light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == 0x03) // brake light
	{
		//util_led(UTIL_LED_GREEN_BLINK_4X); // debug indicator

		if (msg->ARGUMENT > OCR_MAX_BRAKE_LIGHT)
		{
			OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
			util_led(UTIL_LED_RED_BLINK_2X); // CHECKME: something drives this OCR dangerously up
		}
		else
			OCR_BRAKE_LIGHT = msg->ARGUMENT;
	}
}