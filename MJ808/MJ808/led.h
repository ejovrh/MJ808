#ifndef LED_H_
#define LED_H_

typedef union															// union describing LED flags
{
	struct																// individual bits - each bit corresponds to the device-specific LED enum
	{
		uint8_t bit0 : 1;												// LED indicated by enum. position 1
		uint8_t bit1 : 1;												// LED indicated by enum. position 2
		uint8_t bit2 : 1;												// LED indicated by enum. position 3
		uint8_t bit3 : 1;												// LED indicated by enum. position 4
		uint8_t bit4 : 1;												// LED indicated by enum. position 5
		uint8_t bit5 : 1;												// LED indicated by enum. position 6
		uint8_t bit6 : 1;												// LED indicated by enum. position 7
		uint8_t bit7 : 1;												// LED indicated by enum. position 8
	};
	uint8_t All;														// all bits at once
} ledflags_t;

typedef struct															// struct describing a single generic LED
{
	void (*Shine)(uint8_t in_val);										// virtual function for LED operation
} individual_led_t;

typedef struct	leds_t													// struct describing LEDs on any device
{
	individual_led_t *led;												// "virtual" pointer to array of LEDs present on particular device
	volatile ledflags_t *flags;											// pointer to bitfield struct for flagging individual LEDs to be lit

	void (*virtual_led_ctor)(volatile struct leds_t *self);				// virtual  constructor
} leds_t;

extern volatile ledflags_t LEDFlags;									// declare LEDFlags object
extern volatile leds_t LED;												// declare LED object

#endif /* LED_H_ */