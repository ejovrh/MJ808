#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "uci_spi.h"
#include "cos\lmp92064sd\lmp92064sd.h"

#define	SPI_SS_LMP92064SD_pin	D,	6,	6								// LMP92064SD Voltage/Current meter Slave Select, Buck-Boost-out (pre-load)

static lmp92064sd_t __LMP92064SD __attribute__ ((section (".data")));	// declare lmp92064sd_t actual and put into .data

static void __DownloadData(uint8_t *data_array)							// downloads voltage / current measurement from device
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

lmp92064sd_t *lmp92064sd_ctor()											// initialize lmp92064sd_t object and set function pointers
{
	__LMP92064SD.DownloadData = &__DownloadData;						// set function pointer

	return &__LMP92064SD;												// return address of public part
};										