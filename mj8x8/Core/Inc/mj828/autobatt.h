#ifndef CORE_INC_MJ828_AUTOBATT_H_
#define CORE_INC_MJ828_AUTOBATT_H_

#include "main.h"

#if defined(MJ828_)	// if this particular device is active

typedef struct	// struct describing the AutoBatt functionality
{
	uint8_t AutoBattEnabled;  // flag indicating AutoBatt is active
	uint8_t AutoBattisOn;  // flag if AutoBatt is on

	void (*Do)(void);  // AutoBatt functionality
	void (*DisplayBatteryVoltage)(void);  // displays battery voltage
} autobatt_t;

autobatt_t* autobatt_ctor(void);	// the AutoBatt constructor

#endif

#endif /* CORE_INC_MJ828_AUTOBATT_H_ */
