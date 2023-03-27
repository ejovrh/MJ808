#ifndef MJ808_LED_C_
#define MJ808_LED_C_

#if defined(MJ808_)	// if this particular device is active

#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

extern TIM_HandleTypeDef htim2;  // front light PWM on channel 2
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
volatile static uint8_t i = 0;	// front

// called indirectly by timer1 (_SystemInterrupt()), handles the fading
static void _MacNamaraFadeHandler(void)
{
	if(FRONT_LIGHT_CCR < Device->led->led[Front].ocr)  // fade up
		{
			FRONT_LIGHT_CCR = _fade_fransfer[i++];

			if(i == Device->led->led[Front].ocr)
				Device->StopTimer(&htim14);  // stop the timer
		}

	if(FRONT_LIGHT_CCR > Device->led->led[Front].ocr)  // fade down
		{
			FRONT_LIGHT_CCR = _fade_fransfer[--i];

			if(FRONT_LIGHT_CCR == 0)
				{
					Device->StopTimer(&htim14);  // stop the timer
					Device->StopTimer(&htim2);  // stop the timer
					Device->activity->FrontLightOn = 0;	// mark inactivity
				}
		}
}

// does high beam on/off without fading
static void _HighBeam(const uint8_t value)
{
	static uint8_t OldOCR;	// holds previous OCR value

	if(value == 200)	// high beam off command
		{
			if (Device->activity->FrontLightOn)	// if front light is on
				{
					FRONT_LIGHT_CCR = OldOCR;	// restore original OCR
					return;
				}
			else
				{	// FIXME - _HighBeam off - sometimes light stays on (CAN seems ok)
					FRONT_LIGHT_CCR = 0;	// turn off light
					Device->StopTimer(&htim2);  // stop the timer - front light PWM
					Device->StartTimer(&htim2);
					Device->StopTimer(&htim2);
					Device->activity->HighBeamOn = 0;	// mark inactivity
					return;
				}
		}

	if(value > 200)	// high beam on command
		{
			Device->activity->HighBeamOn = 1;	// mark activity

			if (Device->activity->FrontLightOn)	// if front light is on
				OldOCR = FRONT_LIGHT_CCR;	// store original OCR value
			else
				Device->StartTimer(&htim2);  // start the timer - front light PWM

			FRONT_LIGHT_CCR = 100;	// high beam on
			return;
		}
}

// set OCR value to fade to
static inline void _primitiveFrontLED(const uint8_t value)
{
	if(value >= 200)	// special case for high beam
		{
			_HighBeam(value);
			return;
		}

	Device->StartTimer(&htim14);  // start the timer - LED handling
	__HAL_TIM_DISABLE_IT(&htim14, TIM_IT_UPDATE);	// disable interrupts until ocr is set (timer14's ISR will otherwise kill it very soon)

	if(value)
		{
			Device->StartTimer(&htim2);  // start the timer - front light PWM
			Device->activity->FrontLightOn = 1;
		}

	Device->led->led[Front].ocr = value;	// set OCR value, the handler will do the rest
	__HAL_TIM_ENABLE_IT(&htim14, TIM_IT_UPDATE);	// start timer
}

// concrete utility LED handling function
static void _primitiveUtilityLED(uint8_t in_val)
{
	uint8_t led = 0;	// holds the pin of the LED: D0 - green (default), D1 - red

	if(in_val & _BV(B3))	// if the 4th bit is set, the command is for a red led, otherwise it is green
		led = 1;	// red

	// the led variable is relevant for bit-shifting, since the red and green LEDs are pin-wise next door neighbours;
	//	RedLED_Pin shifted left by one is the green LED

	in_val &= 7;	// clear everything except B2:0, which is the blink count (1-6)

	if(in_val == 0x00)	// B3:B0 is 0 - turn off
		{
			HAL_GPIO_WritePin(GPIOB, (RedLED_Pin << led), GPIO_PIN_SET);  // set high to turn off
			return;
		}

	if(in_val == 0x07)	// B3:B0 is 7 - turn on
		{
			HAL_GPIO_WritePin(GPIOB, (RedLED_Pin << led), GPIO_PIN_RESET);  // set low to turn on
			return;
		}

	while(in_val--)  // blink loop
		{
			HAL_Delay(BLINK_DELAY);  // TODO - get rid of blocking HAL_Delay();
			HAL_GPIO_TogglePin(GPIOB, (RedLED_Pin << led));  // toggle the led pin
			HAL_Delay(BLINK_DELAY);  // TODO - get rid of blocking HAL_Delay();
			HAL_GPIO_TogglePin(GPIOB, (RedLED_Pin << led));  // toggle the led pin
		}
}

// turns whole device ON (via pushbutton)
static void __componentLED_On(void)
{
	Device->led->led[Utility].Shine(UTIL_LED_GREEN_ON);  // green LED on
	Device->led->led[Front].Shine(75);  // front light on - low key; gets overwritten by LU command, since it comes in a bit later
}

// turns whole device OFF (via pushbutton)
static void __componentLED_Off(void)
{
	Device->led->led[Utility].Shine(UTIL_LED_GREEN_OFF);	// green LED off
	Device->led->led[Front].Shine(0);  // front light off
}

// delegates operations from LED component downwards to LED leaves
static void _componentLED(const uint8_t val)
{
	if(val)  // true - on, false - off
		__componentLED_On();	// delegate indirectly to the leaves
	else
		__componentLED_Off();
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,  // assign pointer to LED array
	.public.Shine = &_componentLED,  // component part ("interface")
	.flags = 0	//
	};

// implementation of virtual constructor for LEDs
composite_led_t* _virtual_led_ctorMJ808()
{
	__LED.public.led[Utility].Shine = &_primitiveUtilityLED;  // LED-specific implementation
	__LED.public.led[Utility].ocr = 0;	// TODO - not needed yet but it has potential...
	__LED.public.led[Front].Shine = &_primitiveFrontLED;  // LED-specific implementation
	__LED.public.led[Front].ocr = 0;	// is already at 0, but nevertheless
	__LED.public.Handler = &_MacNamaraFadeHandler;  // timer-based periodic LED control function

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ808_

#endif /* MJ808_LED_C_ */
