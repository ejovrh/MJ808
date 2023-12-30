#ifndef CORE_INC_MJ514_AS5601_H_
#define CORE_INC_MJ514_AS5601_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

/*	AS5601 7-bit I2C address is 0x36
 * 		(DS. p. 10)
 * 		(0x36 << 1) = 0x6C
 */
#define AS5601_I2C_ADDR  (uint16_t) 0xD6 // DS. p. 50 - left-shifted 0x6B

#define FULL_REVOLUTION 12345.0	// ticks for a full 360 deg. revolution

typedef struct	// struct describing the Rotary Encoder functionality
{
	volatile rotation_t Rotation :1;  //

	float (*CountRotation)(void);  //
	void (*Read)(void);  //
	void (*Write)(void);  //
} as5601_t;

as5601_t* as5601_ctor(void);	// the AS5601 constructor

#endif

#endif /* CORE_INC_MJ514_AS5601_H_ */
