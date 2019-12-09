//#include <avr/interrupt.h>
//#include <avr/sleep.h>
//#include <avr/io.h>
//#include <util/delay.h>
//#include <avr/pgmspace.h>

#include "gpio.h"
//#include "uci_spi.h"
#include "cos\mcp23s08\mcp23s08.h"

#define	SPI_SS_MCP23S08_pin		D,	1,	1								// MCP23S08 Port Expander Slave Select

typedef struct															// mcp23s08_t actual
{
	mcp23s08_t public;													// public struct

} __mcp23s08_t;


mcp23s08_t * mcp23s08_ctor()
{
	;
}

extern __mcp23s08_t __MCP23S08;											// declare mcp23s08_t actual