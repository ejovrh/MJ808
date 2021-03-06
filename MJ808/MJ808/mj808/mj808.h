#ifndef MJ808_H_
#define MJ808_H_

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"

// definitions of device/PCB layout-dependent hardware pins
#define	GREEN_LED_pin			D,	0,	0								// green LED
#define	RED_LED_pin				D,	1,	1								// red LED
#define	PUSHBUTTON_pin			D,	4,	4								// tactile pushbutton

#define	MCP2561_standby_pin		B,	1,	1								// MCP2561 standby
#define	PWM_front_light_pin		B,	2,	2								// PWM - front light
// definitions of device/PCB layout-dependent hardware pins

enum mj808_leds															// enum of lights on this device
{
	Utility,
	Front
};

enum mj808_buttons														// enum of buttons on this device
{
	Center
};

typedef struct															// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;														// pointer to the base class
	composite_led_t *led;												// pointer to LED structure
	button_t *button;													// array of button_t - one buttons
} mj808_t;

void mj808_ctor();														// declare constructor for concrete class

extern mj808_t * const Device;											// declare pointer to public struct part

#endif /* MJ808_H_ */