#include "tps630701.h"
#include "cos\lmp92064sd\lmp92064sd.h"									// SPI Current/Voltage sensor

typedef struct															// tps630701_t actual
{
	tps630701_t public;													// public struct
	lmp92064sd_t *_lmp92064sd;											// SPI Voltage/Current meter
} __tps630701_t;

extern __tps630701_t __TPS630701;										// forward declare object

void _GetValues(uint8_t *in_data_array)									// download current and voltage measurement into external container
{
	__TPS630701._lmp92064sd->DownloadData(in_data_array);
};

__tps630701_t __TPS630701 =												// instantiation/initialization of object, saves us the constructor
{
	._lmp92064sd = &LMP92064SD,											// save address of lmp92064sd_t object
	.public.PWM = (volatile uint8_t *) 0x56,							// address of OCR0A SFR
	.public.GetValues = &_GetValues										// set function pointer
};

tps630701_t * const TPS630701 = &__TPS630701.public;					// set const pointer to TPS630701 public part