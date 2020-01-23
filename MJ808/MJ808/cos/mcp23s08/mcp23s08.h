#ifndef MCP23S08_H_
#define MCP23S08_H_

#include <avr/io.h>
#include <inttypes.h>

#include "cos\cos.h"													// caller header file, contains pin definitions for SPI

#define MCP23S08_REGISTER_COUNT	11										// 11 configuration registers

#define MCP23S08_IODIR		0x00										// I/O direction register, IO7:0 - 1 input, 0 output
#define MCP23S08_IPOL		0x01										// Input Polarity port register, IP7:0 - 1 reflects opposite logic state as input pin, 0 same logic
#define MCP23S08_GPINTEN	0x02										// Interrupt on Change control register, GPINT7:0 - 1 enable interrupt change event, 0 disable
#define MCP23S08_DEFVAL		0x03										// Default compare register for interrupt on change, DEF7:0
#define MCP23S08_INTCON		0x04										// Interrupt on change control register, IOC7:0

#define MCP23S08_IOCON		0x05										// configuration register
#define MCP23S08_IOCON_SEQOP	05										// sequential operation mode bit - 1 enabled, 0 disabled
#define MCP23S08_IOCON_DISSLW	04										// slew rate control bit - 1 enabled, 0 disabled
#define MCP23S08_IOCON_HAEN		03										// Hardware Address Enable - 1 enabled, 0 disabled
#define MCP23S08_IOCON_ODR		02										// configures INT pin as open-drain output - 1 open drain (overrides INTPOL bit), 0 active driver
#define MCP23S08_IOCON_INTPOL	1										// sets the polarity of the INT output pin - 1 active high, 0 active low

#define MCP23S08_GPPU		0x06										// Pull-up resistor configuration register, PU7:0 - 1 enabled, 0 disabled
#define MCP23S08_INTF		0x07										// RO - interrupt flag register, INT7:0 - 1 interrupt pending, 0 not
#define MCP23S08_INTCAP		0x08										// RO - interrupt capture register, ICP7:0 - 1 logic high, 0 logic low
#define MCP23S08_GPIO		0x09										// Port register, GP7:0 - 1 logic high, 0 logic low
#define MCP23S08_OLAT		0x0a										// Output latch register, OL7:0 - 1 logic high, 0 logic low

enum MCP23S08RegisterIndex												// used to reference _register
{
	IODIR = 0,															// datasheet p. 10, I/O direction register: 0 - output, 1 - input
	IPOL = 1,															// datasheet p. 11, input polarity register: 0 - same logic state, 1 - opposite logic state
	GPINTEN = 2,														// datasheet p. 12, interrupt-on-change control register: 0 - disable interrupt, 1 - enable interrupt
	DEFVAL = 3,															// datasheet p. 13, default compare register for interrupts
	INTCON	= 4,														// datasheet p. 14, interrupt control register: 0 - pin compared to previous value, 1 - pin compared to defval
	IOCON = 5,															// datasheet p. 15, configuration register
	GPPU = 6,															// datasheet p. 16, pull-up configuration register: 0 - pull-up disabled, 1 - pull-up enabled
	INTF = 7,															// datasheet p. 17, interrupt flag register: 0 - interrupt not pending, 1 - pin caused interrupt
	INTCAP	= 8,														// datasheet p. 18, interrupt capture register: 0 - logic low, 1 - logic high
	GPIO = 9,															// datasheet p. 19, port (GPIO) register: 0 - logic low, 1 - logic high
	OLAT = 10															// datasheet p. 20, output latch register: 0 - logic low, 1 - logic high
};

typedef struct mcp23s08_t												// mcp23s08_t actual struct describing the port expander as a whole
{
	void (* const SendCommand)(const uint8_t in_command, const uint8_t in_val);	// sends command and value to MCP23S08
	uint8_t *(* const GetRegisters)(void);										// downloads device configuration registers into __MCP23S08.Registers[]

	uint8_t Register[MCP23S08_REGISTER_COUNT];							// array for storing downloaded device configuration registers
} mcp23s08_t __attribute__((aligned(8)));

extern mcp23s08_t MCP23S08;												// forward declare object

#endif /* MCP23S08_H_ */