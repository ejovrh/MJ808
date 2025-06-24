#ifndef CORE_INC_ZEROCROSS_ZEROCROSS_H_
#define CORE_INC_ZEROCROSS_ZEROCROSS_H_

typedef struct zerocross_t	// struct describing the zero cross detector functionality
{
	uint32_t ZeroCrossFrequency;  // zero-cross frequency [mHz]
#if USE_RATE_CALC
	int32_t ZeroCrossFrequencyRate;  // dynamo AC frequency change rate - df/dt [mHz/s]
#endif

	void (*Do)(void);  // periodic execution
	void (*Start)(void);	// starts the zero-cross functionality (timer2 & DMA peripherals)
	void (*Stop)(void);  // stops the zero-cross functionality (timer2 & DMA peripherals)
} zerocross_t;

#endif /* CORE_INC_ZEROCROSS_ZEROCROSS_H_ */
