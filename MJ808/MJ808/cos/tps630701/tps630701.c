#include "tps630701.h"
#include "cos\lmp92064sd\lmp92064sd.h"									// SPI Current/Voltage sensor

typedef struct															// tps630701_t actual
{
	tps630701_t public;													// public struct
	lmp92064sd_t __lmp92064sd;											//
} __tps630701_t;

extern __tps630701_t __TPS630701;										// forward declare tps630701_t actual

void _GetValues(uint8_t *in_data_array)									// download current and voltage measurement into external container
{
	__TPS630701.__lmp92064sd.DownloadData(in_data_array);
};

__tps630701_t __TPS630701 =												// instantiate __tps630701_t actual and set function pointers
{
	.public.PWM = 0x056,												// OCR0A
	.public.GetValues = &_GetValues
};