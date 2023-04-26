#include "main.h"

#if defined(MJ828_)	// if this particular device is active

#include "autobatt.h"

typedef struct	// autobatt_t actual
{
	autobatt_t public;  // public struct
} __autobatt_t;

static __autobatt_t __AutoBatt __attribute__ ((section (".data")));  // preallocate __AutoBatt object in .data

static double _VddaConversionConstant;	// constant values pre-computed in constructor
static uint8_t i;  // iterator for average calculation
static uint32_t tempvbat;  // temporary variable for average calculation
static uint32_t tempvrefint;	// ditto
static uint32_t temptemp;  // ditto

// display battery charge status depending on ADC read
void _DisplayBatteryVoltage(void)
{
	static uint8_t state;

	if(state == 0)
		{
			state |= ON;
			state = 1;
		}
	else
		{
			state &= ~ON;
			state = 0;
		}

	if(__AutoBatt.public.Vbat <= 6075 && __AutoBatt.public.Vbat > 1)  // below 6.1V (as measured by internal ADC) - below 20% charge
		{
			Device->led->led[Red].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 6075)  // 1890 - 6.1V to 6.6V - above approx. 20% charge
		{
			Device->led->led[Battery1].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 6650)  // 2070 - 6.6V to 7.8V	- above approx. 40% charge
		{
			Device->led->led[Battery2].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 7200)  // 2245 - 7.2V to 7.8V - above approx. 60% charge
		{
			Device->led->led[Battery3].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 7800)  // 2430 - above 7.8V	- above approx. 80% charge
		{
			Device->led->led[Battery4].Shine(state);
		}
}

// AutoBatt functionality based on battery charge
static void _Do(void)
{
	tempvbat += Device->adc->GetChannel(Vbat);	// sum up raw data
	tempvrefint += Device->adc->GetChannel(Vrefint);	// ditto
	temptemp += Device->adc->GetChannel(Temperature);  // ...

	if(++i == ADC_MEASURE_ITERATIONS)
		{
			tempvbat /= ADC_MEASURE_ITERATIONS;  // divide over iterations
			tempvrefint /= ADC_MEASURE_ITERATIONS;	// ditto
			temptemp /= ADC_MEASURE_ITERATIONS;  // ...

			/* ADC channel voltage calculation - see RM 0091, chapter 13.8, p. 260
			 *
			 * using the Vrefint reference channel, the formula for calculating the ADC channel voltage is:
			 * 	Vchannel = (Vdda_charact. * Vrefint_cal * ADC_data) / (Vrefint_data * full scale)
			 *
			 * 	of these, only ADC_data and Vrefint_data are variable, the rest are constants which can be computed in advance
			 * 	thus, the formula becomes:
			 *
			 * 	Vchannel = (ADC_data/Vrefint_data) * ( (Vdda_charact. * Vrefint_cal) / full scale ) * 4
			 * 		the latter term is computed once in the constructor.
			 * 		in absolute numbers it is 4915.7509157509157
			 *
			 * 			since we are working with a 300k & 100k voltage divider, the above term was multiplied by 4
			 *
			 * 	Vchannel becomes (ADC_data/Vrefint_data) * VddaConversionConstant, true battery voltage in mV
			 * 	this is then typecast into uint16_t to have a nice round number
			 */

			__AutoBatt.public.Vbat = (uint16_t) ((double) tempvbat / tempvrefint * _VddaConversionConstant);

			/* ADC temperature calculation - see RM0091, chapter 13.8, p. 259
			 *
			 *	https://techoverflow.net/2015/01/13/reading-stm32f0-internal-temperature-and-voltage-using-chibios/
			 */

			__AutoBatt.public.Temp = (((((double) (temptemp * VREFINT_CAL) / tempvrefint) - TS_CAL1) * 800) / (int16_t) (TS_CAL2 - TS_CAL1)) + 300;

			tempvbat = 0;  // reset
			tempvrefint = 0;
			temptemp = 0;
			i = 0;
		}

	// TODO - take action based on battery states, Vbat values are example values only
	if(__AutoBatt.public.Vbat <= 5800 && __AutoBatt.public.Vbat > 1)	// if battery state below 10%)
		{
			Device->led->led[Red].Shine(BLINK);  // indicate
			MsgHandler->SendMessage(MSG_BUTTON_EVENT_04, 10, 2);	// send out event with 10% as argument
		}

	if(__AutoBatt.public.Vbat > 4900)  // TODO - replace example value with meaningful one
		{
			MsgHandler->SendMessage(MSG_BUTTON_EVENT_04, 50, 2);	//
		}
}

static __autobatt_t __AutoBatt =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do,  // set function pointer
	.public.DisplayBatteryVoltage = &_DisplayBatteryVoltage,	// ditto
	.public.Vbat = 0,  // battery voltage in mV
	.public.Temp = 0	// Temperature in tenths of deg. C (36.7 is 367)
	};

autobatt_t* autobatt_ctor(void)  //
{
	_VddaConversionConstant = (double) (3300 * VREFINT_CAL * 4) / 4095;  // 3300 - 3.3V for mV, 4 for resistor divider
	return &__AutoBatt.public;  // set pointer to AutoBatt public part
}

#endif
