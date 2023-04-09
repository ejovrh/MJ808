#ifndef MJ818_LED_C_
#define MJ818_LED_C_

#if defined(MJ818_)	// if this particular device is active

#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

extern TIM_HandleTypeDef htim2;  // rear light PWM on channel 1
extern TIM_HandleTypeDef htim3;  // brake light PWM on channel 4
extern TIM_HandleTypeDef htim14;  // Timer14 object - LED handling - 20ms

static primitive_led_t __primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

static const uint8_t _fade_fransfer[] =	// fade transfer curve according to MacNamara
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

volatile static uint8_t i = 0;	// rear
volatile static uint8_t j = 0;	// brake

// called indirectly by timer1 (_SystemInterrupt()), handles the fading
static void _MacNamaraFadeHandler(void)
{
	if(REAR_LIGHT_CCR < Device->led->led[Rear].ocr)  // fade up
		REAR_LIGHT_CCR = _fade_fransfer[i++];

	if(BRAKE_LIGHT_CCR < Device->led->led[Brake].ocr)  // fade up
		BRAKE_LIGHT_CCR = _fade_fransfer[j++];

	if(REAR_LIGHT_CCR > Device->led->led[Rear].ocr)  // fade down
		{
			REAR_LIGHT_CCR = _fade_fransfer[--i];

			if(REAR_LIGHT_CCR == 0)
				Device->activity->RearLightOn = 0;	// mark inactivity
		}

	if(BRAKE_LIGHT_CCR > Device->led->led[Brake].ocr)  // fade down
		{
			BRAKE_LIGHT_CCR = _fade_fransfer[--j];

			if(BRAKE_LIGHT_CCR == 0)
				Device->activity->BrakeLightOn = 0;	// mark inactivity
		}

	if(Device->activity->RearLightOn == 0 && Device->activity->BrakeLightOn == 0)
		{
			Device->StopTimer(&htim14);  // stop the timer - LED handling
			Device->StopTimer(&htim2);  // stop the timer - rear light PWM
			Device->StopTimer(&htim3);	// stop the timer - brake light PWM
		}

	if(BRAKE_LIGHT_CCR == Device->led->led[Brake].ocr && REAR_LIGHT_CCR == Device->led->led[Rear].ocr)
		Device->StopTimer(&htim14);  // stop the timer
}

// set OCR value to fade to
static void _primitiveRearLED(uint8_t value)
{

	Device->StartTimer(&htim14);  // start the timer - LED handling
	__HAL_TIM_DISABLE_IT(&htim14, TIM_IT_UPDATE);	// disable interrupts until ocr is set (timer14's ISR will otherwise kill it very soon)

	if(value)
		{
			Device->StartTimer(&htim2);  // start the timer - rear light PWM
			Device->activity->RearLightOn = 1;
		}

	Device->led->led[Rear].ocr = value;  // set OCR value, the handler will do the rest
	__HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);	// start timer
}

// does high beam on/off without fading
static void _BrakeLight(const uint8_t argument)
{
	static uint8_t OldOCR;	// holds previous OCR value

	if(argument == ARG_BRAKELIGHT_OFF)	// brake light off command
		{
			BRAKE_LIGHT_CCR = OldOCR;	// restore original OCR

			if(OldOCR == 0)
				{
					// FIXME - _BrakeLight off - sometimes light stays on (CAN seems ok)
					Device->StopTimer(&htim3);  // stop the timer - brake light PWM
					Device->StartTimer(&htim3);  // stop the timer - brake light PWM
					Device->StopTimer(&htim3);  // stop the timer - brake light PWM
				}

			Device->activity->BrakeLightOn = 0;	// mark inactivity
			return;
		}

	if(argument == ARG_BRAKELIGHT_ON)	// brake light on command
		{
			Device->activity->BrakeLightOn = 1;	// mark activity
			OldOCR = BRAKE_LIGHT_CCR;	// store original OCR value

			if(BRAKE_LIGHT_CCR == 0)	// light was previously off
				Device->StartTimer(&htim3);  // start the timer - brake light PWM

			BRAKE_LIGHT_CCR = 100;	// brake light on
			return;
		}
}

// set OCR value to fade to
static void _primitiveBrakeLED(uint8_t value)
{
	if(value >= REAR_BRAKELIGHT)	// special case for brake light
		{
			_BrakeLight(value);
			return;
		}

	Device->StartTimer(&htim14);  // start the timer - LED handling
	__HAL_TIM_DISABLE_IT(&htim14, TIM_IT_UPDATE);	// disable interrupts until ocr is set (timer14's ISR will otherwise kill it very soon)

	if(value)
		Device->StartTimer(&htim3);  // start the timer - brake light PWM

	Device->led->led[Brake].ocr = value;	// set OCR value, the handler will do the rest
	__HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);	// start timer
}

static inline void __componentLED_On(const uint8_t val)
{
	// note - if the order is reversed, it doesnt work:
	Device->led->led[Rear].Shine(val);  // rear light on
//	Device->led->led[Brake].Shine(val);  // brake LED on
}

static inline void __componentLED_Off(void)
{
	Device->led->led[Brake].Shine(0);  // brake LED off
	Device->led->led[Rear].Shine(0);  // rear light off
}

static void _componentLED(const uint8_t val)
{
	if(val)  // true - on, false - off
		__componentLED_On(val);  // delegate indirectly to the leaves
	else
		__componentLED_Off();
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,  // assign pointer to LED array
	.public.Shine = &_componentLED,  // component part ("interface")
	._flags = 0  //
	};

composite_led_t* _virtual_led_ctorMJ818()
{
	__LED.public.led[Rear].Shine = &_primitiveRearLED;  // LED-specific implementation
	__LED.public.led[Brake].Shine = &_primitiveBrakeLED;  // LED-specific implementation
	__LED.public.Handler = &_MacNamaraFadeHandler;  // timer-based periodic LED control function

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ818_

#endif /* MJ808_LED_C_ */
