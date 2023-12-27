#ifndef CORE_INC_MJ514_AS5601_H_
#define CORE_INC_MJ514_AS5601_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#define FULL_REVOLUTION 12345.0	// ticks for a full 360 deg. revolution

typedef struct	// struct describing the Rotary Encoder functionality
{
	uint8_t FlagRotatingCW :1;  //
	uint8_t FlagRotatingCCW :1;  //

	float (*CountRotation)(void);  //
} as5601_t;

as5601_t* as5601_ctor(void);	// the AS5601 constructor

#endif

#endif /* CORE_INC_MJ514_AS5601_H_ */
