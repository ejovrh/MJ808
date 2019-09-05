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
																		// component pattern
typedef struct															// struct describing a primitive LED - the leaf
{
	void (*Shine)(const uint8_t in_val);								// virtual function for LED operation of a primitive (i.e.) single LED
} primitive_led_t;

typedef struct composite_led_t											// struct describing the aggregate of all LEDs on a device - the composite
{
	void (*Shine)(const uint8_t in_val);								// virtual function for LED operation of composite LEDs
	primitive_led_t *led;												// "virtual" pointer to array of primitive LEDs - pointer to leaves
	ledflags_t *flags;													// pointer to bitfield struct for flagging individual LEDs to be lit

	//composite_led_t *(*virtual_led_ctor)(composite_led_t * const self);		// virtual  constructor
} composite_led_t;

#endif /* LED_H_ */