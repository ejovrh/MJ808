#ifndef MJ818_LED_C_
#define MJ818_LED_C_

#include <avr/interrupt.h>

#include "mj818\mj818.h"
#include "led\led.h"

#include "led\composite_led_actual.c"									// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

static primitive_led_t primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

extern void DoNothing(void);

// TODO - optimize
static void _wrapper_fade_mj818_rear(uint8_t value)
{
// TODO - optimize
	_fade(value, &OCR_REAR_LIGHT);
};

static void _wrapper_fade_mj818_brake(uint8_t value)
{
	// TODO - optimize
	_fade(value, &OCR_BRAKE_LIGHT);
};

inline void __component_led_mj818_device_on(const uint8_t val)
{
	_fade(val, &OCR_BRAKE_LIGHT);
	_fade(val, &OCR_REAR_LIGHT);
};

inline void __component_led_mj818_device_off(void)
{
	_fade(0x00, &OCR_BRAKE_LIGHT);
	_fade(0x00, &OCR_REAR_LIGHT);
};

static void _component_led_mj818(const uint8_t val)
{
	if (val)															// true - on, false - off
		__component_led_mj818_device_on(val);							// delegate indirectly to the leaves
	else
		__component_led_mj818_device_off();
};

static __composite_led_t __LED =
{
	.public.led = primitive_led,										// assign pointer to LED array
	.public.Shine = &_component_led_mj818,								// component part ("interface")
	.flags = 0
};

static composite_led_t *_virtual_led_ctorMJ818()
{
	__LED.public.led[Rear].Shine = &_wrapper_fade_mj818_rear;			// LED-specific implementation
	__LED.public.led[Brake].Shine = &_wrapper_fade_mj818_brake;			// LED-specific implementation
	__LED.public.Handler = &DoNothing;									// do noting

	return &__LED.public;												// return address of public part; calling code accesses it via pointer
};

#endif /* MJ808_LED_C_ */