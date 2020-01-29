#include "mcp73871.h"
#include "cos\mcp23s08\mcp23s08.h"										// SPI port expander
#include "cos\ad5160\ad5160.h"											// SPI Digital Potentiometer

typedef struct															// mcp73871_t actual
{
	mcp73871_t public;													// public struct
	ad5160_t *_ad5160;													// SPI potentiometer
	mcp23s08_t *_mcp23s08;												// SPI port expander
} __mcp73871_t;

extern __mcp73871_t __MCP73871;											// forward declare object

void _SetResistor(const uint8_t in_val)									// sets potentiometer to val - see datasheet p. 14
{
	__MCP73871._ad5160->SetWiper(in_val);								// simply write the value
};

uint8_t _GetMCP23S08(void)												// gets the device status (PG, STAT1, STAT2)
{
	return __MCP73871._mcp23s08->ReadRegisters();						// read GPIO from port expander
};

void _SetMCP23S08(const uint8_t in_register, const uint8_t in_arg)		// temporary for functional verification
{
	__MCP73871._mcp23s08->WriteRegister(in_register, in_arg);
};

__mcp73871_t __MCP73871 =
{
	._ad5160 = &AD5160,													// get address of ad5160_t object
	._mcp23s08 = &MCP23S08,												// get address of mcp23s08_t object

	.public.GetMCP23S08 = &_GetMCP23S08,								// gets mcp73871's device status (PG, STAT1, STAT2) via mcp23s08's GPIO
	.public.SetResistor = &_SetResistor,								// sets ad5160 potentiometer to val - see datasheet p. 14
	.public.SetMCP23S08 = &_SetMCP23S08									// temporary for functional verification
};

mcp73871_t * const MCP73871 = &__MCP73871.public;						// set const pointer to MCP73871 public part