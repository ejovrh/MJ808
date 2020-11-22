#ifndef MJ828_H_
#define MJ828_H_

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"
#include "phototransistor.h"

#define MJ828_CAN_ID	(PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D)

// definitions of device/PCB layout-specific hardware pins
#define MCP2561_standby_pin						D,	6,	6				// MCP2561 standby

#define PUSHBUTTON_pin							D,	0,	0				// Pushbutton
#define PHOTOTRANSISTOR_COLLECTOR_pin			D,	1,	1				// IR Phototransistor Collector - high - darkness, low - daylight
#define HIGH_IN_pin								D,	4,	4				// active low hall-effect switch, brake lever high beam signal
#define BREAK_IN_pin							D,	5,	5				// active low hall-effect switch, brake lever break light signal

#define LED_CP4_pin								B,	0,	0				// LED Charlie-plexed pin4
#define LED_CP3_pin								B,	1,	1				// LED Charlie-plexed pin3
#define LED_CP2_pin								B,	2,	2				// LED Charlie-plexed pin2
#define LED_CP1_pin								B,	3,	3				// LED Charlie-plexed pin1
// definitions of device/PCB layout-specific hardware pins

enum mj828_leds															// enum of lights on this device
{
	Red,																// 0 - brake light  on/off
	Green,																// 1 - lights on/off
	Blue,																// 2 - high beam on/off
	Yellow,																// 3 - auto mode on/off (turn lights on via phototransistor)
	Battery_LED1,														// 4 - battery indicator
	Battery_LED2,														// 5 - battery indicator
	Battery_LED3,														// 6 - battery indicator
	Battery_LED4														// 7 - battery indicator
} ;

enum mj828_buttons														// enum of buttons on this device
{
	Pushbutton,															// pushbutton on device
	HighBeam,															// brake lever pressed forward
	BrakeLight															// brake lever in brake position
};

typedef struct															// struct describing devices on MJ828
{
	mj8x8_t *mj8x8;														// pointer to the base class
	composite_led_t *led;												// pointer to LED structure
	button_t *button;													// array of button_t - two buttons
	phototransistor_t *phototransistor;									// pointer to phototransistor structure
} mj828_t;

void mj828_ctor();														// declare constructor for concrete class

extern mj828_t * const Device;											// declare pointer to public struct part

#endif /* MJ828_H_ */