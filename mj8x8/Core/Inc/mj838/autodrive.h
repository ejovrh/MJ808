#ifndef CORE_INC_MJ838_AUTODRIVE_H_
#define CORE_INC_MJ838_AUTODRIVE_H_

#include "main.h"

#if defined(MJ838_)	// if this particular device is active

typedef struct	// struct describing the AutoDrive functionality
{
//	uint8_t FlagLightisOn;  // flag if AutoDrive turned Light on

	float (*GetSpeed_mps)(void);	// get speed in meters per second
	float (*GetSpeed_kph)(void);	// get speed in kilometres per hour
	float (*GetDistance_m)(void);  // get distance in meters
	void (*Do)(void);  // AutoDrive functionality

	void (*LightOff)(void);  // turns off lights on standstill situation
} autodrive_t;

autodrive_t* autodrive_ctor(void);	// the AutoDrive constructor

#endif

#endif /* CORE_INC_MJ838_AUTODRIVE_H_ */
