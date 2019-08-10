#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "mj8x8.h"
#include "led.h"

// interprets CMND_UTIL_LED command - utility LED (red, green, on, off, blink)
void util_led(uint8_t in_val)
{
	uint8_t led = 0;													// holds the pin of the LED: D0 - green (default), D1 - red

	if (in_val & _BV(B3))												// determine B3 value: red or green (default)
	led = 1;															// red

	in_val &= 7;														// clear everything except B2:0, which is the blink count (1-6)

	if (in_val == 0x00)													// B3:B0 is 0 - turn off
	{
		PORTD |= (1<<led);												// clear bit
		return;
	}

	if (in_val == 0x07)													// B3:B0 is 7 - turn on
	{
		PORTD &= ~(1<<led);												// set bit
		return;
	}

	while (in_val--)													// blink loop
	{
		// TODO - util_led() - get rid of _delay_ms()
		_delay_ms(BLINK_DELAY);											// waste a few cycles (non-blocking)
		PORTD ^= (1<<led);												// toggle the led pin
		_delay_ms(BLINK_DELAY);											// waste a few cycles (non-blocking)
		PORTD ^= (1<<led);												// toggle the led pin
	}
};

// private function - fades *ocr to value (or ocr_max) - up to OCR_MAX or down to 0x00
void fade(uint8_t value, volatile uint8_t *ocr)
{
	cli();																// if without cli(), *ocr gets corrupted; im suspecting an ISR while ocr is incrementin/decrementing
	//	hence an atomic fade()

	if (value > *ocr)													// we need to get brighter
	{
		while (++*ocr < value)											// loop until we match the OCR with the requested value & increment the OCR
			_delay_ms(5);												// delay it a bit for visual stimulus ...

		sei();															// enable interrupts
		return;
	}

	if (value < *ocr)													// we need to get dimmer & decrement the OCR
	{
		while (--*ocr > value)											// loop until we match the OCR with the requested value
		{
			#if defined(MJ808_)
			_delay_ms(2.5);												// delay it a bit for visual stimulus ...
			#endif
			#if defined(MJ818_)
			_delay_ms(1);												// delay it a bit for visual stimulus ...
			#endif
		}

		sei();															// enable interrupts
		return;
	}
};

volatile leds_t LED __attribute__ ((section (".data")));				// define LED object and put it into .data