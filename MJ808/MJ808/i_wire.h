#ifndef WIRE_H_
#define WIRE_H_

typedef struct															// an interface which defines one function, is implemented by a low-level communications driver
{
	uint8_t (*  Transmit)(const uint8_t val);
} i_wire_t;

extern const i_wire_t Wire;

#endif /* WIRE_H_ */