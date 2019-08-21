#ifndef MJ808_H_
#define MJ808_H_

#include "mj8x8.h"
#include "led.h"
#include "button.h"

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
	volatile mj8x8_t *mj8x8;											// pointer to the base class
	volatile leds_t	*led;												// pointer to LED structure
	volatile button_t *button;											// array of button_t - one buttons
} mj808_t;

void mj808_ctor(volatile mj808_t * const self, volatile leds_t *led, volatile button_t *button);

extern volatile mj808_t Device;

#endif /* MJ808_H_ */