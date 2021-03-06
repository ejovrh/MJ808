#ifndef MJ818_H_
#define MJ818_H_

#include "mj8x8\mj8x8.h"
#include "led\led.h"

// definitions of device/PCB layout-dependent hardware pins
#define MCP2561_standby_pin		B,	1,	1								// MCP2561 standby
#define PWM_rear_light_pin		B,	2,	2								// PWM - brake light
#define PWM_brake_light_pin		B,	3,	3								// PWM - brake light
// definitions of device/PCB layout-dependent hardware pins

enum mj818_leds															// enum of lights on this device
{
	Rear,
	Brake
};

typedef struct															// struct describing devices on MJ818
{
	mj8x8_t *mj8x8;														// pointer to the base class
	composite_led_t *led;												// pointer to LED structure
} mj818_t;

void mj818_ctor();														// declare constructor for concrete class

extern mj818_t * const Device;											// declare pointer to public struct part

#endif /* MJ818_H_ */