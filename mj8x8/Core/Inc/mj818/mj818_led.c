#ifndef MJ818_LED_C_
#define MJ818_LED_C_

#include "main.h"
#if defined(MJ818_)	// if this particular device is active

#include "mj818\mj818.h"
#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

static primitive_led_t __primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

extern void DoNothing(void);

//
static void _wrapper_fade_mj818_rear(uint8_t value)
{  //	TODO - implement _fade(val, &OCR_REAR_LIGHT);
	;
}

//
static void _wrapper_fade_mj818_brake(uint8_t value)
{  //	TODO - implement _fade(val, &OCR_BRAKE_LIGHT);
	;
}

static inline void __component_led_mj818_device_on(const uint8_t val)
{
	//	TODO - implement _fade(val, both lights);

	OCR_BRAKE_LIGHT = val;
	OCR_REAR_LIGHT = val;
}

static inline void __component_led_mj818_device_off(void)
{
	//	TODO - implement _fade(val, both lights);

	OCR_BRAKE_LIGHT = 0;
	OCR_REAR_LIGHT = 0;
}

static void _component_led_mj818(const uint8_t val)
{
	if(val)  // true - on, false - off
		__component_led_mj818_device_on(val);  // delegate indirectly to the leaves
	else
		__component_led_mj818_device_off();
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,  // assign pointer to LED array
	.public.Shine = &_component_led_mj818,	// component part ("interface")
	.flags = 0	//
	};

static composite_led_t* _virtual_led_ctorMJ818()
{
	__LED.public.led[Rear].Shine = &_wrapper_fade_mj818_rear;  // LED-specific implementation
	__LED.public.led[Brake].Shine = &_wrapper_fade_mj818_brake;  // LED-specific implementation
	__LED.public.Handler = &DoNothing;	// do noting

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ818_

#endif /* MJ808_LED_C_ */
