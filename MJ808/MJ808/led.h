#ifndef LED_H_
#define LED_H_

typedef struct															// struct describing a single generic LED
{
	#if defined(MJ828_)
	// TODO - used on mj828 only
	uint8_t Flag_On : 1;												// 0 - off, 1 - on
	#endif

	void (*Shine)(uint8_t in_val);										// virtual function for LED operation
} individual_led_t;

typedef struct	leds_t													// struct describing LEDs on any device
{
	#if defined(MJ828_)
	// TODO - used on mj828 only
	uint8_t flag_any_glow : 1;											// flag indicating if anything at all shall glow
	#endif
	individual_led_t *led;												// "virtual" pointer to array of LEDs present on particular device

	void (*virtual_led_ctor)(volatile struct leds_t *self);				// virtual constructor
} leds_t;

extern volatile leds_t LED;												// declare LED object

#endif /* LED_H_ */