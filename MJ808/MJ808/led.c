#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "mj8x8.h"
#include "led.h"

// private function - fades *ocr to value (or ocr_max) - up to OCR_MAX or down to 0x00
void _fade(const uint8_t value, volatile uint8_t *ocr)
{
	cli();																// if without cli(), *ocr gets corrupted; im suspecting an ISR while ocr is incrementin/decrementing
	//	hence an atomic fade()

	if (value > *ocr)													// we need to get brighter
	{
		while (++*ocr < value)											// loop until we match the OCR with the requested value & increment the OCR
		_delay_ms(5);													// delay it a bit for visual stimulus ...

		sei();															// enable interrupts
		return;
	}

	if (value < *ocr)													// we need to get dimmer & decrement the OCR
	{
		while (--*ocr > value)											// loop until we match the OCR with the requested value
		_delay_ms(1);												// delay it a bit for visual stimulus ...

		sei();															// enable interrupts
		return;
	}
};

volatile leds_t LED __attribute__ ((section (".data")));				// define LED object and put it into .data