#ifndef CORE_INC_ADC_ADC_ACTUAL_C_
#define CORE_INC_ADC_ADC_ACTUAL_C_

#include <inttypes.h>

#include "adc.h"

typedef struct	// adc_t actual
{
	adc_t public;  // public struct
	void (*_Do)(void);	// DMA ISR executed function for ADC computation tasks
	volatile uint32_t *__adc_results;  // private - pointer to array of ADC readout destinations
} __adc_t;

#endif /* CORE_INC_ADC_ADC_ACTUAL_C_ */
