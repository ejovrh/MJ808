#ifndef ADC_ACTUAL_C_
#define ADC_ACTUAL_C_

#include <inttypes.h>

#include "zerocross.h"

typedef struct	// zerocross_t actual
{
	zerocross_t public;  // public struct

	volatile float _ZeroCrossFrequency;  // dynamo AC frequency, as measured by zero-cross detection
} __zerocross_t;

#endif /* ADC_ACTUAL_C_ */
