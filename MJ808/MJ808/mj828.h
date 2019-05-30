#ifndef MJ828_H_
#define MJ828_H_

#include <inttypes.h>

#include "mj8x8.h"

// hardware pin layout definitions
#define MCP2561_standby_pin		B,	3,	3								// MCP2561 standby
#define SPI_SS_MCP2515_pin		B,	4,	4								// SPI - SS
#define ICSP_DI_MISO			B,	5,	5								// SPI - MISO; aka. DI; if run in master mode this is ... MISO
#define ICSP_DO_MOSI			B,	6,	6								// SPI - MOSI; aka. DO; ditto
#define SPI_SCK_pin				B,	7,	7								// SPI - SCK

#define PUSHBUTTON1_pin			D,	0,	0								// Pushbutton 1 - sw1
#define PUSHBUTTON2_pin			D,	1,	1								// Pushbutton 2 - sw2
#define MCP2515_INT_pin			D,	3,	3								// INT1

#define LED_CP4_pin				D,	6,	6								// LED Charlie-plexed pin4
#define LED_CP3_pin				B,	0,	0								// LED Charlie-plexed pin3
#define LED_CP2_pin				B,	1,	1								// LED Charlie-plexed pin2
#define LED_CP1_pin				B,	2,	2								// LED Charlie-plexed pin1
// hardware pin layout definitions


typedef struct															// struct describing devices on MJ828
{
	//volatile uint8_t timer_counter[2];								// timer counter array
	volatile button_t button[2];										// array of button_t - two buttons
	volatile leds_t	*led;												// pointer to LED structure
	volatile can_t *can;												// pointer to the CAN structure
	volatile attiny4313_t *mcu;											// pointer to MCU structure
	volatile mj8x8_t *mj8x8;											// pointer to the base class
} mj828_t;

volatile mj828_t device;												// forward declaration of mj828_t struct for mj828

volatile mj828_t *mj828_ctor(volatile mj828_t *self, volatile mj8x8_t *base);




#endif /* MJ828_H_ */