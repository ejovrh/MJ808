//#include <avr/interrupt.h>
//#include <avr/sleep.h>
//#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/pgmspace.h>

#include "gpio.h"
//#include "uci_spi.h"
#include "cos\ad5160\ad5160.h"

#define	SPI_SS_AD5160_pin		D,	5,	5								// AD5160 Poet Expander Slave Select

typedef struct															// ad5160_t actual
{
	ad5160_t public;													// public struct

} __ad5160_t;

extern __ad5160_t __AD5160;												// declare ad5160_t actual