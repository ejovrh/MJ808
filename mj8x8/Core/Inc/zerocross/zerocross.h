#ifndef CORE_INC_ZEROCROSS_ZEROCROSS_H_
#define CORE_INC_ZEROCROSS_ZEROCROSS_H_

typedef struct zerocross_t	// struct describing the zero cross detector functionality
{
	float ZeroCrossFrequency;  // zero-cross frequency
	float ZeroCrossFrequencyRate;  // dynamo AC frequency change rate - df/dt [Hz/s]

	void (*Do)(void);  // periodic execution
	void (*Start)(void);	// starts the zero-cross functionality (timer2 & DMA peripherals)
	void (*Stop)(void);  // stops the zero-cross functionality (timer2 & DMA peripherals)
} zerocross_t;

#endif /* CORE_INC_ZEROCROSS_ZEROCROSS_H_ */
