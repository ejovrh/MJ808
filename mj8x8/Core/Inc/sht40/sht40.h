#ifndef CORE_INC_SHT40_SHT40_H_
#define CORE_INC_SHT40_SHT40_H_

#include "main.h"

#if USE_SHT40	// if this particular device is active

typedef struct	// struct describing the SHT40 functionality
{
	int8_t Temp;	// temperature in integer degrees Celsius
	int8_t RH;	// relative humidity in integer percent

	void (*Measure)(void);  // reads the sensor & computes temperature and humidity
} sht40_t;

sht40_t* sht40_ctor(void);	// the SHT40 constructor

#endif

#endif /* CORE_INC_SHT40_SHT40_H_ */
