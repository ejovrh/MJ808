#ifndef CORE_INC_ZEROCROSS_ZEROCROSS_ACTUAL_C_
#define CORE_INC_ZEROCROSS_ZEROCROSS_ACTUAL_C_

#include <inttypes.h>

#include "zerocross.h"

typedef struct	// zerocross_t actual
{
	zerocross_t public;  // public struct

	float _ZeroCrossFrequency;  // dynamo AC frequency, as measured by zero-cross detection
	float _ZeroCrossFrequencyRate;  // dynamo AC frequency change rate - df/dt [Hz/s]
} __zerocross_t;

#endif /* CORE_INC_ZEROCROSS_ZEROCROSS_ACTUAL_C_ */
