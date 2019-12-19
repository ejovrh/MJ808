#ifndef COS_H_
#define COS_H_

#include "mj8x8\mj8x8.h"												// base infrastructure
#include "tps630701\tps630701.h"										// BuckBoost controller object
#include "mcp73871\mcp73871.h"											// LiIon Charger object
#include "reg\reg.h"													// AC voltage regulator object

// definitions of device/PCB layout-dependent hardware pins
#define	COMPARATOR_IN_pin		B,	0,	0								// Comparator input pin - zero cross from dynamo
#define	COMPARATOR_REF_pin		B,	1,	1								// Comparator reference pin = GND

// TODO: use pullup resistor?
#define TPS630702_PWM_pin		B,	2,	2								// TPS630702_PWM PWM Control Pin (PS/SYNC)

#define	MCP2561_standby_pin		B,	3,	3								// MCP2561 standby, controlled in mcp2515.c

#define	MP3221_EN_pin			D,	2,	2								// 5V0 output stage boost converter enable pin, pulled low
#define INT_MCP2515_pin			D,	3,	3								// MCP2515 CAN Controller interrupt pin
#define INT_MCP23S08_pin		D,	4,	4								// MCP23S08 Port Expander Interrupt pin
// definitions of device/PCB layout-dependent hardware pins

#define OCR_BUCK_BOOST OCR0A											// Output Compare Register for PWM of BuckBoost regulator

typedef struct															// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;														// pointer to the base class
	tps630701_t *BuckBoost;												// 5V0 out Buck-Boost converter, powered by rectified dynamo, powers LiIon Charger
	mcp73871_t *LiIonCharger;											// LiIon Charger & Powerpath controller, powered by 5V0, powers downstream with LiIon cell voltage (2.8-4.2V)
	reg_t *Reg;															// AC regulator: Graetz bridge, tuning capacitors on/off, Delon voltage doubler on/off
	volatile uint8_t ACFreq;											// length of one Dynamo AC period in n cycles, as determined by comparator and measured by input capture
} cos_t;

void cos_ctor();														// declare constructor for concrete class

extern cos_t * const Device;											// declare pointer to public struct part

#endif /* COS_H_ */