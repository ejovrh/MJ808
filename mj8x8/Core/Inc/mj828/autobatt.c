#include "main.h"

#if defined(MJ828_)	// if this particular device is active

#include "autobatt.h"

typedef struct	// autobatt_t actual
{
	autobatt_t public;  // public struct
} __autobatt_t;

static __autobatt_t __AutoBatt __attribute__ ((section (".data")));  // preallocate __AutoBatt object in .data

static double _VddaConversionConstant;

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

	// TODO - specify values for specific Li-Ion cells (once battery pack is empty)
	if(__AutoBatt.public.Vbat <= 4200 && __AutoBatt.public.Vbat > 1)  // below 4.2V (as displayed on DP832)
		{
			Device->led->led[Red].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 4200)  // 1302 - 4.2V to 5.4V
		{
			Device->led->led[Battery1].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 5400)  // 1675 - 5.4V to 6.6V
		{
			Device->led->led[Battery2].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 6600)  // 2048 - 6.6V to 7.8V
		{
			Device->led->led[Battery3].Shine(state);
		}

	if(__AutoBatt.public.Vbat > 7800)  // 2421 - above 7.8V
		{
			Device->led->led[Battery4].Shine(state);
		}
}

// AutoBatt functionality based on battery charge
static void _Do(void)
{
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
	__AutoBatt.public.Vbat = (uint16_t) ((double) Device->adc->GetChannel(Vbat) / Device->adc->GetChannel(Vrefint) * _VddaConversionConstant);

	/* ADC temperature calculation - see RM0091, chapter 13.8, p. 259
	 *
	 *	https://techoverflow.net/2015/01/13/reading-stm32f0-internal-temperature-and-voltage-using-chibios/
	 */

	__AutoBatt.public.Temp = (((((double) (Device->adc->GetChannel(Temperature) * VREFINT_CAL) / Device->adc->GetChannel(Vrefint)) - TS_CAL1) * 800) / (int16_t) (TS_CAL2 - TS_CAL1)) + 300;

	// TODO - take action based on battery states
//	if(__AutoBatt._ADCval <= 1714 && __AutoBatt._ADCval > 1)	// below 4.2V (as displayed on DP832)
//		{
//			MsgHandler->SendMessage(MSG_BUTTON_EVENT_04, 25, 2);	// send out event of battery < 25%
//		}
//
//	if(__AutoBatt._ADCval > 1302)  // 4.2V to 5.4V
//		{
//			MsgHandler->SendMessage(MSG_BUTTON_EVENT_04, 50, 2);	// send out event of battery < 50%
//		}
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
