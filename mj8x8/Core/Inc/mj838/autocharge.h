#ifndef CORE_INC_MJ838_AUTOCHARGE_H_
#define CORE_INC_MJ838_AUTOCHARGE_H_

#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#if USE_INA219
#include "ina219/ina219.h"	// power monitor IC
#endif

#define LOAD_CONNECT_THRESHOLD_SPEED_LOW 2	// speed threshold in m/s for load disconnect
#define LOAD_CONNECT_THRESHOLD_SPEED_HIGH 3	// speed threshold in m/s for load connect

typedef struct	// struct describing the AutoDrive functionality
{
#if USE_ADJUSTABLE_LOAD
	uint16_t AdjustableLoadState;  // adjustable load DAC output voltage: 0 off, non-zero: variable
	void (*AdjustLoad)(const uint16_t voltage);  // set DAC output voltage
#endif
#if USE_APPLICATION_LOAD
	uint8_t (*IsAppLoadConnected)(void);  // returns High-Side load switch state: 0 - disconnected, 1 - connected
#endif
#if USE_INA219
	ina219_t *PowerMonitor;  // pointer to INA219 object
#endif
	void (*Do)(void);  // AutoCharge functionality
} autocharge_t;

autocharge_t* autocharge_ctor(void);	// the AutoDrive constructor

#endif

#endif /* CORE_INC_MJ838_AUTOCHARGE_H_ */
