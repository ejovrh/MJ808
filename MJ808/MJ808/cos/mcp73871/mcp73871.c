#include "mcp73871.h"
#include "cos\mcp23s08\mcp23s08.h"										// SPI port expander
#include "cos\ad5160\ad5160.h"											// SPI Digital Potentiometer

typedef struct															// mcp73871_t actual
{
	mcp73871_t public;													// public struct
	ad5160_t __ad5160;													// SPI potentiometer
	mcp23s08_t __mcp23s08;												// SPI port expander
} __mcp73871_t;

extern __mcp73871_t __MCP73871;											// forward declare mcp73871_t actual

void _SetResistor(const uint8_t in_val)									// sets potentiometer to val - see datasheet p. 14
{
	__MCP73871.__ad5160.SetWiper(in_val);								// simply write the value
};

uint8_t _GetStatus(void)												// gets the device status (PG, STAT1, STAT2)
{
	return __MCP73871.__mcp23s08.GetBar(0x02);							// read GPIO from port expander
};

__mcp73871_t __MCP73871 =												// instantiate __mcp73871_t actual and set function pointers
{
	.public.SetResistor = &_SetResistor,								// sets ad5160 potentiometer to val - see datasheet p. 14
	.public.GetStatus = &_GetStatus										// gets mcp73871's device status (PG, STAT1, STAT2) via mcp23s08's GPIO
};