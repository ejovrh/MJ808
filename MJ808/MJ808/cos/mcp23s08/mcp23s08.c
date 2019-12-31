#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "uci_spi.h"
#include "cos\mcp23s08\mcp23s08.h"

#define	SPI_SS_MCP23S08_pin		D,	1,	1								// MCP23S08 Port Expander Slave Select

static mcp23s08_t __MCP23S08 __attribute__ ((section (".data")));		// declare mcp23s08_t actual and put into .data

void _SetIODir(const uint8_t in_val)									// sets I/O direction register: 0 - output, 1 - input
{																		// datasheet p. 10
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_IODIR);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetIPOL(const uint8_t in_val)										// sets input polarity register: 0 - same logic state, 1 - opposite logic state
{																		// datasheet p. 11
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_IPOL);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetGPIntEN(const uint8_t in_val)									// sets interrupt-on-change control register: 0 - disable interrupt, 1 - enable interrupt
{																		// datasheet p. 12
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_GPINTEN);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetDEFVal(const uint8_t in_val)									// sets default compare register for interrupts
{																		// datasheet p. 13
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_DEFVAL);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetIntCon(const uint8_t in_val)									// sets interrupt control register: 0 - pin compared to previous value, 1 - pin compared to defval
{																		// datasheet p. 14
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_INTCON);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetIOCon(const uint8_t in_val)									// sets configuration register
{																		// datasheet p. 15
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_IOCON);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetPullup(const uint8_t in_val)									// sets pull-up configuration register: 0 - pull-up disabled, 1 - pull-up enabled
{																		// datasheet p. 16
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_GPPU);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetIntF(const uint8_t in_val)										// sets interrupt flag register: 0 - interrupt not pending, 1 - pin caused interrupt
{																		// datasheet p. 17
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_INTF);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetIntCap(const uint8_t in_val)									// sets interrupt capture register: 0 - logic low, 1 - logic high
{																		// datasheet p. 18
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_INTCAP);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetPort(const uint8_t in_val)										// sets port (GPIO) register: 0 - logic low, 1 - logic high
{																		// datasheet p. 19
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_GPIO);									// send the command byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

void _SetOLAT(const uint8_t in_val)										// sets output latch register: 0 - logic low, 1 - logic high
{																		// datasheet p. 20
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_OLAT);									// send the command byte
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
	__MCP23S08.SetIODir = &_SetIODir;									// sets I/O direction register: 0 - output, 1 - input
	__MCP23S08.SetIPOL = &_SetIPOL;										// sets input polarity register: 0 - same logic state, 1 - opposite logic state
	__MCP23S08.SetGPIntEN = &_SetGPIntEN;								// sets interrupt-on-change control register: 0 - disable interrupt, 1 - enable interrupt
	__MCP23S08.SetDEFVal = &_SetDEFVal;									// sets default compare register for interrupts
	__MCP23S08.SetIntCon = &_SetIntCon;									// sets interrupt control register: 0 - pin compared to previous value, 1 - pin compared to defval
	__MCP23S08.SetIOCon = &_SetIOCon;									// sets configuration register
	__MCP23S08.SetPullup = &_SetPullup;									// sets pull-up configuration register: 0 - pull-up disabled, 1 - pull-up enabled
	__MCP23S08.SetIntF = &_SetIntF;										// sets interrupt flag register: 0 - interrupt not pending, 1 - pin caused interrupt
	__MCP23S08.SetIntCap = &_SetIntCap;									// sets interrupt capture register: 0 - logic low, 1 - logic high
	__MCP23S08.SetPort = &_SetPort;										// sets port (GPIO) register: 0 - logic low, 1 - logic high
	__MCP23S08.SetOLAT = &_SetOLAT;										// sets output latch register: 0 - logic low, 1 - logic high

	return &__MCP23S08;													// return address of public part
};