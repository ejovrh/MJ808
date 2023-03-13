#ifndef MJ818_LED_C_
#define MJ818_LED_C_

#if defined(MJ818_)	// if this particular device is active

#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

extern TIM_HandleTypeDef htim2;  // rear light PWM on channel 1
extern TIM_HandleTypeDef htim3;  // brake light PWM on channel 4
extern TIM_HandleTypeDef htim14;  // Timer14 object - LED handling - 20ms

static primitive_led_t __primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

// called indirectly by timer1 (_SystemInterrupt()), handles the fading
static void _FadeHandler(void)
{
	if(Device->led->led[Rear].ocr > OCR_REAR_LIGHT)  // fade up
		++OCR_REAR_LIGHT;

	if(Device->led->led[Brake].ocr > OCR_BRAKE_LIGHT)  // fade up
		++OCR_BRAKE_LIGHT;

	if(Device->led->led[Rear].ocr < OCR_REAR_LIGHT)  // fade down
		--OCR_REAR_LIGHT;

	if(Device->led->led[Brake].ocr < OCR_BRAKE_LIGHT)  // fade down
		--OCR_BRAKE_LIGHT;

	if(OCR_BRAKE_LIGHT == 0)
		Device->activity->BrakeLightOn = 0;	// mark inactivity

	if(OCR_REAR_LIGHT == 0)
		Device->activity->RearLightOn = 0;	// mark inactivity

	if(OCR_BRAKE_LIGHT == 0 && OCR_REAR_LIGHT == 0)
		{
			Device->StopTimer(&htim14);  // stop the timer - LED handling
			Device->StopTimer(&htim2);  // stop the timer - rear light PWM
			Device->StopTimer(&htim3);	// stop the timer - brake light PWM
		}

	if(OCR_BRAKE_LIGHT == Device->led->led[Brake].ocr && OCR_REAR_LIGHT == Device->led->led[Rear].ocr)
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
static void _BrakeLight(const uint8_t value)
{
	static uint8_t OldOCR;	// holds previous OCR value

	if(value == 200)	// brake light off command
		{
			Device->activity->BrakeLightOn = 0;	// mark inactivity
			OCR_BRAKE_LIGHT = OldOCR;	// restore original OCR

			if(OldOCR == 0)
				Device->StopTimer(&htim3);  // stop the timer - brake light PWM
		}

	if(value > 200)	// brake light on command
		{
			Device->activity->BrakeLightOn = 1;	// mark activity
			OldOCR = OCR_BRAKE_LIGHT;	// store original OCR value

			if(OCR_FRONT_LIGHT == 0)	// light was previously off
				Device->StartTimer(&htim3);  // start the timer - brake light PWM

			OCR_BRAKE_LIGHT = 100;	// brake light on
		}
}

// set OCR value to fade to
static void _primitiveBrakeLED(uint8_t value)
{
	if(value >= 200)	// special case for brake light
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
	Device->led->led[Brake].Shine(val);  // brake LED on - low key gets overwritten by LU command, since it comes in a bit later
	Device->led->led[Rear].Shine(val);  // rear light on
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
	.flags = 0  //
	};

composite_led_t* _virtual_led_ctorMJ818()
{
	__LED.public.led[Rear].Shine = &_primitiveRearLED;  // LED-specific implementation
	__LED.public.led[Brake].Shine = &_primitiveBrakeLED;  // LED-specific implementation
	__LED.public.Handler = &_FadeHandler;  // timer-based periodic LED control function

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ818_

#endif /* MJ808_LED_C_ */
