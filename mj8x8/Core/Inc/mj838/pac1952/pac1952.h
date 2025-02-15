#ifndef CORE_INC_MJ838_PAC1952_PAC1952_H_
#define CORE_INC_MJ838_PAC1952_PAC1952_H_

#include "main.h"

#if USE_PAC1952	// if this particular device is active

typedef struct	// struct describing the PAC1952 functionality
{
	float (*GetVbus)(const uint8_t channel);  // get bus voltage for channel x
	float (*GetVsense)(const uint8_t channel);  // get sense voltage for channel x
	float (*GetVpower)(const uint8_t channel);  // get Vsense x Vbus for channel x
	void (*Measure)(void);  // refresh_v
	void (*Power)(const uint8_t state);  // power device on & initialize
} pac1952_t;

pac1952_t* pac1952_ctor(void);	// the PAC1952 constructor

#endif

#endif /* CORE_INC_MJ838_PAC1952_PAC1952_H_ */
