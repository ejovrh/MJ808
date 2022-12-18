#ifndef CORE_INC_LED_LED_H_
#define CORE_INC_LED_LED_H_

typedef struct	// struct describing a primitive LED - the leaf
{
	void (*Shine)(const uint8_t in_val);	// virtual function for LED operation of a primitive (i.e.) single LED
} primitive_led_t;

typedef struct composite_led_t	// struct describing the aggregate of all LEDs on a device - the composite
{
	void (*Handler)(void);	//
	void (*Shine)(const uint8_t in_val);	// virtual function for LED operation of composite LEDs
	primitive_led_t *led;  // "virtual" pointer to array of primitive LEDs - pointer to leaves

//composite_led_t *(*virtual_led_ctor)(composite_led_t * const self);	// virtual  constructor
} composite_led_t;

#endif /* CORE_INC_LED_LED_H_ */
