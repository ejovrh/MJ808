#ifndef CORE_INC_MJ514_GEAR_H_
#define CORE_INC_MJ514_GEAR_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

typedef struct	// struct describing the Gear Shifter functionality
{
	uint8_t (*GetGear)(void);  // returns current Rohloff gear
	void (*ShiftGear)(const int8_t n);  // shifts Rohloff to gear n
} gear_t;

gear_t* gear_ctor(void);	// the Gear constructor

#endif

#endif /* CORE_INC_MJ514_GEAR_H_ */
