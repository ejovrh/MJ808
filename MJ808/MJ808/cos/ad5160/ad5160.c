#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "uci_spi.h"
#include "cos\ad5160\ad5160.h"

#define	SPI_SS_AD5160_pin		D,	5,	5								// AD5160 Port Expander Slave Select

static ad5160_t __AD5160 __attribute__ ((section (".data")));			// declare ad5160_t actual and put into .data

static void __SetWiper(const uint8_t in_val)							// sets resistor value by sending a 1-byte value; see datasheet pp. 13; 1 LSB = 60R
{
	gpio_clr(SPI_SS_AD5160_pin);										// select the slave
	spi_uci_transfer(in_val);											// send the byte
	_delay_us(1);
	gpio_set(SPI_SS_AD5160_pin);										// de-select the slave
};

ad5160_t *ad5160_ctor()													// initialize ad5160_t actual and set function pointers
{
	__AD5160.SetWiper = &__SetWiper;									// set up function pointer for public methods

	return &__AD5160;													// return address of public part
};