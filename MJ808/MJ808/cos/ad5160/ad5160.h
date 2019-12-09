#ifndef AD5160_H_
#define AD5160_H_

typedef struct ad5160_t													// mcp23s08_t struct describing the port expander as a whole
{
	void (* SetWiper)(const uint8_t in_val);							// sets resistor wiper to numerical value, 60R per LSB
} ad5160_t __attribute__((aligned(8)));

#endif /* AD5160_H_ */