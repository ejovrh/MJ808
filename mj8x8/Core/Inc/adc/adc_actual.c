#ifndef MJ828_ADC_ACTUAL_C_
#define MJ828_ADC_ACTUAL_C_

#include <inttypes.h>

#include "adc.h"

typedef struct	// adc_t actual
{
	adc_t public;  // public struct

	uint16_t *__channel;  // private - pointer to array of ADC readout destinations
	uint8_t __index;	// private - auto-incrementing index for reading consecutive channels
} __adc_t;

#endif /* MJ828_ADC_ACTUAL_C_ */
