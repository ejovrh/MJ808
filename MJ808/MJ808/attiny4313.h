#ifndef ATTINY4313_H_
#define ATTINY4313_H_

typedef struct
{
	//volatile uint8_t *wdtcr;

} attiny4313_t;

volatile attiny4313_t *attiny_ctor(volatile attiny4313_t * self);

extern volatile attiny4313_t MCU;										// declare MCU object

#endif /* ATTINY4313_H_ */