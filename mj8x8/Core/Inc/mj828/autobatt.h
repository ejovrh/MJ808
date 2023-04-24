#ifndef CORE_INC_MJ828_AUTOBATT_H_
#define CORE_INC_MJ828_AUTOBATT_H_

#include "main.h"

// TODO - specify Li-Ion cell specific values
#define BATTERY_FULL 8400	// mV, 2SxP pack, LiIon cells
#define BATTERY_EMPTY	5500	// mV, 2SxP pack, LiIon cells
#define ADC_MEASURE_ITERATIONS 40	// iterations for measurement data value average

#if defined(MJ828_)	// if this particular device is active

typedef struct	// struct describing the AutoBatt functionality
{
	uint16_t Vbat;	// battery voltage in mV
	int16_t Temp;  // temperature

	void (*Do)(void);  // AutoBatt functionality
	void (*DisplayBatteryVoltage)(void);  // displays battery voltage
} autobatt_t;

autobatt_t* autobatt_ctor(void);	// the AutoBatt constructor

#endif

#endif /* CORE_INC_MJ828_AUTOBATT_H_ */
