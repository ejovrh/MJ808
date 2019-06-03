#ifndef LED_H_
#define LED_H_

#include <inttypes.h>

enum led_enum															// TODO - investigate if enum actually has move value over defines
{
	red,
	green,
	blue,
	yellow,
	battery_led1,
	battery_led2,
	battery_led3,
	battery_led4
} ;

typedef struct															// struct describing a single generic LED
{
	uint8_t on : 1;														// 0 - off, 1 - on
	uint8_t blink_count : 3;											// 000 - no blink, 001 - blink 1x, 010 - blink 2x, 011 - blink 3x, 100 - blink forever
} led_t;

#if defined(MJ808_)	|| defined(MJ818_)									// leds_t struct for mj808/mj818
typedef struct															// struct describing LEDs on device MJ828
{
	led_t leds[2];														// array of led_t - one for each LED
	uint8_t led_count : 3;												// number of LEDs on device, max 8
	uint8_t flag_any_glow : 1;											// flag indicating if anything at all shall glow

	void (*digest)(volatile can_msg_t *in_msg);
} leds_t;
#endif

#if defined(MJ828_)														// leds_t struct for mj828
typedef struct															// struct describing LEDs on device MJ828
{
	led_t leds[8];														// array of led_t - one for each LED
	uint8_t led_count : 3;												// number of LEDs on device, max 8

	uint8_t flag_any_glow : 1;											// flag indicating if anything at all shall glow

	void (*digest)(volatile can_msg_t *in_msg);
} leds_t;
#endif

void led_ctor(volatile leds_t *self);									// not really needed for now

#endif /* LED_H_ */