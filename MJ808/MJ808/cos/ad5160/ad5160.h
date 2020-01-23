#ifndef AD5160_H_
#define AD5160_H_

#include <avr/io.h>

#include <inttypes.h>

#include "cos\cos.h"													// caller header file, contains pin definitions for SPI

typedef struct ad5160_t													// ad5160_t actual struct describing the potentiometer as a whole
{
	void (* const SetWiper)(const uint8_t in_val);						// sets resistor wiper to numerical value, 60R per LSB
} ad5160_t __attribute__((aligned(8)));

extern ad5160_t AD5160;													// forward declare public struct

#endif /* AD5160_H_ */