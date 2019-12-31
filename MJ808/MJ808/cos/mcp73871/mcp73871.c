#include "mcp73871.h"
#include "cos\mcp23s08\mcp23s08.h"										// SPI port expander
#include "cos\ad5160\ad5160.h"											// SPI Digital Potentiometer

typedef struct															// mcp73871_t actual
{
	mcp73871_t public;													// public struct
	ad5160_t *__ad5160;													// SPI potentiometer
	mcp23s08_t *__mcp23s08;												// SPI port expander
} __mcp73871_t;

static __mcp73871_t __MCP73871 __attribute__ ((section (".data")));		// declare mcp73871_t actual and put into .data

void _SetResistor(const uint8_t in_val)									// sets potentiometer to val - see datasheet p. 14
{
	__MCP73871.__ad5160->SetWiper(in_val);								// simply write the value
};

uint8_t _GetStatus(void)												// gets the device status (PG, STAT1, STAT2)
{
	//return __MCP73871.__mcp23s08->GetBar(0x02);							// read GPIO from port expander
};

mcp73871_t *mcp73871_ctor(void)											// initializes mcp73871_t object
{
	__MCP73871.__ad5160 = ad5160_ctor();								// get address of __ad5160
	__MCP73871.__mcp23s08 = mcp23s08_ctor();							// get address of __mcp23s08

	__MCP73871.public.GetStatus = &_GetStatus;							// gets mcp73871's device status (PG, STAT1, STAT2) via mcp23s08's GPIO
	__MCP73871.public.SetResistor = &_SetResistor;						// sets ad5160 potentiometer to val - see datasheet p. 14

	return &__MCP73871.public;											// return address of public part
};