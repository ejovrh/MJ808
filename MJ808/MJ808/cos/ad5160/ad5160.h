#ifndef AD5160_H_
#define AD5160_H_

typedef struct ad5160_t													// mcp23s08_t struct describing the port expander as a whole
{

} ad5160_t __attribute__((aligned(8)));

ad5160_t *ad5160_ctor();												// mcp23s08_t object constructor - does function pointer & hardware initialization

#endif /* AD5160_H_ */