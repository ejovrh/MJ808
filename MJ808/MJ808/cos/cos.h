#ifndef COS_H_
#define COS_H_

#include "mj8x8\mj8x8.h"												// base infrastructure
#include "ad5160\ad5160.h"												// SPI Digital Potentiometer
#include "lmp92064sd\lmp92064sd.h"										// SPI Current/Voltage sensor
#include "mcp23s08/mcp23s08.h"											// SPI port expander

// definitions of device/PCB layout-dependent hardware pins
#define	COMPARATOR_IN_pin		B,	0,	0								// Comparator input pin - zero cross from dynamo
#define	COMPARATOR_REF_pin		B,	1,	1								// Comparator reference pin = GND
#define TPS630702_PWM_pin		B,	2,	2								// TPS630702_PWM PWM Control Pin (PS/SYNC)
#define	MCP2561_standby_pin		B,	3,	3								// MCP2561 standby, controlled in mcp2515.c

#define INT_MCP2515_pin			D,	3,	3								// MCP2515 CAN Controller interrupt pin
#define INT_MCP23S08_pin		D,	4,	4								// MCP23S08 Port Expander Interrupt pin
// definitions of device/PCB layout-dependent hardware pins

#define OCR_BUCK_BOOST OCR0A											// Output Compare Register for PWM of BuckBoost regulator

typedef struct															// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;														// pointer to the base class
} cos_t;

void cos_ctor();														// declare constructor for concrete class

extern cos_t * const Device;											// declare pointer to public struct part

#endif /* COS_H_ */