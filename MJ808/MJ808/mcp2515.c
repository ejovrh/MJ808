#include <avr/io.h>
#include "gpio.h"
#include "gpio_definitions.h"

#include "uci_spi.h"
#include "mcp2515.h"

/* the basic building blocks of interaction with the MCP2515:
 * opcodes -low level instructions- which the hardware executes
 *	they are meant to be "private" and not be used in main() directly
 */

static void mcp2515_opcode_reset(void); // reset - opcode 0xC0 - resets the MCP2515 (ch. 12.2, p 65)
static void mcp2515_opcode_read(const uint8_t addr, uint8_t *data, const uint8_t len); // read - opcode 0x03 - reads len byt	es at addr and returns them via *data (ch 12.3,  p 65)
//TODO - rewrite mcp2515_opcode_read_rx_buffer
static void mcp2515_opcode_read_rx_buffer(const uint8_t buffer, uint8_t *data); // read RX buffer - opcode 0x90 - loads a RX buffer identified by the bit mask 'buffer' into '*data', ch. 12.4, p 65
void mcp2515_opcode_write_byte(const uint8_t addr, const uint8_t value); // write - opcode 0x02 - writes single byte to addr
void mcp2515_opcode_write_bytes(const uint8_t addr, const uint8_t *value, const uint8_t len); // write - opcode 0x02 - writes len-1 bytes to addr
static void mcp2515_opcode_load_tx_buffer(const uint8_t buffer, const uint8_t *data, const uint8_t len); // load TX buffer - opcode 0x40 - loads '*data' into TX buffer identified by the bit mask 'buffer'
void mcp2515_opcode_rts(const uint8_t buffer); // RTS - opcode 0x80 - sends RTS for 'buffer', table ch. 12.7 & 12.1
static uint8_t mcp2515_opcode_read_status(void); // read status - opcode 0xA0 - returns byte with status bits for msg RX & TX, ch. 12.8 & table 12.9
static uint8_t mcp2515_opcode_rx_status(void); // rx status - opcode 0xB0 - returns byte with received message RX filter and type info, ch. 12.9
static void mcp2515_opcode_bit_modify(const uint8_t addr, const uint8_t mask, const uint8_t byte); //bit modify - opcode 0x05 - a means for setting specific registers, ch. 12.10 & figure 12-1

/* endianness and byte order are a problem on AVR
 *	say, we put word = 0x12af
 *	when reading, *(word+1) will hold the higher byte (12), while *(word) will hold the lower byte (af)
 *
 *	the solution is elegant:
 */
uint16_t word; // the 2byte value
uint8_t *word_ptr = (uint8_t *) &word; // a 1byte pointer to the address of the 2byte value; the pointer gets incremented by 1 byte, not 2!

// initialization & configuration after power on
void mcp2515_init(void)
{
	mcp2515_opcode_reset(); // reset first, must be held for at least 2us
	mcp2515_opcode_bit_modify(CANCTRL, 0xff, 0x80); // set device into config mode, turn off clock out & prescaler stuff

	/* bit timing settings
	 * see https://www.kvaser.com/support/calculators/bit-timing-calculator/
	 * oscillator frequency: 8MHz, desired bandwidth: 500kbps
	 * datasheet pp. 39
	 */
	//TODO: verify correct write
	mcp2515_opcode_bit_modify(CNF1, 0xFF, 0x00); // 1 x Tq sampling, 2 x ( 0 + 1) / 8 MHz prescaler
	mcp2515_opcode_bit_modify(CNF2, 0xFF, _BV(BTLMODE) | _BV(SAM) |_BV(PHSEG11) ); // Length of PS2 determined by PHSEG22:PHSEG20 bits of CNF3; PS1 Length (PHSEG1 + 1) x TQ
	mcp2515_opcode_bit_modify(CNF3, 0xFF, _BV(WAKFIL) | _BV(PHSEG21) ); // PS2 Length bits (PHSEG2 + 1) x TQ

	// TXRTSCTRL device settings
	mcp2515_opcode_bit_modify(TXRTSCTRL, 0x07, 0x00); // zero out & set B2RTSM, B1RTSM, B0RTSM to digital input (turn off RTS functionality on pins)

	// misc stuff
	mcp2515_opcode_bit_modify(CANINTE, ( _BV(WAKIE) | _BV(RX1IE) | _BV(RX0IE) ), ( _BV(RX1IE) | _BV(RX0IE) )); // enable wake, RX interrupts on INT pin
	mcp2515_opcode_bit_modify(BFPCTRL, _BV(B1BFE) | _BV(B0BFE), 0x00); // disable RX0BF and RX1BF Pins, ch. 4.4 p 24

	// set up RX buffer control registers
	mcp2515_opcode_bit_modify(RXB0CTRL,  _BV(RXM0) | _BV(BUKT), _BV(RXM0) | _BV(BUKT)); // enable rollover, ch. 4.2 p 23, RX all messages & receive only standard identifiers for RXB0
	mcp2515_opcode_bit_modify(RXB1CTRL, _BV(RXM0), _BV(RXM0)); // receive only standard identifiers for RXB1

	// set up mask registers -- for development purposes lets let all through

	/*
	 * filters:
	 RXB0 [unicast]
	 1M, 2F
	 - RXM0: 0x01FF - mask unicast to own device class and ID (self)
	 - RXF0: 0x01B0 - front light towards own light ID (rear light)
	 - RXF1: 0x0170 - logic unit towards own light ID (rear light)

	 RXB1 [broadcast]
	 1M, 4F
	 - RXM1: 0x00FF - mask broadcast towards own device class
	 - RXF2: 0x0030 - dynamo towards lights (rear)
	 - RXF3: 0x0070 - logic units towards lights (rear)
	 - RXF4: 0x00B0 - front light towards lights (rear)
	 - RXF5: 0x00F0 - self towards own device class (rear)
	 */

// TODO: filter bytes in uint16_t format will need <<5 to match registers, additionally union byte order is a problem
	word = 0x01FF;
	mcp2515_opcode_write_byte(RXM0SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXM0SIDH, *(++word_ptr));

	word = 0x01B0;
	mcp2515_opcode_write_byte(RXF0SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXF0SIDH, *++word_ptr);

	word = 0x0170;
	mcp2515_opcode_write_byte(RXF1SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXF1SIDH, *++word_ptr);

	word = 0x00FF;
	mcp2515_opcode_write_byte(RXM1SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXM1SIDH, *++word_ptr);

	word = 0x0030;
	mcp2515_opcode_write_byte(RXF2SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXF2SIDH, *++word_ptr);

	word = 0x0070;
	mcp2515_opcode_write_byte(RXF3SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXF3SIDH, *++word_ptr);

	word = 0x00B0;
	mcp2515_opcode_write_byte(RXF4SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXF4SIDH, *++word_ptr);

	word = 0x00F0;
	mcp2515_opcode_write_byte(RXF5SIDL, *word_ptr);
	mcp2515_opcode_write_byte(RXF5SIDH, *++word_ptr);

	mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x00); // put into normal mode
	//mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x40); // put into loopback mode
};

// provide data to MCP2515 and flag for TX over the CAN bus
//	provide not more than 4 bytes of data and len !!!
void mcp2515_can_msg_send(can_message *msg)
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

/*
 *
 mcp2515_opcode_write_byte(TXB0SIDH, can_msg.sidh);
 mcp2515_opcode_write_byte(TXB0SIDL, can_msg.sidl);
 mcp2515_opcode_write_byte(TXB0DLC, can_msg.dlc);
 mcp2515_opcode_write_bytes(TXB0Dm, *can_msg.data, 2);
 mcp2515_opcode_rts(0x01);
 *
 */

	uint8_t buffer;
	uint8_t priority;

	// step 1 & 2
	// FIXME - this is ugly as hell, doesnt work and only a one-time shot; it needs to be looped
	buffer = mcp2515_opcode_read_status(); // TXB0CNTRL.TXREQ, TXB1CNTRL.TXREQ, TXB2CNTRL.TXREQ are of interest

	if (buffer & 0x04)
	{
		buffer = 0;
		mcp2515_opcode_write_byte(TXB0DLC, 1);
	}

	if (buffer & 0x16)
	{
		buffer = 2;
		mcp2515_opcode_write_byte(TXB1DLC, 1);
	}

	if (buffer & 0x64)
	{
		buffer = 4;
		mcp2515_opcode_write_byte(TXB2DLC, 1);
	}
return;
	// step 3 and 4
	mcp2515_opcode_load_tx_buffer(buffer, msg, 2); //	send the load_tx_buffer command for TXBnSIDH and TXBnSIDL, datasheet p. 66, table 12.1

	buffer +=1;
	// step 5 and 6
	mcp2515_opcode_load_tx_buffer(buffer, msg, msg->dlc); //	send the load_tx_buffer command for TXBnDm (data - if applicable), datasheet p. 66, table 12.1

	// step 7
	priority = msg->dlc; // the identifier high byte has only 3 bits - the leftmost 3...
	priority = msg->dlc;

	// step 8
	mcp2515_opcode_bit_modify(TXB0CTRL, ( _BV(TXP1) | _BV(TXP0) ), priority); // set message priority into TXBnCTRL.TXP1:0

	// step 9
	//TODO - set TXBnDLC.DLC3:0 - data length in bytes

	// step 10
	mcp2515_opcode_rts(buffer); // send RTS - flag the buffer for TX
	mcp2515_opcode_rts(buffer-1);
	// check TXBnCTRL.TXREQ for successful TX
};

// fetches a received CAN message from the MCP2515, triggered by RX interrupt
void mcp2515_can_msg_receive(can_message *msg)
{
	uint8_t buffer = mcp2515_opcode_read_status(); // figure out in which RX buffer the incoming message is stored
	buffer = (buffer >> 5); //  datasheet p.69, table 12.9 and 12.3
	mcp2515_opcode_read_rx_buffer(buffer, msg );	// pass the buffer and message container further on
};

// read message rx status bits
// datasheet p.69, table 12.9
uint8_t mcp2515_read_rx_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	retval = spi_uci_transfer(MCP2515_OPCODE_RX_STATUS); // send the rx status command
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval; // return the status
};



/* internal functions to this translation unit, aka. "private"
 *	low-level instruction set: reset, read, read RX buffer, write, load TX buffer, RTS, read status, RX status, bit modify
 *	they are all described in the datasheet in chapter 12 - SPI interface
 */

// reset - opcode 0xC0 - resets the MCP2515 (ch. 12.2, p 65)
static void mcp2515_opcode_reset(void)
{
	uint8_t i;

	for (i=0; i<255; ++i) // wait a while to begin
		; // do nothing

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_RESET); // after a reset the device is in configuration mode (datasheet p. 59, para. 10.1)
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave

	for (i=0; i<255; ++i) // wait a while for the reset to take effect
		; // do nothing
}

// read - opcode 0x03 - reads len byt	es at addr and returns them via *data (ch 12.3,  p 65)
static void mcp2515_opcode_read(const uint8_t addr, uint8_t *data, const uint8_t len)
{
	uint8_t i = 0;

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_READ); // send read command
	spi_uci_transfer(addr); // send the address

	for (i = 0; i<len && i < CAN_MAX_MSG_LEN; ++i) // while the SS is held, the address is auto-incremented, thus multiple bytes can be read
		*(data+i) = spi_uci_transfer(0xff);	// get the result

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// read RX buffer - opcode 0x90 - loads a RX buffer identified by the bit mask 'buffer' into '*data', ch. 12.4, p 65
// datasheet p.66 and table 12.3
//TODO - rewrite mcp2515_opcode_read_rx_buffer
void mcp2515_opcode_read_rx_buffer(const uint8_t buffer, uint8_t *data)
{
	uint8_t i;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	// the complete message can be max. 8 bytes long - 4 bytes for identifiers and 4 bytes for the payload
	// however, we need to read in 9 bytes: 4 identifier, one data length code, 4 actual data
	spi_uci_transfer(MCP2515_OPCODE_READ_RX_BUFFER | buffer); // send command along with bit mask

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

//FIXME -  i<*(data+2) seems to contain crap; loop does not work
//	for(i=0; i< (uint8_t) *(data+2); i++)
	for(i=0; i< 4; i++) // loop to 4 is incorrect and put there to barely work
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

// write - opcode 0x02 - writes single byte to addr
void mcp2515_opcode_write_byte(const uint8_t addr, const uint8_t value)
{
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_WRITE); // send the write command
	spi_uci_transfer(addr); // set the register address
	spi_uci_transfer(value); // write the byte
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
}

// write - opcode 0x02 - writes len-1 bytes to addr
void mcp2515_opcode_write_bytes(const uint8_t addr, const uint8_t *value, const uint8_t len)
{
	uint8_t i = len;
	--i;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_WRITE); // send the write command
	spi_uci_transfer(addr); // set the register address

	for(i=0; i<len; ++i)
		spi_uci_transfer(*value+i); // write the byte

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
}

// load TX buffer - opcode 0x40 - loads '*data' into TX buffer identified by the bit mask 'buffer'
// ch12.6 & table 12.5
static void mcp2515_opcode_load_tx_buffer(const uint8_t buffer, const uint8_t *data, const uint8_t len)
{
	uint8_t i;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_LOAD_TX_BUFFER | buffer); // send load TX buffer command & buffer

	for(i=0; i<len; i++)
		spi_uci_transfer(*(data+i)); // write single byte or stream

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// RTS - opcode 0x80 - sends RTS for 'buffer', table ch. 12.7 & 12.1
void mcp2515_opcode_rts(const uint8_t buffer)
{
	if (buffer == 0) // we didn't specify any buffer to do a RTS on
	return;	// hence, do nothing

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_RTS | buffer); // send RTS command & the buffer (bit mask)
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};

// read status - opcode 0xA0 - returns byte with status bits for msg RX & TX, ch. 12.8 & table 12.9
static uint8_t mcp2515_opcode_read_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_READ_STATUS); // send the read status command
	retval = spi_uci_transfer(0xFF);	//  and get data
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval;
};

// rx status - opcode 0xB0 - returns byte with received message RX filter and type info, ch. 12.9
static uint8_t mcp2515_opcode_rx_status(void)
{
		uint8_t retval;
		gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
		spi_uci_transfer(MCP2515_OPCODE_RX_STATUS); // send the RX status command
		retval = spi_uci_transfer(0xFF); // and get data
		gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
		return retval;
}

//bit modify - opcode 0x05 - a means for setting specific registers, ch. 12.10 & figure 12-1
void mcp2515_opcode_bit_modify(const uint8_t addr, const uint8_t mask, const uint8_t byte)
{
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_BIT_MODIFY); // send the bit modify command
	spi_uci_transfer(addr); // set the register address
	spi_uci_transfer(mask); // set the mask byte - i.e. what is 1'ed can change, otherwise not
	spi_uci_transfer(byte); // set the data byte - i.e. 1'ed becomes 1, 0 becomes 0
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};