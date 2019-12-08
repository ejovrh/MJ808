#ifndef COS_H_
#define COS_H_

#include "mj8x8\mj8x8.h"

// definitions of device/PCB layout-dependent hardware pins
#define	SPI_SS_MCP23S08_pin		D,	1,	1								// MCP23S08 Port Expander Slave Select
#define	COMPARATOR_IN_pin		B,	0,	0								// Comparator input pin
#define	COMPARATOR_REF_pin		B,	1,	1								// Comparator reference pin
#define	MCP2561_standby_pin		B,	2,	2								// MCP2561 standby
#define BCK_BST_PS_SYNC_pin		B,	3,	3								// Buck&Boost 5V0 out
#define	SPI_SS_MCP2515_pin		B,	4,	4								// SPI - SS
// definitions of device/PCB layout-dependent hardware pins

typedef struct															// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;														// pointer to the base class
} cos_t;

void cos_ctor();														// declare constructor for concrete class

extern cos_t * const Device;											// declare pointer to public struct part

#endif /* COS_H_ */