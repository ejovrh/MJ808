#ifndef CORE_INC_ZEROCROSS_ZEROCROSS_H_
#define CORE_INC_ZEROCROSS_ZEROCROSS_H_

typedef struct zerocross_t	// struct describing the event handler functionality
{
	void (*Start)(void);	// starts the zero-cross functionality (timer2 & DMA peripherals)
	void (*Stop)(void);  // stops the zero-cross functionality (timer2 & DMA peripherals)
	uint16_t (*GetWheelFrequency)(const uint8_t);  // returns detected wheel frequency
} zerocross_t;

#endif /* CORE_INC_ZEROCROSS_ZEROCROSS_H_ */
