#include <avr/io.h>
#include "gpio.h"
#include "gpio_definitions.h"

#include "uci_spi.h"
#include "mcp2515.h"

static void reset(void); // resets the MCP2515, defined way blow
static void load_tx_buffer(const uint8_t buffer, const uint8_t *data, const uint8_t len); // loads '*data' into TX buffer identified by the bit mask 'buffer'
static void read_rx_buffer(const uint8_t buffer, uint8_t *data); // loads a RX buffer identified by the bit mask 'buffer' into '*data'
static void mcp2515_send_rts(const uint8_t buffer); // sends RTS for 'buffer'
static uint8_t rx_status(void); // returns byte with flags indicating which filter was matched

// initialization & configuration after power on
void mcp2515_init(void)
{
	union uint16_t
	{
		uint16_t val;
		uint8_t bytes[2];
	} word;

	reset(); // reset first, must be held for at least 2us
	mcp2515_bit_modify(CANINTE, ( _BV(RX0IE) | _BV(RX1IE) ), ( _BV(RX0IE) | _BV(RX1IE) )); // enable RX interrupts on INT pin
	mcp2515_bit_modify(TXRTSCTRL, 0xFF, 0x00); // zero out & set B2RTSM, B1RTSM, B0RTSM to digital input (turn off RTS functionality on pins)
	mcp2515_bit_modify(RXB0CTRL, _BV(RXM0), _BV(RXM0)); // receive only standard identifiers for RXB0
	mcp2515_bit_modify(RXB1CTRL, _BV(RXM0), _BV(RXM0)); // receive only standard identifiers for RXB1

	// bit timing settings
	//	see https://www.kvaser.com/support/calculators/bit-timing-calculator/
	//	oscillator frequency: 8MHz, desired bandwidth: 500kbps
	//	datasheet pp. 39
	mcp2515_bit_modify(CNF1, 0xFF, 0x00); // 1 x Tq sampling, 2 x ( 0 + 1) / 8 MHz prescaler
	mcp2515_bit_modify(CNF2, 0xFF, (_BV(SAM) | _BV(PHSEG11) | _BV(PHSEG10)) ); // PS2 > PS1, 3x sampling of bus line, 4 x Tq
	mcp2515_bit_modify(CNF3, 0xFF, (_BV(WAKFIL) | _BV(PHSEG21) | _BV(PHSEG20)) ); // no clockout, wakeup filter enabled, PS2 length set to 4 x Tq

	/*
	 * filters:
	 RXB0 [unicast]
	 1M, 2F
	 - RXM0: 0x1FF - mask unicast to own device class and ID (self)
	 - RXF0: 0x1B0 - front light towards own light ID (rear light)
	 - RXF1: 0x170 - logic unit towards own light ID (rear light)

	 RXB1 [broadcast]
	 1M, 4F
	 - RXM1: 0xFF - mask broadcast towards own device class
	 - RXF2: 0x30 - dynamo towards lights (rear)
	 - RXF3: 0x70 - logic units towards lights (rear)
	 - RXF4: 0xB0 - front light towards lights (rear)
	 - RXF5: 0xF0 - self towards own device class (rear)
	 */

	uint16_t words[32] = {0x1FF,0x1FF};

	word.val = 0x1FF;
	mcp2515_register_write_bytes(RXM0SIDH, word.bytes, 2);

	word.val = 0x1B0;
	mcp2515_register_write_bytes(RXF0SIDH, word.bytes, 2);

	word.val = 0x170;
	mcp2515_register_write_bytes(RXF1SIDH, word.bytes, 2);

	word.val = 0xFF;
	mcp2515_register_write_bytes(RXM1SIDH, word.bytes, 2);

	word.val = 0x30;
	mcp2515_register_write_bytes(RXF2SIDH, word.bytes, 2);

	word.val = 0x70;
	mcp2515_register_write_bytes(RXF3SIDH, word.bytes, 2);

	word.val = 0xB0;
	mcp2515_register_write_bytes(RXF4SIDH, word.bytes, 2);

	word.val = 0xF0;
	mcp2515_register_write_bytes(RXF5SIDH, word.bytes, 2);

	mcp2515_bit_modify(CANCTRL, 0xFF, 0x00); // put into normal mode (no CLKOUT, no ABAT, no OSM
};

// reads one byte at addr and returns it
uint8_t mcp2515_register_read(const uint8_t addr)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_READ); // send read command
	retval =  spi_uci_transfer(addr);	// return result
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval;
};

// writes len bytes of data into addr
// provide not more than 4 bytes of data and len !!!
void mcp2515_register_write_bytes(const uint8_t addr, const uint8_t *data, const uint8_t len)
{
	int i;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_WRITE); // issue the write command
	spi_uci_transfer(addr); // set the address

	for (i=0; i<len; i++) // loop over how many bytes we need to write
		spi_uci_transfer(*(data+i));

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// provide data to MCP2515 and flag for TX over the CAN bus
//	provide not more than 4 bytes of data and len !!!
void mcp2515_can_msg_send(const uint8_t *data, const uint8_t len)
{
	/* mode of operation
	 *	1. find an empty TX buffer
	 *	2.	select the buffer
	 *	3. issue load_tx_buffer for the selected buffer
	 *	4.	load register
	 *	5. if data is to be sent, again send load_tx_buffer (buffer incremented by one)
	 *	6.	load register
	 *	7. determine message priority
	 *	8. load message priority into register
	 *	9. set data length code
	 *	10. set RTS
	 */

	uint8_t buffer;
	uint8_t priority;

	// step 1 & 2
	// FIXME - this is ugly as hell and only a one-time shot; it needs to be looped
	buffer = mcp2515_read_status(); // TXB0CNTRL.TXREQ, TXB1CNTRL.TXREQ, TXB2CNTRL.TXREQ are of interest

	if (buffer & 0x04)
	{
		buffer = 0;
		mcp2515_register_write_bytes(TXB0DLC, &len, 1);
	}

	if (buffer & 0x16)
	{
		buffer = 2;
		mcp2515_register_write_bytes(TXB1DLC, &len, 1);
	}

	if (buffer & 0x64)
	{
		buffer = 4;
		mcp2515_register_write_bytes(TXB2DLC, &len, 1);
	}

	// step 3 and 4
	load_tx_buffer(buffer, data, 2); //	send the load_tx_buffer command for TXBnSIDH and TXBnSIDL, datasheet p. 66, table 12.1

	buffer +=1;
	// step 5 and 6
	load_tx_buffer(buffer, data + sizeof(uint8_t), *(data + 2*sizeof(uint8_t)) ); //	send the load_tx_buffer command for TXBnDm (data - if applicable), datasheet p. 66, table 12.1

	// step 7
	priority = *data; // the identifier high byte has only 3 bits - the leftmost 3...
	priority = (*data >> 6);

	// step 8
	mcp2515_bit_modify(TXB0CTRL, ( _BV(TXP1) | _BV(TXP0) ), priority); // set message priority into TXBnCTRL.TXP1:0

	// step 9
	//TODO - set TXBnDLC.DLC3:0 - data length in bytes

	// step 10
	mcp2515_send_rts(buffer); // send RTS - flag the buffer for TX
	mcp2515_send_rts(buffer-1);
	// check TXBnCTRL.TXREQ for successful TX
};

// fetches a received CAN message from the MCP2515, triggered by RX interrupt
void mcp2515_can_msg_receive(uint8_t *data)
{
	uint8_t buffer = rx_status(); // figure out in which RX buffer the incoming message is stored
	buffer = (buffer >> 5); //  datasheet p.69, table 12.9 and 12.3
	read_rx_buffer(buffer, data );	// pass the buffer and message container further on
};

// sends RTS for 'buffer'
void mcp2515_send_rts(const uint8_t buffer)
{
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_RTS | buffer); // send RTS command & the buffer (bit mask)
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// sets or clears individual bits (defined by byte) via mask at addr
void mcp2515_bit_modify(const uint8_t addr, const uint8_t mask, const uint8_t byte)
{
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_BIT_MODIFY); // send the bit modify command
	spi_uci_transfer(addr); // set the register address
	spi_uci_transfer(mask); // set the mask byte
	spi_uci_transfer(byte); // set the data byte
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// read common status bits
// datasheet p.69, table 12.8
uint8_t mcp2515_read_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	retval = spi_uci_transfer(MCP2515_READ_STATUS); // send the status command
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval; // return the status
};

// read message rx status bits
// datasheet p.69, table 12.9
uint8_t mcp2515_read_rx_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	retval = spi_uci_transfer(MCP2515_RX_STATUS); // send the rx status command
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval; // return the status
};



// internal functions to this translation unit, aka. "private"

// loads '*data' into TX buffer identified by the bit mask 'buffer'
// datasheet p.65 and table 12.5
static void load_tx_buffer(const uint8_t buffer, const uint8_t *data, const uint8_t len)
{
	uint8_t i;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_LOAD_TX_BUFFER | buffer); // send load TX buffer command & buffer

	for(i=0; i<len; i++)
	spi_uci_transfer(*(data+i)); // write single byte or stream

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// loads a RX buffer identified by the bit mask 'buffer' into '*data'
// datasheet p.66 and table 12.3
void read_rx_buffer(const uint8_t buffer, uint8_t *data)
{
	uint8_t i;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	// the complete message can be max. 8 bytes long - 4 bytes for identifiers and 4 bytes for the payload
	// however, we need to read in 9 bytes: 4 identifier, one data length code, 4 actual data
	spi_uci_transfer(MCP2515_READ_RX_BUFFER | buffer); // send command along with bit mask

	for(i=0; i<10; i++)
		*(data+i) = spi_uci_transfer(0xFF); // read the whole byte stream for that RX buffer into data[]

	/*
	 *	now the data container holds:
	 *	data[0]		RXBnSIDH
	 *	data[1]		RXBnSIDL
	 *	data[2]		RXBnEID8
	 *	data[3]		RXBnEID0
	 *	data[4]		RXBnDLC
	 *	data[5]		RXBnD1
	 *	data[6]		RXBnD2
	 *	data[7]		RXBnD3
	 *	data[8]		RXBnD4
	 *	data[9]		RXBnD5
	 *	data[10]	RXBnD6
	 */


	// now get rid of the extended identifiers; the useful data lenght is saved in RXBnDLC
	*(data+2) = ( *(data+4) & ~0xF0); // clear the leftmost 4 bits of the DLC and save in data[2]

	for(i=0; i<*(data+2); i++)
		*(data+2+i) = *(data+5+i); // move bytes to the position of the extended identifier

	/* now we have in data:
	 *	data[0]		RXBnSIDH
	 *	data[1]		RXBnSIDL
	 *	data[2]		message length in bytes
	 *	data[3]		RXBnD1
	 *	data[4]		RXBnD2
	 *	data[5]		RXBnD3
	 *	data[6]		RXBnD4
	 *	data[7]		RXBnD5
	 *	data[8]		RXBnD6
	 */

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// returns byte with flags indicating which filter was matched
//	datasheet p.66, table 12.9
static uint8_t rx_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	retval = spi_uci_transfer(MCP2515_READ_STATUS); // send the RX status command and get data
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval;
};

// resets the MCP2515
static void reset(void)
{
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_RESET); // after a reset the device is in configuration mode (datasheet p. 59, para. 10.1)
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
}