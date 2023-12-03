#include "main.h"

#if defined(MJ828_)	// if this particular device is active

#include "autobatt.h"

typedef struct	// autobatt_t actual
{
	uint8_t LowBattery :1;  // flag indicating low battery status

	autobatt_t public;  // public struct
} __autobatt_t;

static __autobatt_t __AutoBatt __attribute__ ((section (".data")));  // preallocate __AutoBatt object in .data

// display voltage-based battery charge status depending on ADC read
static void _DisplayBatteryVoltage(void)
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

	if(Device->adc->GetChannel(Vbat) > BATTERY_20_PCT)  // 1890 - 6.1V to 6.6V - above approx. 20% charge
		{
			Device->led->led[Battery1].Shine(state);
		}

	if(Device->adc->GetChannel(Vbat) > BATTERY_40_PCT)  // 2070 - 6.6V to 7.8V	- above approx. 40% charge
		{
			Device->led->led[Battery2].Shine(state);
		}

	if(Device->adc->GetChannel(Vbat) > BATTERY_60_PCT)  // 2245 - 7.2V to 7.8V - above approx. 60% charge
		{
			Device->led->led[Battery3].Shine(state);
		}

	if(Device->adc->GetChannel(Vbat) > BATTERY_80_PCT)  // 2430 - above 7.8V	- above approx. 80% charge
		{
			Device->led->led[Battery4].Shine(state);
		}
}

// AutoBatt functionality based on battery charge
static void _Do(void)
{
	// low battery indicator
	if(__AutoBatt.LowBattery)  // if it is active
		{  // deactivate if possible
			if(Device->adc->GetChannel(Vbat) > BATTERY_10_PCT)  // if battery state above 10%)
				{
					Device->led->led[Red].Shine(OFF);  // indicate
					// TODO - work via some external lookup table so that in SendMessage() below something like "autobatt deactivate low voltage warning" is issued. the lookup of said string should be external to this file. the lookup should at the same time identify an action to be taken.
//					MsgHandler->SendMessage(MSG_BUTTON_EVENT_04, 10, 2);	// send out event with 10% as argument
					EventHandler->Notify(EVENT08);	// notify event
					__AutoBatt.LowBattery = 0;
				}
		}
	else
		{  // activate if needed
			if(Device->adc->GetChannel(Vbat) <= BATTERY_10_PCT && Device->adc->GetChannel(Vbat) > 1)  // if battery state below 10%)
				{
					Device->led->led[Red].Shine(ON);  // indicate
					EventHandler->Notify(EVENT08);	// notify event
					__AutoBatt.LowBattery = 1;
				}
		}
}

static __autobatt_t __AutoBatt =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do,  // set function pointer
	.public.DisplayBatteryVoltage = &_DisplayBatteryVoltage,	// ditto
	};

autobatt_t* autobatt_ctor(void)  //
{
	return &__AutoBatt.public;  // set pointer to AutoBatt public part
}

#endif
