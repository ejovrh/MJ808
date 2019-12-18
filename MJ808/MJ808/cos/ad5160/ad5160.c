#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "uci_spi.h"
#include "cos\ad5160\ad5160.h"

#define	SPI_SS_AD5160_pin		D,	5,	5								// AD5160 Port Expander Slave Select

static void __set_wiper(const uint8_t in_val)							// sets resistor value by sending a 1-byte value; see datasheet pp. 13; 1 LSB = 60R
{
	gpio_clr(SPI_SS_AD5160_pin);										// select the slave
	spi_uci_transfer(in_val);											// send the byte
	_delay_us(1);
	gpio_set(SPI_SS_AD5160_pin);										// de-select the slave
};

ad5160_t __AD5160 =														// instantiate ad5160_t actual and set function pointers
{
	.SetWiper = &__set_wiper											// set up function pointer for public methods
};