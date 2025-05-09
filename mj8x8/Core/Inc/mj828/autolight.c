#include "main.h"

#if defined(MJ828_)	// if this particular device is active

#include "autolight.h"

typedef struct	// autolight_t actual
{
	autolight_t public;  // public struct
	uint8_t _AutoLightTimer;	// a delay timer/counter variable, updated every 250ms -- 2s delay is 8 iteration

} __autolight_t;

static __autolight_t __AutoLight __attribute__ ((section (".data")));  // preallocate __AutoLight object in .data

// implements AutoLight feature - turn light on/off automatically based on light sensor input (ADC data)
static void _Do(void)
{
	if(Device->mj8x8->GetActivity(AUTOLIGHT))  // if AutoLight is enabled
		{
			if(Device->adc->GetChannel(Darkness) > AUTOLIGHT_THRESHOLD_LIGHT_ON)  // if it is dark
				{
					// turn light on

					if(__AutoLight.public.FlagLightisOn == 0)  // run once
						{
							EventHandler->Notify(EVENT07);	// notify event
							__AutoLight.public.FlagLightisOn = 1;  // mark as on
						}
				}

			if(Device->adc->GetChannel(Darkness) < AUTOLIGHT_THRESHOLD_LIGHT_OFF)  // if it is light
				{
					// turn light off - but not immediately

					if(__AutoLight.public.FlagLightisOn == 1)  // run once ...
						{
							if(__AutoLight._AutoLightTimer > 8)  // ... if timer is expired
								{
									EventHandler->Notify(EVENT07);	//	notify event
									__AutoLight.public.FlagLightisOn = 0;  // mark as off
									__AutoLight._AutoLightTimer = 0;	// reset off-timer
								}

							++__AutoLight._AutoLightTimer;	// increment counter for off-timer - once every 250ms
						}
				}
		}
	else	// if AutoLight is disabled
		{
			if(__AutoLight.public.FlagLightisOn)
				{
					// turn everything off
					EventHandler->Notify(EVENT07);	// notify event
					__AutoLight.public.FlagLightisOn = 0;  // mark as off
				}
		}
}

static __autolight_t __AutoLight =  // instantiate autolight_t actual and set function pointers
	{  //
	.public.Do = &_Do,  //	that one method that does its thing
	.public.FlagLightisOn = 0,  // set to off
	._AutoLightTimer = 0,  // Initialize timer
	};

autolight_t* autolight_ctor(void)  //
{
	return &__AutoLight.public;  // set pointer to AutoLight public part
}

#endif
