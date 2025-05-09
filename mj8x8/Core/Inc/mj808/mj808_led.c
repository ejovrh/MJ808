#ifndef MJ808_LED_C_
#define MJ808_LED_C_

#if defined(MJ808_)	// if this particular device is active

#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

extern TIM_HandleTypeDef htim3;  // front light PWM on channel 1
extern TIM_HandleTypeDef htim14;  // LED handling - 20ms

// fixme - should be 3
static primitive_led_t __primitive_led[4] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

static uint32_t (*_fptr)(const uint8_t state);	// function pointer for branch table
static uint8_t _BlinkExclusionMask;	// exclusion mask used for blinking

static const uint8_t _fade_transfer[] =	// fade transfer curve according to MacNamara
	{  // see https://tigoe.github.io/LightProjects/fading.html
	0, 0, 0, 0, 0, 0, 0, 1, 1, 1,	// 10 by 10
	1, 1, 1, 1, 1, 1, 1, 1, 2, 2,	// ...
	2, 2, 2, 2, 2, 2, 3, 3, 3, 3,	//
	3, 4, 4, 4, 4, 4, 4, 5, 5, 5,	//
	6, 6, 6, 7, 7, 7, 8, 8, 9, 9,	//
	9, 10, 10, 11, 12, 12, 13, 13, 14, 15,  //
	16, 16, 17, 18, 19, 20, 21, 22, 23, 24,	//
	25, 27, 28, 29, 31, 32, 34, 35, 37, 39,	//
	41, 43, 45, 47, 49, 51, 54, 57, 59, 62,	//
	65, 68, 71, 75, 78, 82, 86, 90, 94, 100	//
	};

static volatile uint8_t front_iterator = 0;	// front

// fades front light pleasingly for the human eye
static void _MacNamaraFader(void)
{
	/* the primitive LED functions do start timers, yet this function turns them off when needed
	 * the iterators are stateful (static), so their value in a way reflects the CCR state.
	 */

	// front light
	if(FRONT_LIGHT_CCR < Device->led->led[Front].ocr)  // fade up
		{
			FRONT_LIGHT_CCR = _fade_transfer[front_iterator++];	// from lookup table into timer's CCR

			if (FRONT_LIGHT_CCR >= Device->led->led[Front].ocr  && __LED._BlinkFlags == 0)	// if we did fade up for long enough
					Device->StopTimer(&htim14);  // stop the LED handling timer since fading ought to end; the iterator will keep its value
		}

	if(FRONT_LIGHT_CCR > Device->led->led[Front].ocr)  // fade down
		{
			FRONT_LIGHT_CCR = _fade_transfer[--front_iterator];	// from lookup table into timer's CCR

			if (FRONT_LIGHT_CCR <= Device->led->led[Front].ocr)	// if we did fade down for long enough
				{
					if (__LED._BlinkFlags == 0)
						Device->StopTimer(&htim14);  // stop the LED handling timer since fading ought to end; the iterator will keep its value

					if(FRONT_LIGHT_CCR == 0)	// if the light is off
						{
							Device->StopTimer(&htim3);  // stop the PWM timer - rear light PWM
							if (__LED._BlinkFlags == 0)
								Device->StopTimer(&htim14);  // stop the LED handling timer since fading ought to end; the iterator will keep its value

							Device->mj8x8->UpdateActivity(FRONTLIGHT, OFF);	// update the bus
						}
				}
		}
}

// does high beam on/off without fading
static void _HighBeam(const uint8_t value)
{
	static uint8_t OldOCR;	// holds previous OCR value

	if(value == ARG_HIGHBEAM_OFF)	// high beam off command
		{
			Device->mj8x8->UpdateActivity(HIGHBEAM, OFF);	// update the bus

			if(Device->mj8x8->GetActivity(FRONTLIGHT))	// if front light is on
				{
					FRONT_LIGHT_CCR = OldOCR;	// restore original OCR
					return;
				}
			else
				{
					FRONT_LIGHT_CCR = 0;	// turn off light
					Device->StopTimer(&htim3);  // stop the timer - front light PWM
					Device->StartTimer(&htim3);
					Device->StopTimer(&htim3);
					return;
				}
		}

	if(value == ARG_HIGHBEAM_ON)	// high beam on command
		{
			Device->mj8x8->UpdateActivity(HIGHBEAM, ON);	// update the bus
//			__LED._ShineFlags |= _BV(HighBeam);	// set the high beam flag

			if(Device->mj8x8->GetActivity(FRONTLIGHT))	// if front light is on
				OldOCR = FRONT_LIGHT_CCR;	// store original OCR value
			else
				Device->StartTimer(&htim3);  // start the timer - front light PWM

			FRONT_LIGHT_CCR = 100;	// high beam on
			return;
		}
}

// delegates operations from LED component downwards to LED leaves
static void _componentLED(const uint8_t val)
{
	if(val)  // true - on, false - off
		{	// delegate indirectly to the leaves
			Device->led->led[Green].Shine(ON);  // green LED on
			Device->led->led[Front].Shine(val);  // front light on - low key; gets overwritten by LU command, since it comes in a bit later
		}
	else
		{	// delegate indirectly to the leaves
			Device->led->led[Green].Shine(OFF);	// green LED off
			Device->led->led[Front].Shine(0);  // front light off
		}
}

// set OCR value to fade to
static inline void _physicalFrontLED(const uint8_t value)
{
	if(value >= FRONT_HIGHBEAM)	// special case for high beam
		{
			_HighBeam(value);
			return;
		}

	__HAL_TIM_DISABLE_IT(&htim14, TIM_IT_UPDATE);	// disable interrupts until ocr is set (timer14's ISR will otherwise kill it very soon)
	Device->StartTimer(&htim14);  // start the timer - LED handling

	if(value && (Device->mj8x8->GetActivity(FRONTLIGHT) == OFF))
		{
			Device->StartTimer(&htim3);  // start the timer - front light PWM
			Device->mj8x8->UpdateActivity(FRONTLIGHT, ON);	// update the bus
			__LED._ShineFlags |= _BV(Front);	// set the front light flag
		}

	Device->led->led[Front].ocr = value;	// set OCR value, the _LEDHandler() will do the rest
	__HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);	// start timer
}

// handler for physical LED
static inline void __physicalRedLED(const uint8_t state)  // red LED on/off
{
#if USE_UTIL_LED
	// state == 1 - off
	// state == 0 - on
	HAL_GPIO_WritePin(RedLED_GPIO_Port, RedLED_Pin, (! state));

	if (! __LED._BlinkFlags)	// if we aren't blinking...
		Device->mj8x8->UpdateActivity(UTILLED, (__LED._ShineFlags & 0x03 ) > 0);	// update the bus
#else
	(void) state;
#endif

	return;
}

// handler for physical LED
static inline void __physicalGreenLED(const uint8_t state)  // green LED on/off
{
#if USE_UTIL_LED
	// state == 1 - off
	// state == 0 - on
	HAL_GPIO_WritePin(GreenLED_GPIO_Port, GreenLED_Pin, (! state));

	if (! __LED._BlinkFlags)	// if we aren't blinking...
		Device->mj8x8->UpdateActivity(UTILLED, (__LED._ShineFlags & 0x03 ) > 0);	// update the bus
#else
	(void) state;
#endif

	return;
}

// branch table for direct primitive LED execution
static uint32_t (*__physicalLEDBranchTable[])(const uint8_t state) =
		{//
				(void *)&__physicalRedLED,	// physical red LED on/off
				(void *)&__physicalGreenLED,	// physical green LED on/off
				(void *)&_physicalFrontLED,	// physical front light CCR value set
		};

// concrete utility LED handling function
static inline void __LEDBackEnd(const uint8_t led, const uint8_t state)
{
	if (state == ON)	// transition from BLINK to ON state
		__LED._OldBlinkFlags = __LED._BlinkFlags;	// store previous blink state

	if(state == OFF) // transition from SHINE to BLINK state
		{
			if (__LED._OldBlinkFlags != 0)	// ...really from SHINE to BLINK
				{
					__LED._BlinkFlags = __LED._OldBlinkFlags;
					__LED._OldBlinkFlags &= ~__LED._OldBlinkFlags;
				}
			else	// observe for dirty write condition (blink issued from other devices)
			__LED._BlinkFlags ^= ((-((state>>1) & 0x01) ^ __LED._BlinkFlags) & (1 << led));
		}
		else	// transition to BLINK (or true OFF) state
			__LED._BlinkFlags ^= ((-((state>>1) & 0x01) ^ __LED._BlinkFlags) & (1 << led));	// sets "led" bit to "state" value

	if(__LED._BlinkFlags == 0)
		Device->StopTimer(&htim14);  // stop the timer

	// fixme - front led shine flag remains set after led is off
	__LED._ShineFlags ^= ((-(state & 0x01) ^ __LED._ShineFlags) & (1 << led));	// sets "led" bit to "state" value

	Device->mj8x8->UpdateActivity(UTILLED, (__LED._BlinkFlags | 0x03) > 0);	// update the bus, but only for blinking (timer is needed); shining doesnt need the timer and wont set this bit

	if(state == BLINK)	// state blink
		{
			Device->StartTimer(&htim14);	//
			return;
		}

	_fptr = __physicalLEDBranchTable[led];	// set function pointer to indicated address of primitive LED function
	(_fptr)(state);	// execute according to arg
}

// frontend for the primitive red LED handler
static inline void __primitiveRedLEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Red, state);	// execute according to state
}

// frontend for the primitive green LED handler
static inline void __primitiveGreenLEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Green, state);	// execute according to state
}

//// frontend for the primitive front light handler
//static inline void __primitiveFrontLightHandler(const uint8_t state)
//{
//	__LEDBackEnd(Front, state);	// execute according to state
//}

// handles blinking
static void _Blinker(void)
{
	static uint8_t i = 0;  // persistent iterator across function calls loops over all LEDs on device

	if((__LED._ShineFlags | __LED._BlinkFlags) == 0)	// if there is any LED to glow at all
		{
			Device->StopTimer(&htim14);  // stop the timer
			Device->mj8x8->UpdateActivity(UTILLED, OFF);	// update the bus
			return;
		}

	// 126 = slow, 64 - medium, 32 = fast
	if ((__LED._BlinkCounter & 16) == 0)	// blink frequency by means of ANDing whole base-2 numbers
		_BlinkExclusionMask &= (uint8_t) (~__LED._BlinkFlags);	// clear particular bits - populate exclusion (i.e. primitive LED off for blinking)
	else
		_BlinkExclusionMask = 0xFF;	// all on (normal glow)

	_fptr = __physicalLEDBranchTable[i];	// set function pointer to indicated address of primitive LED function
	(_fptr)(  ( (__LED._ShineFlags | (__LED._BlinkFlags & _BlinkExclusionMask) ) & _BV(i) ) > 0  );	// it is ridiculous, i know...

	(i >= 1) ? i = 0 : ++i;  // circular bit-wise iterator over a byte range, count up to 7 and then restart from zero (we have 8 LEDs)

	if (__LED._BlinkFlags)	// increment the counter only if there is stuff to blink
		++__LED._BlinkCounter;
}

// called indirectly by timer1 (_SystemInterrupt()), handles the fading (and blinking)
static void _LEDHandler(void)
{
	if (Device->mj8x8->GetActivity(FRONTLIGHT))
		_MacNamaraFader();	// fades front light pleasingly for the human eye

	if (__LED._BlinkFlags)
		_Blinker(); // handles blinking
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,  // assign pointer to LED array
	.public.Shine = &_componentLED,  // component part ("interface")
	._ShineFlags = 0	//
	};

// implementation of virtual constructor for LEDs
composite_led_t* _virtual_led_ctor()
{
	__LED.public.led[Red].Shine = &__primitiveRedLEDFrontEnd;  // control function for one single LED
	__LED.public.led[Green].Shine = &__primitiveGreenLEDFrontEnd;	// ditto
	__LED.public.led[Front].Shine = &_physicalFrontLED;  // ditto
	__LED.public.Handler = &_LEDHandler;  // timer-based periodic LED control function

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ808_

#endif /* MJ808_LED_C_ */
