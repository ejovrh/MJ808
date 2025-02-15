#ifndef CORE_INC_MJ838_USE_DAC121C081_USE_DAC121C081_H_
#define CORE_INC_MJ838_USE_DAC121C081_USE_DAC121C081_H_

#include "main.h"

#if USE_DAC121C081	// if this particular device is active

typedef struct	// struct describing the DAC121C081 functionality
{
	uint16_t (*Read)(void);  // reads 2 bytes from DAC register
	void (*Write)(const uint16_t *data);  // writes 2 bytes to DAC register
	void (*PowerOff)(void);  // power off & activate 100k pulldown
} dac121c081_t;

dac121c081_t* dac121c081_ctor(void);	// the DAC121C081 constructor

#endif

#endif /* CORE_INC_MJ838_USE_DAC121C081_USE_DAC121C081_H_ */
