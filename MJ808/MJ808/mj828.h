#ifndef MJ828_H_
#define MJ828_H_

#include "mj8x8.h"
#include "led.h"
#include "button.h"

// definitions of device/PCB layout-specific hardware pins
#define MCP2561_standby_pin		B,	3,	3								// MCP2561 standby

#define PUSHBUTTON1_pin			D,	0,	0								// Pushbutton 1 - sw1
#define PUSHBUTTON2_pin			D,	1,	1								// Pushbutton 2 - sw2

#define LED_CP4_pin				D,	6,	6								// LED Charlie-plexed pin4
#define LED_CP3_pin				B,	0,	0								// LED Charlie-plexed pin3
#define LED_CP2_pin				B,	1,	1								// LED Charlie-plexed pin2
#define LED_CP1_pin				B,	2,	2								// LED Charlie-plexed pin1
// definitions of device/PCB layout-specific hardware pins

void charlieplexing_handler(volatile composite_led_t *in_led);			// handles LEDs in charlieplexed configuration

enum mj828_leds															// enum of lights on this device
{
	Red,																// 0
	Green,																// 1
	Blue,																// 2
	Yellow,																// 3
	Battery_LED1,														// 4
	Battery_LED2,														// 5
	Battery_LED3,														// 6
	Battery_LED4														// 7
} ;

enum mj828_buttons														// enum of buttons on this device
{
	Right,
	Left
};

typedef struct															// struct describing devices on MJ828
{
	volatile mj8x8_t *mj8x8;											// pointer to the base class
	volatile composite_led_t *led;										// pointer to LED structure
	volatile button_t *button;											// array of button_t - two buttons
} mj828_t;

 void mj828_ctor();

extern mj828_t * const Device;											// declare pointer to public struct part

#endif /* MJ828_H_ */