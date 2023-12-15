#ifndef CORE_INC_ADC_ADC_H_
#define CORE_INC_ADC_ADC_H_

typedef struct adc_t	// struct describing the ADC functionality
{
	void (*Start)(void);	// starts the ADC peripheral
	void (*Stop)(void);  // stops the ADC peripheral
	uint16_t (*GetChannel)(const uint8_t);  // returns channel reading at index
} adc_t;

#endif /* CORE_INC_ADC_ADC_H_ */
