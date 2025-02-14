#ifndef CORE_INC_MJ838_TLC59208_TLC59208_H_
#define CORE_INC_MJ838_TLC59208_TLC59208_H_

#include "main.h"

#if USE_TLC59208	// if this particular device is active

typedef struct	//  struct describing the LED driver functionality
{
	void (*Write)(const uint8_t RegAddr, const uint16_t *data);  // writes 2 bytes of data to address n and n+1 using auto-increment
	void (*Power)(uint8_t val);  // set LED Driver GPIO to val
} tlc59208_t;

tlc59208_t* tlc59208_ctor(void);	// the TLC59208 constructor

#endif

#endif /* CORE_INC_MJ838_TLC59208_TLC59208_H_ */
