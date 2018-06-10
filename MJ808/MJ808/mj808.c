#include <inttypes.h>
#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mj808.h"

void blink_red(uint8_t led, uint8_t count)
{
	do
	{
		gpio_toggle(RED_LED_pin);
		_delay_ms(BLINK_DELAY);
		gpio_toggle(RED_LED_pin);
		_delay_ms(BLINK_DELAY);
	} while (--count);
}