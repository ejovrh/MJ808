#ifndef MJ808_H_
#define MJ808_H_

#include <inttypes.h>

#include "mj8x8.h"

// hardware pin layout definitions
#define MCP2561_standby_pin		B,	1,	1								// MCP2561 standby
#define PWM_rear_light_pin		B,	2,	2								// PWM - brake light
#define PWM_brake_light_pin		B,	3,	3								// PWM - brake light

#define MCP2515_INT_pin			D,	3,	3								// INT1

#define SPI_SS_MCP2515_pin		B,	4,	4								// SPI - SS
#define ICSP_DI_MISO			B,	5,	5								// SPI - MISO; aka. DI; if run in master mode this is ... MISO
#define ICSP_DO_MOSI			B,	6,	6								// SPI - MOSI; aka. DO; ditto
#define SPI_SCK_pin				B,	7,	7								// SPI - SCK
// hardware pin layout definitions


typedef struct															// struct describing devices on MJ818
{
	//volatile uint8_t timer_counter[2];								// timer counter array
	volatile leds_t	*led;												// pointer to LED structure
	volatile can_t *can;												// pointer to the CAN structure
	volatile attiny4313_t *mcu;											// pointer to MCU structure
	volatile mj8x8_t *mj8x8;											// pointer to the base class
} mj818_t;

volatile mj818_t device;												// forward declaration of mj818_t struct for mj818


volatile mj818_t *mj818_ctor(volatile mj818_t *self, volatile mj8x8_t *base);




#endif /* MJ808_H_ */