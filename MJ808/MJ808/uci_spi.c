#include <avr/io.h>
#include "uci_spi.h"

/* now, how does the bloody thing work ?!?
 * see datasheet section 16.3.1 for an explanation and flowchart
 * it is a scam. this is almost SW SPI - aka. bit banging
 * spi_via_usi_driver.c (AVR319) uses timers for intermediate SCK speeds; this function goes to the max. the CPU can deliver
 *
 * anyway....
 * see: https://github.com/tessel/avr-usi-spi/blob/master/spi_via_usi_driver.c
 * http://srcpro.pl/more.php?lang=ENG&pg=1&article=13
 * http://www.avrfreaks.net/forum/how-attiny2313-usi-spi-slavemaster-c-code
 * http://www.avrfreaks.net/forum/attiny2313-spi-master
 *

 * NOTE:
 *	MCP2515
 *		SPI 0 and 3
 *		SCK <10MHz
 *		MSB first
 */

// sends (and receives) data from the SPI bus
// slave select must happen outside of this function
uint8_t spi_uci_transfer(const uint8_t data)
{
	USIDR = data;									// put the payload into the USI data register

	USISR |= 1 << USIOIF;					// counter overflow interrupt, indicates counter overflow

	while ((USISR & (1 << USIOIF)) == 0 ) // this loop provides the software defined clock; this all is SW SPI...
	{
		USICR = ( _BV(USIWM0)|	// select three wire mode; USIWM1 is 0 - the inital value
							_BV(USICS1)|	// SPI mode: USICS1 == 1 && USICS0 == 0 <=> SPI mode 3
							_BV(USICLK)|	// clock strobe
							_BV(USITC));		// toggles the clock
	}	// by clocking the USIDR is shifted into the slave (and data received)

	return USIDR;									// hopefully have something useful here...
}