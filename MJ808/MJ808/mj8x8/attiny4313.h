#ifndef ATTINY4313_H_
#define ATTINY4313_H_

typedef struct
{
	//volatile uint8_t wdtcr;
} ATtiny4313_t;

ATtiny4313_t *attiny_ctor();											// declare constructor for concrete class, does hardware initialization

#endif /* ATTINY4313_H_ */