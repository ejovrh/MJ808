
#include "cos\reg\reg.h"
#include "cos\mcp23s08\mcp23s08.h"										// SPI port expander

typedef struct															// cap_t actual
{
	reg_t public;														// public struct
	mcp23s08_t *__mcp23s08;												// SPI port expander
} __reg_t;

static __reg_t __REG __attribute__ ((section (".data")));				// declare reg_t actual and put into .data

void _SetRegulatorMode(const uint8_t in_val)							// sets the regulator to desired work mode: pure Graetz, Delon or tuning caps
{
	__REG.public.RegulatorMode = in_val;								// set regulator mode data member
	__REG.__mcp23s08->SetFoo(in_val);									// sets the regulator to work as indicated by function argument by modification on the port expander
};

reg_t *reg_ctor()														// initializes reg_t object
{
	__REG.__mcp23s08 = mcp23s08_ctor();									// get address of __mcp23s08

	__REG.public.SetRegulatorMode = &_SetRegulatorMode;					// sets regulator operating mode

	return &__REG.public;												// return address of public part
};