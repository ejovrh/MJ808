#ifndef ATTINY4313_H_
#define ATTINY4313_H_

typedef struct
{
	//volatile uint8_t *wdtcr;

} attiny4313_t;

volatile attiny4313_t *attiny_ctor(volatile attiny4313_t * self);

#endif /* ATTINY4313_H_ */