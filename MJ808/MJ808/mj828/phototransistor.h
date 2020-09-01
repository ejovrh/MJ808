#ifndef PHOTOTRANSISTOR_H_

#include <inttypes.h>

typedef struct															// struct describing the phototransistor
{
	uint8_t (*GetPinState)(void);										// returns pin state: high - daylight, low - darkness
} phototransistor_t;


extern phototransistor_t * const Phototransistor;						// declare pointer to public struct part

#endif /* PHOTOTRANSISTOR_H_ */