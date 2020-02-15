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
	 current
		0x0203 - MSB - 01h
		0x0202 - LSB - 54h
		0x0154h complete current value
	 voltage
		0x0201 - MSB - 09h
		0x0200 - LSB - 3Ah
		0x093Ah complete voltage value

	Ratiometric measurements!!

	Voltage:
		resistor divider at ADC input stage: Vcc * ( 100k/(324k + 100k) ) = Vadc; Vadc * 4.24 = Vcc

	 	0x093A hex is 2362 decimal; full range is 4095 decimal
		2362/4095 = 0.577; 4095 is 2,048V, thus 2362 is approx. 2,048V*0.577 = 1.182V
		1.182V * 4.24 = 5.01V

		((2362/4095) * 2.048 )* 4.24)

	Current:
		shunt is 43m? = 0.043?;

		0x0154h is 340 decimal; full range is 3840 decimal
		340/3840 = 0.089; 3840 is 81.92mV = 0.08192V, thus 340 is approx. 0.08192V * 0.089 = 0.007V
		U/R = I gives 0.007V / 0.043? = 0.163A (rather large, but the PCB in question is known to fluctuate)


	two example charging cycle measurements:
	array indices 0,1 - current: 0x0423h (1059d), 0x02ceh (718d): 1059d equals 571mA, 718 equals 387mA
	array indices 2,3 - voltage: 0x0917h (2327d), 0x0922h (2338d): 2327d equals 4.93V, 2338 equals 4.96V
*/

	__TPS630701._lmp92064sd->DownloadData(__TPS630701.public.OutValues);
};

__tps630701_t __TPS630701 =												// instantiation/initialization of object, saves us the constructor
{
	._lmp92064sd = &LMP92064SD,											// save address of lmp92064sd_t object
	.public.PWM = (volatile uint8_t *) 0x56,							// address of OCR0A SFR
	.public.GetValues = &_GetValues										// set function pointer
};

tps630701_t * const TPS630701 = &__TPS630701.public;					// set const pointer to TPS630701 public part