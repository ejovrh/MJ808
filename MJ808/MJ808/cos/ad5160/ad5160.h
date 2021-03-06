#ifndef AD5160_H_
#define AD5160_H_

#include <inttypes.h>

typedef struct ad5160_t													// mcp23s08_t actual struct describing the port expander as a whole
{
	void (* SetWiper)(const uint8_t in_val);							// sets resistor wiper to numerical value, 60R per LSB
} ad5160_t __attribute__((aligned(8)));

ad5160_t *ad5160_ctor();												// initialize ad5160_t actual and set function pointers;

#endif /* AD5160_H_ */