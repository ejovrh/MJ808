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

typedef struct															// struct describing devices on MJ808
{
	volatile mj8x8_t *mj8x8;											// pointer to the base class
	volatile leds_t	*led;												// pointer to LED structure
	volatile button_t button[1];										// array of button_t - one buttons
	volatile uint8_t button_count : 2;									// max. 4 buttons
} mj808_t;

volatile mj808_t *mj808_ctor(volatile mj808_t *self, volatile mj8x8_t *base, volatile leds_t *led, volatile button_t *button, volatile message_handler_t *msg);

static volatile mj808_t device;

#endif /* MJ808_H_ */