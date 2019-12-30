#include "tps630701.h"
#include "cos\lmp92064sd\lmp92064sd.h"									// SPI Current/Voltage sensor

typedef struct															// tps630701_t actual
{
	tps630701_t public;													// public struct
	lmp92064sd_t *__lmp92064sd;											// SPI Voltage/Current meter
} __tps630701_t;

static __tps630701_t __TPS630701 __attribute__ ((section (".data")));	// declare tps630701_t actual and put into .data

void _GetValues(uint8_t *in_data_array)									// download current and voltage measurement into external container
{
	__TPS630701.__lmp92064sd->DownloadData(in_data_array);
};

tps630701_t *tps630701_ctor(void)										// initializes tps630701_t object
{
	__TPS630701.__lmp92064sd = lmp92064sd_ctor();						// get address of __lmp92064sd

	__TPS630701.public.PWM = (volatile uint8_t *) 0x56;					// set member to SFR address
	__TPS630701.public.GetValues = &_GetValues;							// download current and voltage measurement into external container

	return &__TPS630701.public;											// return address of public part
};