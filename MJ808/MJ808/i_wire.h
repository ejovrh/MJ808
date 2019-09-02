#ifndef WIRE_H_
#define WIRE_H_

typedef struct
{
	uint8_t (*Transmit)(const uint8_t val);
} i_wire_t;

i_wire_t Wire;

#endif /* WIRE_H_ */