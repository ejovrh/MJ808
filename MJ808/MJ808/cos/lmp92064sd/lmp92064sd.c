//#include <avr/interrupt.h>
//#include <avr/sleep.h>
//#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/pgmspace.h>

#include "gpio.h"
//#include "uci_spi.h"
#include "cos\lmp92064sd\lmp92064sd.h"

#define	SPI_SS_LMP92064SD_pin	D,	6,	6								// LMP92064SD Voltage/Current meter Slave Select, Buck-Boost-out (pre-load)

typedef struct															// lmp92064sd_t actual
{
	lmp92064sd_t public;												// public struct

} __lmp92064sd_t;

extern __lmp92064sd_t __MCP23S08;										// declare lmp92064sd actual