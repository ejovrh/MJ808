#ifndef CORE_INC_MJ838_INA219_INA219_H_
#define CORE_INC_MJ838_INA219_INA219_H_

#include "main.h"

#if USE_INA219	// if this particular device is active

typedef struct	// struct describing the INA219 functionality
{
	uint32_t *Voltage;  // pointer to voltage value
	int32_t *Current;  // pointer to current value
	uint32_t *Power;  // pointer to power value
	void (*Measure)(void);  // measure voltage, current and power
} ina219_t;

ina219_t* ina219_ctor(void);	// the INA219 constructor

#endif

#endif /* CORE_INC_MJ838_INA219_INA219_H_ */
