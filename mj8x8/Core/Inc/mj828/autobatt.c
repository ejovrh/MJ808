#include "main.h"

#if defined(MJ828_)	// if this particular device is active

#include "autobatt.h"
// TODO - implement functionality
typedef struct	// autobatt_t actual
{
	autobatt_t public;  // public struct
} __autobatt_t;

static __autobatt_t __AutoBatt __attribute__ ((section (".data")));  // preallocate __AutoBatt object in .data

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

	volatile uint16_t temp = Device->adc->GetChannel(Vbat);

	if(temp <= 1714 && temp > 1)	// below 4.2V (as displayed on DP832)
		{
			Device->led->led[Red].Shine(state);
		}

	if(temp > 1302)  // 4.2V to 5.4V
		{
			Device->led->led[Battery1].Shine(state);
		}

	if(temp > 1675)  // 5.4V to 6.6V
		{
			Device->led->led[Battery2].Shine(state);
		}

	if(temp > 2048)  // 6.6V to 7.8V
		{
			Device->led->led[Battery3].Shine(state);
		}

	if(temp > 2321)  // above 7.8V
		{
			Device->led->led[Battery4].Shine(state);
		}
}

// TODO - implement AutoBatt functionality
static void _Do(void)
{
	;  // TODO - implement e.g. red blink on batt. level < 10%
}

static __autobatt_t __AutoBatt =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do,  //	that one method that does its thing
	.public.DisplayBatteryVoltage = &_DisplayBatteryVoltage,	//
	.public.AutoBattEnabled = 0,  //	set to off
	.public.AutoBattisOn = 0,  //
	};

autobatt_t* autobatt_ctor(void)  //
{
	return &__AutoBatt.public;  // set pointer to AutoBatt public part
}

#endif
