#ifndef MJ818_H_
#define MJ818_H_

#include "mj8x8.h"
#include "led.h"

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
	volatile mj8x8_t *mj8x8;											// pointer to the base class
	volatile leds_t	*led;												// pointer to LED structure
} mj818_t;

void mj818_ctor(volatile mj818_t *self, volatile mj8x8_t *base, volatile leds_t *led, volatile message_handler_t *msg);

extern volatile mj818_t Device;

#endif /* MJ818_H_ */