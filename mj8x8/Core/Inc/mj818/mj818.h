#ifndef CORE_INC_MJ818_MJ818_H_
#define CORE_INC_MJ818_MJ818_H_

#include "mj8x8\mj8x8.h"
#include "led\led.h"

// definitions of device/PCB layout-dependent hardware pins
// PRT - definitions of device/PCB layout-dependent hardware pins
// definitions of device/PCB layout-dependent hardware pins

enum mj818_leds  // enum of lights on this device
{
	  Rear,
	  Brake
};

typedef struct	// struct describing devices on MJ818
{
	mj8x8_t *mj8x8;  // pointer to the base class
	composite_led_t *led;  // pointer to LED structure
} mj818_t;

void mj818_ctor();	// declare constructor for concrete class

extern mj818_t *const Device;  // declare pointer to public struct part

#endif /* CORE_INC_MJ818_MJ818_H_ */
