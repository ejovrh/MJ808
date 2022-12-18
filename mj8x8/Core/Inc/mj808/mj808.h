#ifndef CORE_INC_MJ808_MJ808_H_
#define CORE_INC_MJ808_MJ808_H_

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"

// definitions of device/PCB layout-dependent hardware pins
// PRT - definitions of device/PCB layout-dependent hardware pins
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

typedef struct														// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;														// pointer to the base class
	composite_led_t *led;												// pointer to LED structure
	button_t *button;													// array of button_t - one buttons
} mj808_t;

void mj808_ctor(void);								// declare constructor for concrete class

extern mj808_t *const Device;						// declare pointer to public struct part

#endif /* CORE_INC_MJ808_MJ808_H_ */
