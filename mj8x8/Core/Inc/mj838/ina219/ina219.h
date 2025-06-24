#ifndef CORE_INC_MJ838_INA219_INA219_H_
#define CORE_INC_MJ838_INA219_INA219_H_

#include "main.h"

#if USE_INA219	// if this particular device is active

typedef struct	// struct describing the INA219 functionality
{
	uint16_t *Voltage;  // pointer to voltage array
	int16_t *Current;  // pointer to current array
	uint16_t *Power;  // pointer to power array
	void (*Measure)(void);  // measure voltage, current and power
} ina219_t;

ina219_t* ina219_ctor(void);	// the INA219 constructor

#endif

#endif /* CORE_INC_MJ838_INA219_INA219_H_ */
