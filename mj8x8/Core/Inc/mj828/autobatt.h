#ifndef CORE_INC_MJ828_AUTOBATT_H_
#define CORE_INC_MJ828_AUTOBATT_H_

#include "main.h"

#define BATTERY_FULL 8400	// mV, 2SxP pack, LiIon cells
#define BBATTERY_TURN_ON_THRESHOLD 5800	// do not turn on below that voltage (approx. 10% charge)
#define BATTERY_EMPTY	5500	// mV, 2SxP pack, LiIon cells

#if defined(MJ828_)	// if this particular device is active

typedef struct	// struct describing the AutoBatt functionality
{
	uint8_t FlagBatteryisCritical :1;  // flag if AutoBatt discovered the battery voltage as critical

	void (*Do)(void);  // AutoBatt functionality
	void (*DisplayBatteryVoltage)(void);  // displays battery voltage
} autobatt_t;

autobatt_t* autobatt_ctor(void);	// the AutoBatt constructor

#endif

#endif /* CORE_INC_MJ828_AUTOBATT_H_ */
