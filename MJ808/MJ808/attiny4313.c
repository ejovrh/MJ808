#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "attiny4313.h"

volatile ATtiny4313_t *attiny_ctor(volatile ATtiny4313_t * self)
{
	cli();																// clear interrupts globally

	//self->wdtcr = &WDTCR;												// set proper value

	PRR = _BV(PRUSART);													// turn off USART, we don't need it
	ACSR = _BV(ACD);													// turn off the analog comparator, we don't need it either

	// setup of INT1  - handled via INT1_vect ISR
	MCUCR = _BV(ISC11);													// a falling edge generates an IRQ
	GIMSK = ( _BV(INT1)	|												// enable INT1
			  _BV(PCIE2));												// enable pin change IRQ for PCTIN17-11 (further specified in PCMSK2)

	WDTCR |= (_BV(WDCE) | _BV(WDE));									// WDT change enable sequence
	WDTCR = ( _BV(WDIE) | _BV(WDP2)  );									// watchdog timer set to 0.25s

	sei();																// enable interrupts globally

	return self;
};

volatile ATtiny4313_t MCU __attribute__ ((section (".data")));			// define MCU object and put it into .data