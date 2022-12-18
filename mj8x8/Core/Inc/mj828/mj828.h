#ifndef CORE_INC_MJ828_MJ828_H_
#define CORE_INC_MJ828_MJ828_H_

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"

// definitions of device/PCB layout-specific hardware pins
// PRT - definitions of device/PCB layout-specific hardware pins
// definitions of device/PCB layout-specific hardware pins

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
};

enum mj828_buttons														// enum of buttons on this device
{
	  Right,
	  Left
};

typedef struct														// struct describing devices on MJ828
{
	mj8x8_t *mj8x8;														// pointer to the base class
	composite_led_t *led;												// pointer to LED structure
	button_t *button;													// array of button_t - two buttons
} mj828_t;

void mj828_ctor();										// declare constructor for concrete class

extern mj828_t *const Device;						// declare pointer to public struct part

#endif /* CORE_INC_MJ828_MJ828_H_ */
