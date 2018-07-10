#include <avr/io.h>
#include <util/delay.h>
#include "gpio.h"
#include "gpio_definitions.h"

#include "uci_spi.h"
#include "mcp2515.h"

/* the basic building blocks of interaction with the MCP2515:
 * opcodes -low level instructions- which the hardware executes
 *	they are meant to be "private" and not be used in main() directly
 */

static void mcp2515_opcode_reset(void); // reset - opcode 0xC0 - resets the MCP2515 (ch. 12.2, p 65)
void mcp2515_opcode_read_bytes(const uint8_t addr, uint8_t *data, const uint8_t len); // read - opcode 0x03 - reads len byt	es at addr and returns them via *data (ch 12.3,  p 65)
/*
//TODO - rewrite mcp2515_opcode_read_rx_buffer
static void mcp2515_opcode_read_rx_buffer(const uint8_t buffer, uint8_t *data); // read RX buffer - opcode 0x90 - loads a RX buffer identified by the bit mask 'buffer' into '*data', ch. 12.4, p 65
*/
void mcp2515_opcode_write_byte(const uint8_t addr, const uint8_t value); // write - opcode 0x02 - writes single byte to addr
void mcp2515_opcode_write_bytes(const uint8_t addr, const uint8_t *value, const uint8_t len); // write - opcode 0x02 - writes len-1 bytes to addr
static void mcp2515_opcode_load_tx_buffer(const uint8_t buffer, const uint8_t *data, const uint8_t len); // load TX buffer - opcode 0x40 - loads '*data' into TX buffer identified by the bit mask 'buffer'
void mcp2515_opcode_rts(const uint8_t buffer); // RTS - opcode 0x80 - sends RTS for 'buffer', table ch. 12.7 & 12.1
static uint8_t mcp2515_opcode_read_status(void); // read status - opcode 0xA0 - returns byte with status bits for msg RX & TX, ch. 12.8 & table 12.9
static uint8_t mcp2515_opcode_rx_status(void); // rx status - opcode 0xB0 - returns byte with received message RX filter and type info, ch. 12.9
void mcp2515_opcode_bit_modify(const uint8_t addr, const uint8_t mask, const uint8_t byte); //bit modify - opcode 0x05 - a means for setting specific registers, ch. 12.10 & figure 12-1

/* endianness and byte order are a problem on AVR
 *	say, we put word = 0x12af
 *	when reading, *(word+1) will hold the higher byte (12), while *(word) will hold the lower byte (af)
 *
 *	the solution is elegant:
 */

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
	// set bit timing
	mcp2515_opcode_bit_modify(CNF1, 0xFF, 0x00); // 1 x Tq sampling, 2 x ( 0 + 1) / 8 MHz prescaler
	mcp2515_opcode_bit_modify(CNF2, 0xFF, _BV(BTLMODE) | _BV(SAM) |_BV(PHSEG11) ); // Length of PS2 determined by PHSEG22:PHSEG20 bits of CNF3; PS1 Length (PHSEG1 + 1) x TQ
	mcp2515_opcode_bit_modify(CNF3, 0xFF, _BV(WAKFIL) | _BV(PHSEG21) ); // PS2 Length bits (PHSEG2 + 1) x TQ

	// TXRTSCTRL device settings
	mcp2515_opcode_bit_modify(TXRTSCTRL, 0x07, 0x00); // zero out & set B2RTSM, B1RTSM, B0RTSM to digital input (turn off RTS functionality on pins)

	// interrupts
	mcp2515_opcode_bit_modify(CANINTE, ( _BV(WAKIE) | _BV(ERRIE) | _BV(RX1IE) | _BV(RX0IE) ), _BV(WAKIE) | _BV(ERRIE) | ( _BV(RX1IE) | _BV(RX0IE) )); // enable wake, RX interrupts on INT pin

	// pins
	mcp2515_opcode_bit_modify(BFPCTRL, _BV(B1BFE) | _BV(B0BFE), 0x00); // disable RX0BF and RX1BF Pins, ch. 4.4 p 24

	// clear TXBnCTRL registers
	// TODO: for now for development only
	mcp2515_opcode_bit_modify(TXB0CTRL, _BV(TXREQ), 0x00);
	mcp2515_opcode_bit_modify(TXB1CTRL, _BV(TXREQ), 0x00);
	mcp2515_opcode_bit_modify(TXB2CTRL, _BV(TXREQ), 0x00);

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

	/*
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
*/
	mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x00); // put into normal mode
	//mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x40); // put into loopback mode
}

// provide data to MCP2515 and flag for TX over the CAN bus
//	provide not more than 4 bytes of data and len !!!
void mcp2515_can_msg_send(can_message_t *msg)
{
	/* mode of operation - see figure 3.1 on p.17
	 *	1. find an empty TX buffer
	 *	2.	select that buffer
	 *	3. load SIDH register
	 *	4. load SIDL register
	 *	5. if data is to be sent, again send load_tx_buffer (buffer incremented by one)
	 *	6.	load register
	 *	7. set data length code
	 *	8. determine message priority (not implemented)
	 *	9. load message priority into register (not implemented)
	 *	10. set RTS
	 */

 	uint8_t rts_mask = 0;
	uint8_t tx_buffer_addr = 0x00;

	do 	// step 1 & 2
	{
		msg->status = mcp2515_opcode_read_status(); // TXB0CNTRL.TXREQ (3rd bit), TXB1CNTRL.TXREQ (5th bit), TXB2CNTRL.TXREQ (7th bit) are of interest

		if ( (msg->status & 0x04) == 0 ) // TXB0CNTRL.TXREQ is cleared - i.e. find a buffer with no pending transmissions
		{
			tx_buffer_addr = TXB0CTRL; // select the TXB0CTRL buffer; we will use this as a base address for TX register loads and address relative to that base
			rts_mask = 0x01;
			break;
		}

		if ( (msg->status & 0x10) == 0 ) // TXB1CNTRL.TXREQ is cleared
		{
			tx_buffer_addr = TXB1CTRL;
			rts_mask = 0x02;
			break;
		}

		if ( (msg->status & 0x40) == 0 ) // TXB2CNTRL.TXREQ is cleared
		{
			tx_buffer_addr = TXB2CTRL;
			rts_mask = 0x04;
			break;
		}

		if (msg->status == 0x54) // all the buffers are full
			return; // error state
			//TODO - bus recovery on grave TX error

	} while (tx_buffer_addr); // some TXREQ bit must be clear before we continue (figure 12.8, p69)

	mcp2515_opcode_write_byte(++tx_buffer_addr, msg->sidh); // step 3 - load SIDH into TXBnSIDH; e.g. 0x31
	mcp2515_opcode_write_byte(++tx_buffer_addr, msg->sidl); // step 4 - load SIDL into TXBnSIDL; e.g. 0x32

	tx_buffer_addr += 2; // jump over the extended identifier registers; e.g. 0x33 and 0x34

	mcp2515_opcode_write_byte(++tx_buffer_addr, msg->dlc); // step 7 - load DLC  into TXBnDLC, 3.g. 0x35

	if (msg->dlc) // step 5 - if there is data to send - i.e. length is > 0
		mcp2515_opcode_write_bytes(++tx_buffer_addr, msg->data, msg->dlc); // step 6 - load data byte(s) into data registers, start at e.g. 0x36

	// step 8 - not implemented
	// step 9 - not implemented

	// step 10
	mcp2515_opcode_rts(rts_mask); // send RTS - flag the buffer for TX
	_delay_ms(5); // CHECKME: give other mj8x8s time to digest the new message, in case they are recieving
}

// fetches a received CAN message from the MCP2515, triggered by RX interrupt
void mcp2515_can_msg_receive(can_message_t *msg)
{
	/* mode of operation - see figure 4.2 on p.26
	 *	1. identify RX buffer
	 *	2.	select the appropriate buffer(s) in a loop
	 *	3.	select appropriate RXnIF
	 *	4. fetch various message bytes from the various registers
	 *	5. clear CANINTF.RXnIF
	 */

	uint8_t rx_buffer_addr = 0; // holds the RX buffer address
	uint8_t RXnIF; // holds the IRQ flag for message RX in the CANINTF register

	msg->status = mcp2515_opcode_rx_status(); // step 1: figure out in which RX buffer the incoming message is stored

	if ( !(msg->status & 0xC0) ) // bits 7 and 6 == 0 --> no RX message, nothing to do
		return;

	// step 2: select the appropriate buffer
		if (msg->status & 0x40) // if RXB0 is set - containing a message (bit 6), figure 12.9, p. 69
		{
			rx_buffer_addr = RXB0SIDH;	// mark that address
			msg->status &= ~0x40; // clear bit6
			RXnIF = RX0IF; // step 3: select appropriate RXnIF
		}

		else if (msg->status & 0x80) // if RXB1 is set - containing a message (bit 7), figure 12.9, p. 69
		{
			rx_buffer_addr = RXB1SIDH;
			msg->status &= ~0x80; // clear bit7
			RXnIF = RX1IF;
		}

		// step 4: fetch various message bytes from the various registers
		mcp2515_opcode_read_bytes(rx_buffer_addr, &(msg->sidh), 1); // RXBnSIDH
		mcp2515_opcode_read_bytes(++rx_buffer_addr, &(msg->sidl), 1); // RXBnSIDL

		rx_buffer_addr +=2;
		mcp2515_opcode_read_bytes(++rx_buffer_addr, &(msg->dlc), 1); // RXBnDLC

		if (msg->dlc) // if the data payload is more than zero
			mcp2515_opcode_read_bytes(++rx_buffer_addr, msg->data, msg->dlc); // RXBnDM

		// step 5: clear CANINTF.RXnIF
		mcp2515_opcode_bit_modify(CANINTF, _BV(RXnIF), 0x00);
}

// read message rx status bits
// datasheet p.69, table 12.9
uint8_t mcp2515_read_rx_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	retval = spi_uci_transfer(MCP2515_OPCODE_RX_STATUS); // send the rx status command
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval; // return the status
}



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
void mcp2515_opcode_read_bytes(const uint8_t addr, uint8_t *data, const uint8_t len)
{
	uint8_t i;

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_READ); // send read command
	spi_uci_transfer(addr); // send the address

	for (i = 0; i<len; ++i) // while the SS is held, the address is auto-incremented, thus multiple bytes can be read
		*(data+i) = spi_uci_transfer(0xff);	// get the result

	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
}

/*
// read RX buffer - opcode 0x90 - loads a RX buffer identified by the bit mask 'buffer' into '*data', ch. 12.4, p 65
// datasheet p.66 and table 12.3
//TODO - rewrite mcp2515_opcode_read_rx_buffer
void mcp2515_opcode_read_rx_buffer(const uint8_t buffer, can_message *msg)
{
	if (buffer == 0x00)
		address = RXB0SIDH;

	if (buffer == 0x02)
		address = RXB1SIDH;

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave

	spi_uci_transfer(MCP2515_OPCODE_READ_RX_BUFFER | buffer); // send command along with bit mask


	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
};
*/

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
	uint8_t i;

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_WRITE); // send the write command
	spi_uci_transfer(addr); // set the register address

	for(i=0; i<len; ++i)
		spi_uci_transfer( *(value+i) ); // write the byte(s)

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
}

// RTS - opcode 0x80 - sends RTS for 'buffer', table ch. 12.7 & 12.1
void mcp2515_opcode_rts(const uint8_t mask)
{
	if (mask == 0) // we didn't specify any buffer to do a RTS on
	return;	// hence, do nothing

	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_RTS | mask); // send RTS command & the buffer (bit mask)
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
}

// read status - opcode 0xA0 - returns byte with status bits for msg RX & TX, ch. 12.8 & table 12.9
static uint8_t mcp2515_opcode_read_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);	// select the slave
	spi_uci_transfer(MCP2515_OPCODE_READ_STATUS); // send the read status command
	retval = spi_uci_transfer(0xFF);	//  and get data
	gpio_set(SPI_SS_MCP2515_pin);	// de-select the slave
	return retval;
}

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
}