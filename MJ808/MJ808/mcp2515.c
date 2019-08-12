#include <avr/io.h>
#include <util/delay.h>

#include "gpio.h"
#include "uci_spi.h"
#include "mcp2515.h"

// TODO - move pin definitions out of here
#define	SPI_SS_MCP2515_pin		B,	4,	4								// SPI - SS

#if defined(MJ828_)
#define MCP2561_standby_pin		B,	3,	3								// MCP2561 standby
#endif
#if defined(MJ808_) || defined(MJ818_)
#define	MCP2561_standby_pin		B,	1,	1								// MCP2561 standby
#endif


/* the basic building blocks of interaction with the MCP2515:
 * opcodes -low level instructions- which the hardware executes
 *	they are meant to be "private" and not be used in main() directly
 *
 * internal functions to this translation unit, aka. "private"
 *	low-level instruction set: reset, read, read RX buffer, write, load TX buffer, RTS, read status, RX status, bit modify
 *	they are all described in the datasheet in chapter 12 - SPI interface
 */

// private functions here, object constructor at the end

//bit modify - opcode 0x05 - a means for setting specific registers, ch. 12.10 & figure 12-1
void mcp2515_opcode_bit_modify(const uint8_t addr, const uint8_t mask, const uint8_t byte)
{
	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave
	spi_uci_transfer(MCP2515_OPCODE_BIT_MODIFY);						// send the bit modify command
	spi_uci_transfer(addr);												// set the register address
	spi_uci_transfer(mask);												// set the mask byte - i.e. what is 1'ed can change, otherwise not
	spi_uci_transfer(byte);												// set the data byte - i.e. 1'ed becomes 1, 0 becomes 0
	_delay_us(1);														// delay a little bit for the transfer to complete
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
};

// read - opcode 0x03 - reads len byt	es at addr and returns them via *data (ch 12.3,  p 65)
void mcp2515_opcode_read_bytes(const uint8_t addr, volatile uint8_t *data, const uint8_t len)
{
	uint8_t i;

	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave
	spi_uci_transfer(MCP2515_OPCODE_READ);								// send read command
	spi_uci_transfer(addr);												// send the address

	for (i = 0; i<len; ++i)												// while the SS is held, the address is auto-incremented, thus multiple bytes can be read
	{
		*(data+i) = spi_uci_transfer(0xff);								// get the result
		_delay_us(1);													// delay a little bit for the transfer to complete
	}
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
};

// wrapper for single byte-read
uint8_t mcp2515_opcode_read_byte(const uint8_t addr)
{
	uint8_t retval;

	mcp2515_opcode_read_bytes( addr, &retval, 1);

	return retval;
};

// changes MCP2515 operation modes (and flushes any pending transmissions)
void mcp2515_change_opmode(const uint8_t mode)
{
	do
	{
		mcp2515_opcode_bit_modify(CANCTRL, _BV(ABAT), _BV(ABAT));		// set abort flag
		mcp2515_opcode_bit_modify(CANCTRL, 0xE0, mode);					// set REQOP2:0 bits into CANCTRL, datasheet p. 60
	} while ( (mcp2515_opcode_read_byte(CANSTAT) & 0xE0) != mode);		// loop if OPMOD2:0 bits from CANSTAT differ from the requested mode bits

	mcp2515_opcode_bit_modify(CANCTRL, _BV(ABAT), 0x00);				// unset abort flag
};

// reset - opcode 0xC0 - resets the MCP2515 (ch. 12.2, p 65)
void mcp2515_opcode_reset(void)
{
	mcp2515_opcode_bit_modify(EFLG, 0xC0, 0x00);						// set RX1OVR and RX0OVR to 0
	mcp2515_opcode_bit_modify(CANINTF, 0xFF, 0x00);						// set all interrupt flags to 0

	_delay_us(5);														// wait a while to begin

	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave
	spi_uci_transfer(MCP2515_OPCODE_RESET);								// after a reset the device is in configuration mode (datasheet p. 59, para. 10.1)
	_delay_us(5);														// delay a little bit for the transfer to complete
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave

	_delay_us(5);														// wait a while for the reset to take effect
};

// initialization & configuration after power on
void mcp2515_init(void)
{
	mcp2515_opcode_reset();												// reset first, must be held for at least 2us
	mcp2515_change_opmode(REQOP_CONFIG);								// set device into config mode, turn off clock out & prescaler stuff

	/* bit timing settings
	 * see https://www.kvaser.com/support/calculators/bit-timing-calculator/
	 * oscillator frequency: 8MHz, desired bandwidth: 500kbps
	 * datasheet pp. 39
	 */
	// set bit timing
	mcp2515_opcode_bit_modify(CNF1, 0xFF, 0x00);						// 1 x Tq sampling, 2 x ( 0 + 1) / 8 MHz prescaler
	mcp2515_opcode_bit_modify(CNF2, 0xFF, _BV(BTLMODE) | _BV(SAM) |_BV(PHSEG11) ); // Length of PS2 determined by PHSEG22:PHSEG20 bits of CNF3; PS1 Length (PHSEG1 + 1) x TQ
	mcp2515_opcode_bit_modify(CNF3, 0xFF, _BV(WAKFIL) | _BV(PHSEG21) ); // PS2 Length bits (PHSEG2 + 1) x TQ

	// TXRTSCTRL device settings
	mcp2515_opcode_bit_modify(TXRTSCTRL, 0x07, 0x00);					// zero out & set B2RTSM, B1RTSM, B0RTSM to digital input (turn off RTS functionality on pins)

	// interrupts
	mcp2515_opcode_bit_modify(CANINTE, ( _BV(WAKIE) | _BV(ERRIE) | _BV(RX1IE) | _BV(RX0IE) ), _BV(WAKIE) | _BV(ERRIE) | ( _BV(RX1IE) | _BV(RX0IE) )); // enable wake, RX interrupts on INT pin

	// pins
	mcp2515_opcode_bit_modify(BFPCTRL, _BV(B1BFE) | _BV(B0BFE), 0x00);	// disable RX0BF and RX1BF Pins, ch. 4.4 p 24

	// clear TXBnCTRL registers
	// TODO - implement actual filters, for now for development filters are blank
	mcp2515_opcode_bit_modify(TXB0CTRL, _BV(TXREQ), 0x00);
	mcp2515_opcode_bit_modify(TXB1CTRL, _BV(TXREQ), 0x00);
	mcp2515_opcode_bit_modify(TXB2CTRL, _BV(TXREQ), 0x00);

	// set up RX buffer control registers
	mcp2515_opcode_bit_modify(RXB0CTRL,  _BV(RXM0) | _BV(BUKT), _BV(RXM0) | _BV(BUKT)); // enable rollover, ch. 4.2 p 23, RX all messages & receive only standard identifiers for RXB0
	mcp2515_opcode_bit_modify(RXB1CTRL, _BV(RXM0), _BV(RXM0));			// receive only standard identifiers for RXB1

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

	mcp2515_change_opmode(REQOP_NORMAL); // put into normal mode
	//mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x40); // put into loopback mode
};

// read RX buffer - opcode 0x90 - loads a RX buffer identified by the bit mask 'buffer' into '*data', ch. 12.4, p 65
// datasheet p.66 and table 12.3
void mcp2515_opcode_read_rx_buffer(const uint8_t buffer, volatile uint8_t *data, const uint8_t len)
{
	uint8_t i = 0;

	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave

	if (buffer == RXB0SIDH)												//	select buffer
	spi_uci_transfer(MCP2515_OPCODE_READ_RX_BUFFER | MCP2515_OPCODE_READ_RX_BUFFER_RXB0SIDH);	// select RXB0 - send command along with bit mask

	if (buffer == RXB1SIDH)
	spi_uci_transfer(MCP2515_OPCODE_READ_RX_BUFFER | MCP2515_OPCODE_READ_RX_BUFFER_RXB1SIDH);	// select RXB1

	for (i = 0; i<len; ++i)												// while the SS is held, the address is auto-incremented, thus multiple bytes can be read
	*(data+i) = spi_uci_transfer(0xff);									// get the result

	_delay_us(1);														// delay a little bit for the transfer to complete
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
};

// rx status - opcode 0xB0 - returns byte with received message RX filter and type info, ch. 12.9
uint8_t mcp2515_opcode_rx_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave
	spi_uci_transfer(MCP2515_OPCODE_RX_STATUS);							// send the RX status command
	retval = spi_uci_transfer(0xFF);									// and get data
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
	_delay_us(1);														// delay a little bit for the transfer to complete
	return retval;
};

// read status - opcode 0xA0 - returns byte with status bits for msg RX & TX, ch. 12.8 & table 12.9
uint8_t mcp2515_opcode_read_status(void)
{
	uint8_t retval;
	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave
	spi_uci_transfer(MCP2515_OPCODE_READ_STATUS);						// send the read status command
	retval = spi_uci_transfer(0xFF);									//  and get data
	_delay_us(1);														// delay a little bit for the transfer to complete
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
	return retval;
};

// RTS - opcode 0x80 - sends RTS for 'buffer', table ch. 12.7 & 12.1
void mcp2515_opcode_rts(const uint8_t buffer)
{
	if (buffer == 0)													// we didn't specify any buffer to do a RTS on
	return;																// hence, do nothing

	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave
	spi_uci_transfer(MCP2515_OPCODE_RTS | buffer);						// send RTS command & the buffer (bit mask)
	_delay_us(1);														// delay a little bit for the transfer to complete
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
};

// load TX buffer - opcode 0x40 - loads '*data' into TX buffer identified by the bit mask 'buffer'
// ch12.6 & table 12.5
static void mcp2515_opcode_load_tx_buffer(const uint8_t buffer, volatile const uint8_t *data, const uint8_t len)
{
	uint8_t i;
	gpio_clr(SPI_SS_MCP2515_pin);										// select the slave

	if (buffer == TXB0CTRL)
	spi_uci_transfer(MCP2515_OPCODE_LOAD_TX_BUFFER | MCP2515_OPCODE_LOAD_TX_BUFFER_TXB0SIDH); // send load TX buffer command & buffer

	if (buffer == TXB1CTRL)
	spi_uci_transfer(MCP2515_OPCODE_LOAD_TX_BUFFER | MCP2515_OPCODE_LOAD_TX_BUFFER_TXB1SIDH); // send load TX buffer command & buffer

	if (buffer == TXB2CTRL)
	spi_uci_transfer(MCP2515_OPCODE_LOAD_TX_BUFFER | MCP2515_OPCODE_LOAD_TX_BUFFER_TXB2SIDH); // send load TX buffer command & buffer

	for(i=0; i<len; i++)
	spi_uci_transfer(*(data+i));										// write single byte or stream

	_delay_us(1);														// delay a little bit for the transfer to complete
	gpio_set(SPI_SS_MCP2515_pin);										// de-select the slave
};

// fetches a received CAN message from the MCP2515, triggered by RX interrupt
void mcp2515_can_msg_receive(volatile can_msg_t *msg)
{
	/* mode of operation - see figure 4.2 on p.26
	 *	1. identify RX buffer
	 *	2.	select the appropriate buffer(s) in a loop
	 *	3.	select appropriate RXnIF
	 *	4. fetch message bytes from the RXBn registers
	 */

	uint8_t rx_buffer_addr = 0;											// holds the RX buffer address

	msg->rx_status = mcp2515_opcode_rx_status();						// step 1: figure out in which RX buffer the incoming message is stored

	if ( !(msg->rx_status & 0xC0) )										// bits 7 and 6 == 0 --> no RX message, nothing to do
		return;

	// step 2: select the appropriate buffer
		if (msg->rx_status & 0x40)										// if RXB0 is set - containing a message (bit 6), figure 12.9, p. 69
		{
			rx_buffer_addr = RXB0SIDH;									// step 3: select appropriate buffer
			msg->rx_status &= ~0x40;									// clear bit6
		}

		if (msg->rx_status & 0x80)										// if RXB1 is set - containing a message (bit 7), figure 12.9, p. 69
		{
			rx_buffer_addr = RXB1SIDH;
			msg->rx_status &= ~0x80;									// clear bit7
		}

		// step 4: fetch message bytes from the RXBn registers
			// for convenience, all bytes are fetched - the unused extended identifier and all possibly unused data bytes
		mcp2515_opcode_read_rx_buffer(rx_buffer_addr, &(msg->sidh), 13);
};

// provide data to MCP2515 and flag for TX over the CAN bus
//	provide not more than 4 bytes of data and len !!!
void mcp2515_can_msg_send(volatile can_msg_t *msg)
{
	/* mode of operation - see figure 3.1 on p.17
	 *	1. find an empty TX buffer
	 *	2.	select that buffer
	 *	3. load all 13 message bytes in one row into the appropriate TX buffer
	 *	4. set RTS
	 *	5. unset the TX interrupt flag
	 */

 	uint8_t rts_mask = 0;
	uint8_t tx_buffer_addr = 0x00;

	do 	// step 1 & 2
	{
		msg->rx_status = mcp2515_opcode_read_status();					// TXB0CNTRL.TXREQ (3rd bit), TXB1CNTRL.TXREQ (5th bit), TXB2CNTRL.TXREQ (7th bit) are of interest

		if ( (msg->rx_status & 0x04) == 0 )								// TXB0CNTRL.TXREQ is cleared - i.e. find a buffer with no pending transmissions
		{
			tx_buffer_addr = TXB0CTRL;									// select the TXB0CTRL buffer; we will use this as a base address for TX register loads and address relative to that base
			rts_mask = 0x01;
			break;
		}

		if ( (msg->rx_status & 0x10) == 0 )								// TXB1CNTRL.TXREQ is cleared
		{
			tx_buffer_addr = TXB1CTRL;
			rts_mask = 0x02;
			break;
		}

		if ( (msg->rx_status & 0x40) == 0 )								// TXB2CNTRL.TXREQ is cleared
		{
			tx_buffer_addr = TXB2CTRL;
			rts_mask = 0x04;
			break;
		}

		if (msg->rx_status == 0x54)										// all the buffers are full
			return;														// error state
			//TODO - bus recovery on grave TX error in can_msg_send()

	} while (tx_buffer_addr);											// some TXREQ bit must be clear before we continue (figure 12.8, p69)

	// step 3
	mcp2515_opcode_load_tx_buffer(tx_buffer_addr, &(msg->sidh), 13);	// write all 13 message bytes in one two

	// step 4
	mcp2515_opcode_rts(rts_mask);										// send RTS - flag the buffer for TX

	// step 5
	mcp2515_opcode_bit_modify(CANINTF, (rts_mask<<2), 0x00);			// CHECKME - unset the TX IRQ flags
	_delay_us(5); // CHECKME: give other mj8x8s time to digest the new message, in case they are receiving
};

// puts the whole CAN infrastructure to sleep; 1 - sleep, 0 - awake
void can_sleep(volatile can_t *in_can, const uint8_t in_val)
{
	if ( !(in_can->in_sleep) && in_val)									// if is awake and set to sleep
	{
		mcp2515_change_opmode(REQOP_SLEEP);								// sleep MCP2515
		gpio_conf(MCP2561_standby_pin, OUTPUT, HIGH);					// sleep MCP2561
		in_can->in_sleep = 1;											// mark as sleeping
	}

	if (in_can->in_sleep && !in_val)									// if is sleeping and set to wake up
	{
		mcp2515_opcode_bit_modify(CANINTF, 0xFF, 0x00);					// clear out all interrupt flags so that a wakeup can be asserted (if there are not handled interrupts, a wakeup interrupt will never occur)
		mcp2515_opcode_bit_modify(CANINTF, _BV(WAKIF), _BV(WAKIF));		// create a wake up interrupt event -- the sucker will actually go and create a real one and go on to service it
	}
};

// object constructor
volatile can_t *can_ctor(volatile can_t *self)
{
	// populate self
	self->Sleep = &can_sleep;											// set up function pointer for public methods
	self->RequestToSend = &mcp2515_can_msg_send;						// ditto
	self->FetchMessage = &mcp2515_can_msg_receive;						// ditto
	self->ChangeOpMode = &mcp2515_change_opmode;						// ditto
	self->ReadBytes = &mcp2515_opcode_read_bytes;						// ditto
	self->BitModify = &mcp2515_opcode_bit_modify;						// ditto


	mcp2515_init();														// initialize & configure the MCP2515

	return self;														// hmm... - gets itself as a parameter and then returns itself. i smell bullshit in the making.
};

volatile can_t CAN __attribute__ ((section (".data")));		// define CAN object and put it into .data