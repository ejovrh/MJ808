#include <avr/io.h>
#include <util/delay.h>

#include "uci_spi.h"
#include "cos\ad5160\ad5160.h"

static void _SetWiper(const uint8_t in_val)								// sets resistor value by sending a 1-byte value; see datasheet pp. 13; 1 LSB = 60R
{
	gpio_clr(SPI_SS_AD5160_pin);										// select the slave
	spi_uci_transfer(in_val);											// send the byte
	_delay_us(1);
	gpio_set(SPI_SS_AD5160_pin);										// de-select the slave
};

ad5160_t AD5160 =														// instantiation/initialization of object, saves us the constructor
{
	.SetWiper = &_SetWiper												// set function pointer
};