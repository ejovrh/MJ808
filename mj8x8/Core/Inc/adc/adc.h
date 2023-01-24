#ifndef CORE_INC_ADC_ADC_H_
#define CORE_INC_ADC_ADC_H_

typedef struct adc_t	// struct describing the event handler functionality
{
	void (*ReadChannels)(void);  // reads channels from ADC ISR
	uint8_t (*GetVal)(const uint8_t);  // returns channel reading at index
} adc_t;

#endif /* CORE_INC_ADC_ADC_H_ */
