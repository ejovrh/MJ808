#include <avr/io.h>
#include <avr/interrupt.h>




/* now, how does the bloody thing work ?!?
 * see datasheet section 16.3.1 for an explanation and flowchart
 * it is a scam. this is almost SW SPI - aka. bit banging
 * spi_via_usi_driver.c (AVR319) uses timers for intermediate SCK speeds; this function goes to the max. the CPU can deliver
 *
 * anyway....
 * see: https://github.com/tessel/avr-usi-spi/blob/master/spi_via_usi_driver.c
 * http://srcpro.pl/more.php?lang=ENG&pg=1&article=13
 * http://www.avrfreaks.net/forum/how-attiny2313-usi-spi-slavemaster-c-code
 * http://www.avrfreaks.net/forum/attiny2313-spi-master - that last one is especially interesting.
 *
 * observe DI and DO pins - they are not unnecessarily the same as MISO/MOSI.
 *	e.g. PB6 is labeled as MISO but is in reality DO
 *
 */

/*
 * SPI modes
 *	read: http://dlnware.com/theory/SPI-Transfer-Modes
 * mode 0:
 *		CPOL 0 - clock idle is low
 *		CPHA 0 - sample on the leading (first) edge of clock signal
 *
 * mode 3:
 *		CPOL 1 - clock idle is high
 *		CPHA 1 - sample on the trailing (second) edge of the clock signal
 *
 */

// sends (and receives) data from the SPI bus
// slave select must happen outside of this function
static uint8_t spi_uci_transfer(const uint8_t data)
{
	cli();									// lets call it paranoia but i want the SPI transaction to be atomic - i.e. no IRQ shall be able to abort it
	USIDR = data;							// put the payload into the USI data register

	USISR = _BV(USIOIF);					// counter overflow interrupt, indicates counter overflow

	do
	{
		USICR = ( _BV(USIWM0)				// select three wire mode
						 |_BV(USICS1)		// positive edge - CPHA0
						 |_BV(USICLK)		// clock strobe
						 |_BV(USITC));		// generates the clock by toggling the SCK pin
	}										// due to clocking the USIDR is shifted into the slave (and data received)
	while ((USISR & _BV( USIOIF) ) == 0 );	// this loop provides the software defined clock; this all is SW SPI...

	sei();
	return USIDR;							// hopefully have something useful here...
};

i_wire_t Wire __attribute__ ((section (".data"))) = {.Transmit = spi_uci_transfer} ;				//
