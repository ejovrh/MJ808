#ifndef CORE_INC_MJ828_AUTOLIGHT_H_
#define CORE_INC_MJ828_AUTOLIGHT_H_

#include "main.h"

#if defined(MJ828_)	// if this particular device is active

typedef struct	// struct describing the AutoLight functionality
{
	uint8_t AutoLightisOn;  // flag if AutoLight is on

	void (*Do)(void);  // AutoLight functionality
} autolight_t;

autolight_t* autolight_ctor(void);	// the AutoLight constructor

#endif

#endif /* CORE_INC_MJ828_AUTOLIGHT_H_ */
