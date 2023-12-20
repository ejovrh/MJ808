#ifndef CORE_INC_MJ838_AUTOCHARGE_H_
#define CORE_INC_MJ838_AUTOCHARGE_H_

#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#define LOAD_CONNECT_THRESHOLD_SPEED_LOW 2	// speed threshold in m/s for load disconnect
#define LOAD_CONNECT_THRESHOLD_SPEED_HIGH 3	// speed threshold in m/s for load connect

typedef struct	// struct describing the AutoDrive functionality
{
	uint8_t FlagLoadisConnected :1;  // flag indicating if the load is connected, i.e. high-side load switch is on or off

	void (*Do)(void);  // AutoCharge functionality
} autocharge_t;

autocharge_t* autocharge_ctor(void);	// the AutoDrive constructor

#endif

#endif /* CORE_INC_MJ838_AUTOCHARGE_H_ */
