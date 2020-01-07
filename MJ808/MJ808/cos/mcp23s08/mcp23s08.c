#include <util/delay.h>

#include "uci_spi.h"
#include "cos\mcp23s08\mcp23s08.h"

void _SendCommand(const uint8_t in_command, const uint8_t in_val)		// sends command and value to MCP23S08
{
	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(in_command);										// send the register address byte
	spi_uci_transfer(in_val);											// send the value byte
	_delay_us(1);
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave
};

uint8_t *_GetRegisters(void)											// downloads device configuration registers into __MCP23S08.Registers[]
{

	uint8_t i = 0;

	gpio_clr(SPI_SS_MCP23S08_pin);										// select the slave
	spi_uci_transfer(MCP23S08_IODIR);									// start with the first configuration register

	for (i=0; i < MCP23S08_REGISTER_COUNT; ++i)							// loop over all the MCP23S08's registers
	{																	//	prerequisite is sequential operation mode bit enabled
		MCP23S08.Register[i] = spi_uci_transfer(0xFF);					// get each register and store in struct
		_delay_us(1);
	}
	gpio_set(SPI_SS_MCP23S08_pin);										// de-select the slave

	return MCP23S08.Register;											// returns pointer to device register array in struct for further processing by client code
};

mcp23s08_t MCP23S08 =													// instantiation/initialization of object, saves us the constructor
{
	.SendCommand = &_SendCommand,										// set function pointer
	.GetRegisters = &_GetRegisters										//	ditto
};