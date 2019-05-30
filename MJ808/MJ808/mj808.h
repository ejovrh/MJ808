#ifndef MJ808_H_
#define MJ808_H_

#include <inttypes.h>

#include "mj8x8.h"

// hardware pin layout definitions
#define	GREEN_LED_pin			D,	0,	0								// green LED
#define	RED_LED_pin				D,	1,	1								// red LED
#define	MCP2515_INT_pin			D,	3,	3								// INT1
#define	PUSHBUTTON_pin			D,	4,	4								// tactile pushbutton

#define	MCP2561_standby_pin		B,	1,	1								// MCP2561 standby
#define	PWM_front_light_pin		B,	2,	2								// PWM - front light

#define	SPI_SS_MCP2515_pin		B,	4,	4								// SPI - SS
#define	ICSP_DI_MISO			B,	5,	5								// SPI - MISO; aka. DI; if run in master mode this is ... MISO
#define	ICSP_DO_MOSI			B,	6,	6								// SPI - MOSI; aka. DO; ditto
#define	SPI_SCK_pin				B,	7,	7								// SPI - SCK
// hardware pin layout definitions


typedef struct															// struct describing devices on MJ808
{
	volatile mj8x8_t *mj8x8;											// pointer to the base class
	volatile button_t button[1];										// array of button_t - one buttons
	volatile uint8_t button_count : 2;									// max. 4 buttons
	volatile leds_t	*led;												// pointer to LED structure
} mj808_t;

volatile mj808_t device;												// forward declaration of mj828_t struct for mj828

volatile mj808_t *mj808_ctor(volatile mj808_t *self, volatile mj8x8_t *base);




#endif /* MJ808_H_ */