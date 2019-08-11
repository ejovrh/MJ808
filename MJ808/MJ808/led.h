#ifndef LED_H_
#define LED_H_

#include <inttypes.h>

#if defined(MJ808_)	|| defined(MJ818_)									// leds_t struct for mj808/mj818
typedef struct															// struct describing a single generic LED
{
	void (*Shine)(uint8_t in_val);										// virtual function for LED operation
} individual_led_t;

typedef struct	leds_t													// struct describing LEDs on any device
{
	individual_led_t *led;												// "virtual" pointer to array of LEDs present on particular device

	void (*virtual_led_ctor)(volatile struct leds_t *self);				// virtual constructor
} leds_t;
#endif

#if defined(MJ828_)														// leds_t struct for mj828
typedef struct															// struct describing a single generic LED
{
	// depreciated start
	uint8_t on : 1;														// 0 - off, 1 - on
	//uint8_t blink_count : 3;											// 000 - no blink, 001 - blink 1x, 010 - blink 2x, 011 - blink 3x, 100 - blink forever
	// depreciated end
	void (*Shine)(uint8_t in_val);
} individual_led_t;

typedef struct	leds_t													// struct describing LEDs on device MJ828
{
	individual_led_t *led;
	// depreciated start
	//uint8_t led_count : 3;												// number of LEDs on device, max 8
	uint8_t flag_any_glow : 1;											// flag indicating if anything at all shall glow
	// depreciated end
	void (*virtual_led_ctor)(volatile struct leds_t *self);				// virtual constructor
} leds_t;
#endif

extern volatile leds_t LED;												// declare LED object

#endif /* LED_H_ */