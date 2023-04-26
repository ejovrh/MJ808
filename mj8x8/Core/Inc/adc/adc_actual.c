#ifndef MJ828_ADC_ACTUAL_C_
#define MJ828_ADC_ACTUAL_C_

#include <inttypes.h>

#include "adc.h"

typedef struct	// adc_t actual
{
	adc_t public;  // public struct

	volatile uint32_t *__buffer;  // private - pointer to array of ADC readout destinations
} __adc_t;

#endif /* MJ828_ADC_ACTUAL_C_ */
