#ifndef CORE_INC_MJ514_MB85RC_H_
#define CORE_INC_MJ514_MB85RC_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

typedef struct	// struct describing the FeRAM functionality
{
	uint8_t (*Read)(const uint8_t addr);  //
	void (*Write)(const uint8_t addr, const uint8_t data);	//
} mb85rc_t;

mb85rc_t* mb85rc_ctor(void);	// the MB85RC constructor

#endif

#endif /* CORE_INC_MJ514_MB85RC_H_ */
