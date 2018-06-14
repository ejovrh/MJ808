#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mj808.h"

// controls utility LED (red, green, on, off, blink)
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
		_delay_ms(BLINK_DELAY); //TODO: replace with timer
		PORTD ^= (1<<led); // toggle the led pin
	}
}