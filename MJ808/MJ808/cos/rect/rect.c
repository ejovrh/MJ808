#include "cos\rect\rect.h"
#include "cos\mcp23s08\mcp23s08.h"										// SPI port expander

typedef struct															// rect_t actual
{
	rect_t public;														// public struct
	mcp23s08_t * const _mcp23s08;										// SPI port expander
} __rect_t;

extern __rect_t __RECT;													// forward declare object

// TODO - define argument format for rectifier operation modes
void _SetRectifierMode(const uint8_t in_mode)							// sets the rectifier to desired work mode: pure Graetz, Delon or tuning caps
{
	__RECT.public.RectifierMode = in_mode;								// set rectifier mode data member
	__RECT._mcp23s08->WriteRegister(MCP23S08_IODIR, in_mode);				// sets the rectifier to work as indicated by function argument by modification on the port expander
};

__rect_t __RECT =														// instantiation/initialization of object, saves us the constructor
{
	._mcp23s08 = &MCP23S08,												// get address of __mcp23s08
	.public.SetRectifierMode = &_SetRectifierMode						// sets rectifier operating mode
};

rect_t * const RECT = &__RECT.public;									// set const pointer to Reg public part