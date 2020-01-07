#include <util/delay.h>

#include "uci_spi.h"
#include "cos\lmp92064sd\lmp92064sd.h"

static void _DownloadData(uint8_t * const data_array)					// downloads voltage / current measurement from device into data_array
{
	uint8_t i;

	gpio_clr(SPI_SS_LMP92064SD_pin);									// select the slave
	for (i = 0; i<4; ++i)												// while the SS is held, the address is auto-decremented, thus multiple bytes can be read
	{
		*(data_array+i) = spi_uci_transfer(0x0203-i);					// write results into data array
		_delay_us(1);													// delay a little bit for the transfer to complete
	}
	gpio_set(SPI_SS_LMP92064SD_pin);									// de-select the slave
};

lmp92064sd_t LMP92064SD =												// instantiation/initialization of object, saves us the constructor
{
	.DownloadData = &_DownloadData										// set function pointer
};										