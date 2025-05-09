#ifndef CORE_INC_MJ514_AS5601_H_
#define CORE_INC_MJ514_AS5601_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

typedef enum as5601_reg_t  // register to array index translation
{
	  ZMCO = 0,  // count of ZPOS burn commands, DS. p. 22
	  ZPOS = 1,  //	zero position config register, DS. p. 20
	  CONF = 2,  //	configuration register, DS. p. 20
	  ABN = 3,	// output position and update rate config register, DS. p. 20
	  PUSHTHR = 4,	// pushbutton threshold config register, DS. p. 20
	  RAW_ANGLE = 5,	// raw angle register, DS. p. 21
	  ANGLE = 6,	// angle register, DS. p. 21
	  STATUS = 7,  // status register, DS. p. 21
	  AGC = 8,	// automatic gain control register, DS. p. 21
	  MAGNITUDE = 9,	// magnitude register, DS. p. 21
	  BURN = 10  // non-volatile burn register, DS. p. 22
} as5601_reg_t;

typedef struct	// struct describing the Rotary Encoder functionality
{
	uint16_t (*Read)(const as5601_reg_t Register);  // returns 2 bytes of register values from device register
	void (*Write)(const as5601_reg_t Register, const uint16_t data);  // writes 2 bytes of data into device register
} as5601_t;

as5601_t* as5601_ctor(void);	// the AS5601 constructor

#endif

#endif /* CORE_INC_MJ514_AS5601_H_ */
