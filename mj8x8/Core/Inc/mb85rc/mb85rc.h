#ifndef CORE_INC_MB85RC_H_
#define CORE_INC_MB85RC_H_

#include "main.h"

#if USE_FERAM	// if this particular device is active

typedef struct	// struct describing the FeRAM functionality
{
	uint32_t (*Read)(const uint16_t RegAddr, const uint8_t size);  // reads one byte of data out of the FeRAM module from given address
	void (*Write)(const uint32_t data, const uint16_t RegAddr, const uint8_t size);  // writes one byte of data into the FeRAM module at given address
} mb85rc_t;

mb85rc_t* mb85rc_ctor(void);	// the MB85RC constructor

#endif

#endif /* CORE_INC_MB85RC_H_ */
