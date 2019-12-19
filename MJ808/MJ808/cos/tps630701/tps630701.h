#ifndef TPS630701_H_
#define TPS630701_H_

#include <inttypes.h>

typedef struct															// struct describing the Buck-Boost controller object
{
	uint8_t * const PWM;												// direct access to OCR SFR
	void (* const GetValues)(uint8_t *data_array);						// download current and voltage measurement into external container
} tps630701_t __attribute__((aligned(8)));

#endif /* TPS630701_H_ */