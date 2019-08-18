#ifndef ATTINY4313_H_
#define ATTINY4313_H_

typedef struct
{
	//volatile uint8_t *wdtcr;

} ATtiny4313_t;

volatile ATtiny4313_t *attiny_ctor(volatile ATtiny4313_t * const self);

extern volatile ATtiny4313_t MCU;										// declare MCU object

#endif /* ATTINY4313_H_ */