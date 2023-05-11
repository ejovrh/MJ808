#ifndef ADC_ACTUAL_C_
#define ADC_ACTUAL_C_

#include <inttypes.h>

#include "zerocross.h"

typedef struct	// zerocross_t actual
{
	zerocross_t public;  // public struct

	volatile float _DynamoFrequency;	// dynamo AC frequency, as measured by zero-cross detection
	volatile float _WheelFrequency;  // wheel rotation frequency
	volatile float _ms;  // speed in m/s
	volatile float _kmh;	// speed in km/h
	volatile uint32_t *__buffer;  // private - pointer to array of timer-counter readout destinations
} __zerocross_t;

#endif /* ADC_ACTUAL_C_ */
