//#include <avr/interrupt.h>
//#include <avr/sleep.h>
#include <avr/io.h>
#include <util/delay.h>
//#include <avr/pgmspace.h>

#include "gpio.h"
//#include "uci_spi.h"
#include "cos\lmp92064sd\lmp92064sd.h"

#define	SPI_SS_LMP92064SD_pin	D,	6,	6								// LMP92064SD Voltage/Current meter Slave Select, Buck-Boost-out (pre-load)

typedef struct															// lmp92064sd_t actual
{
	lmp92064sd_t public;												// public struct

	uint8_t	__data_out[4];												// read 4 bytes from 0x0203 down
} __lmp92064sd_t;

static void __read_values(uint8_t *data)
{
	uint8_t i;

	gpio_clr(SPI_SS_LMP92064SD_pin);									// select the slave
	for (i = 0; i<4; ++i)												// while the SS is held, the address is auto-decremented, thus multiple bytes can be read
	{
		*(data+i) = spi_uci_transfer(0x0203-i);							// get the result
		_delay_us(1);													// delay a little bit for the transfer to complete
	}
	gpio_set(SPI_SS_LMP92064SD_pin);									// de-select the slave
};


lmp92064sd_t * lmp92064sd_ctor()
{
	;
}

extern __lmp92064sd_t __MCP23S08;										// declare lmp92064sd actual