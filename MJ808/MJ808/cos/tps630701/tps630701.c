#include "tps630701.h"
#include "cos\lmp92064sd\lmp92064sd.h"									// SPI Current/Voltage sensor

typedef struct															// tps630701_t actual
{
	tps630701_t public;													// public struct
	lmp92064sd_t *_lmp92064sd;											// SPI Voltage/Current meter
} __tps630701_t;

extern __tps630701_t __TPS630701;										// forward declare object

void _GetValues(void)													// download current and voltage measurement into external container
{
	/* example measurement:
	 * current
	 * 0x0203 - MSB - 1d
	 * 0x0202 - LSB - 84d
	 * 0x0184h
	 * voltage
	 * 0x0201 - MSB - 9d
	 * 0x0200 - LSB - 58d
	 * 0x0958h
*/
	__TPS630701._lmp92064sd->DownloadData(__TPS630701.public.OutValues);
};

__tps630701_t __TPS630701 =												// instantiation/initialization of object, saves us the constructor
{
	.public.OutValues = {255,255,255,255},
	._lmp92064sd = &LMP92064SD,											// save address of lmp92064sd_t object
	.public.PWM = (volatile uint8_t *) 0x56,							// address of OCR0A SFR
	.public.GetValues = &_GetValues										// set function pointer
};

tps630701_t * const TPS630701 = &__TPS630701.public;					// set const pointer to TPS630701 public part