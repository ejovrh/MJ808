#ifndef CORE_INC_MJ838_PAC1952_PAC1952_H_
#define CORE_INC_MJ838_PAC1952_PAC1952_H_

#include "main.h"

#if USE_PAC1952	// if this particular device is active

typedef struct	// struct describing the PAC1952 functionality
{
	float *Voltage;	// pointer to voltage array
	float *Current;  // pointer to current array
	float *Power;  // pointer to power array
	void (*Measure)(const uint8_t i);  // refresh_v
	void (*PowerState)(const uint8_t state);  // power device on & initialize
} pac1952_t;

pac1952_t* pac1952_ctor(void);	// the PAC1952 constructor

#endif

#endif /* CORE_INC_MJ838_PAC1952_PAC1952_H_ */
