#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "uci_spi.h"
#include "cos\mcp23s08\mcp23s08.h"

#define	SPI_SS_MCP23S08_pin		D,	1,	1								// MCP23S08 Port Expander Slave Select

static mcp23s08_t __MCP23S08 __attribute__ ((section (".data")));		// declare mcp23s08_t actual and put into .data

void _SetFoo(const uint8_t in_val)										// uploads some data
{
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_IODIR);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

uint8_t _GetBar(uint8_t in_val)											// downloads some data
{
	uint8_t retval;

	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_IPOL);									// send the command byte
	retval = spi_uci_transfer(in_val);									// fetch the value
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave

	return retval;
};

mcp23s08_t *mcp23s08_ctor()												// initialize mcp23s08_t actual and set function pointers
{
	// TODO: implement port expander low level functions
	__MCP23S08.SetFoo = &_SetFoo;										// uploads some data
	__MCP23S08.GetBar = &_GetBar;										// downloads some data

	return &__MCP23S08;													// return address of public part
};