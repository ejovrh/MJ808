#ifndef COS_H_
#define COS_H_

#include <avr\io.h>														// microcontroller hardware defines

#include "mj8x8\mj8x8.h"												// base infrastructure
#include "tps630701\tps630701.h"										// BuckBoost controller object
#include "mcp73871\mcp73871.h"											// LiIon Charger object
#include "rect\rect.h"													// AC voltage regulator object

// definitions of device/PCB layout-dependent hardware pins
#define	COMPARATOR_IN_pin		B,	0,	0								// Comparator input pin - zero cross from dynamo
#define	COMPARATOR_REF_pin		B,	1,	1								// Comparator reference pin = GND
#define TPS630701_PWM_pin		B,	2,	2								// TPS630701_PWM PWM Control Pin (PS/SYNC) // TODO: use pullup resistor?
#define	MCP2561_standby_pin		B,	3,	3								// MCP2561 standby, controlled in mcp2515.c

#define	MP3221_EN_pin			D,	0,	0								// Èos 5V0 output stage boost converter enable pin, pulled low
#define	SPI_SS_MCP23S08_pin		D,	1,	1								// MCP23S08 Port Expander Slave Select
#define INT_MCP2515_pin			D,	3,	3								// MCP2515 CAN Controller interrupt pin
#define INT_MCP23S08_pin		D,	4,	4								// MCP23S08 Port Expander Interrupt pin
#define	SPI_SS_AD5160_pin		D,	5,	5								// AD5160 Port Expander Slave Select
#define	SPI_SS_LMP92064SD_pin	D,	6,	6								// LMP92064SD Voltage/Current meter Slave Select, Buck-Boost-out (pre-load)
// definitions of device/PCB layout-dependent hardware pins

#define OCR_BUCK_BOOST OCR0A											// Output Compare Register for PWM of BuckBoost regulator

#define OP_PARAM_ARRAY_SIZE		6										// size of Èos operational parameters uint8_t data array
// uint8_t OpParamArray fields:
#define TPS630701_IOUT_MSB		0										// LMP92064SD Datasheet p.16, COUT_DATA_MSB[7:0]
#define TPS630701_IOUT_LSB		1										// LMP92064SD Datasheet p.16, COUT_DATA_MSB[7:0]
#define TPS630701_UOUT_MSB		2										// LMP92064SD Datasheet p.16, COUT_DATA_MSB[7:0]
#define TPS630701_UOUT_LSB		3										// LMP92064SD Datasheet p.16, COUT_DATA_MSB[7:0]
#define TPS630701_PWM			4										// Buck-Boost PWM input on PS/SYNC
#define MISC_STATUS_BITS		5										// byte containing misc. Èos device status bits: rectifier mode, charger mode, 6V0 out
// field 7 (which would be a full byte) is left out because of CAN msg limitations: payload max. 8 bytes, COMMAND takes one --> only 7

// OpParamArray[MISC_STATUS_BITS] bit field:
#define MISC_STATUS_BITS_FOO				7							// free for future use
#define MISC_STATUS_BITS_MCP73871_STAT2		6							// MCP73871 LiIon Charger/Powerpath controller mode bits
#define MISC_STATUS_BITS_MCP73871_STAT1		5							// MCP73871 LiIon Charger/Powerpath controller mode bits
#define MISC_STATUS_BITS_MCP73871_PG		4							// MCP73871 LiIon Charger/Powerpath controller mode bits
#define MISC_STATUS_BITS_MP3221_ENABLED		3							// MP3221 6V0 out enabled true/false
#define MISC_STATUS_BITS_AC2				2							// AC rectifier operational mode bits:
#define MISC_STATUS_BITS_AC1				1							// TODO - define rectifier mode bits
#define MISC_STATUS_BITS_AC0				0							//

#define MASK_MISC_STATUS_BITS_MCP73871			0x70
#define MASK_MISC_STATUS_BITS_MP3221_ENABLED	0x08
#define MASK_MISC_STATUS_BITS_AC				0x07

/*
#define COS_6V0_OUT_ENABLE			3									// Èos output enable yes/no
#define COS_CONFIG_AC_REC2			2									// AC rectifier opmode:
#define COS_CONFIG_AC_REC1			1									// 00 - , 01 - , 10 - , 11 -
#define COS_CONFIG_AC_REC0			0									//

#define COS_CONFIG_BUCKBOOST_PWM	2									// PWM value for MCP630701
#define COS_CHARGE_CURRENT			4									// MCP73871 charge current control resistor
*/

typedef struct															// struct describing devices on MJ808
{
	void (* Cos6V0OutputEnabled)(const uint8_t in_val);					// enable/disable the Èos 5V0 output boost converter

	mj8x8_t *mj8x8;														// pointer to the base class
	tps630701_t *BuckBoost;												// 5V0 out Buck-Boost converter, powered by rectified dynamo, powers LiIon Charger
	mcp73871_t *LiIonCharger;											// LiIon Charger & Powerpath controller, powered by 5V0, powers downstream with LiIon cell voltage (2.8-4.2V)
	rect_t *Rect;															// AC regulator: Graetz bridge, tuning capacitors on/off, Delon voltage doubler on/off

	uint8_t OpParamArray[OP_PARAM_ARRAY_SIZE];							//
	float ACfreq;														//
} cos_t;

void cos_ctor();														// declare constructor for concrete class

extern cos_t * const Device;											// declare pointer to public struct part

#endif /* COS_H_ */