#ifndef LED_H_
#define LED_H_
																		// component pattern
typedef struct															// struct describing a primitive LED - the leaf
{
	void (*Shine)(const uint8_t in_val);								// virtual function for LED operation of a primitive (i.e.) single LED
} primitive_led_t;

typedef struct composite_led_t											// struct describing the aggregate of all LEDs on a device - the composite
{
	void (*Shine)(const uint8_t in_val);								// virtual function for LED operation of composite LEDs
	primitive_led_t *led;												// "virtual" pointer to array of primitive LEDs - pointer to leaves
	uint8_t flags;														// uint8_t interpreted bit-wise for flagging individual LEDs to be lit

	//composite_led_t *(*virtual_led_ctor)(composite_led_t * const self);		// virtual  constructor
} composite_led_t;

#endif /* LED_H_ */