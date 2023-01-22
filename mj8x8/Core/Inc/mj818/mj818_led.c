#ifndef MJ818_LED_C_
#define MJ818_LED_C_

#include "main.h"
#if defined(MJ818_)	// if this particular device is active

#include "mj818\mj818.h"
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

	if(OCR_FRONT_LIGHT == 0 && OCR_REAR_LIGHT == 0)
		{
			// TODO - check timer stop in fade via CAN commands
			HAL_TIM_Base_Stop_IT(&htim14);  // stop the timer
			__HAL_RCC_TIM14_CLK_DISABLE();  // stop the clock
			__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock
			__HAL_RCC_TIM3_CLK_DISABLE();  // stop the clock
		}

	if(OCR_BRAKE_LIGHT == Device->led->led[Brake].ocr && OCR_REAR_LIGHT == Device->led->led[Rear].ocr)
		{
			HAL_TIM_Base_Stop_IT(&htim14);  // stop the timer
			__HAL_RCC_TIM14_CLK_DISABLE();  // stop the clock
		}
}

// set OCR value to fade to
static void _primitiveRearLED(uint8_t value)
{
	Device->led->led[Rear].ocr = value;  // set OCR value, the handler will do the rest
}

// set OCR value to fade to
static void _primitiveBrakeLED(uint8_t value)
{
	Device->led->led[Brake].ocr = value;	// set OCR value, the handler will do the rest
}

static inline void __componentLED_On(const uint8_t val)
{
	TIM_OC_InitTypeDef sConfigOC =
		{0};

	// Timer2 init - rear light PWM
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;  // scale by 1
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;	// up counting
	htim2.Init.Period = 99;  // count to 100
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;	// no division
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;	// no pre-load
	__HAL_RCC_TIM2_CLK_ENABLE();	// start the clock
	HAL_TIM_PWM_Init(&htim2);  // commit it

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = LED_OFF;  // 0 to 100% duty cycle in decimal numbers
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);  // commit it
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);  // start the timer

	// timer3 - brake light PWM
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 99;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	__HAL_RCC_TIM3_CLK_ENABLE();	// start the clock
	HAL_TIM_PWM_Init(&htim3);  // commit it

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = LED_OFF;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);  // start the timer

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
		{
			__componentLED_Off();
			return;
		}

	__HAL_RCC_TIM14_CLK_ENABLE();  // start the clock
	htim14.Instance->PSC = 799;  // reconfigure after peripheral was powered down
	htim14.Instance->ARR = 199;
	HAL_TIM_Base_Start_IT(&htim14);  // start the timer
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,  // assign pointer to LED array
	.public.Shine = &_componentLED,  // component part ("interface")
	.flags = 0  //
	};

static composite_led_t* _virtual_led_ctorMJ818()
{
	__LED.public.led[Rear].Shine = &_primitiveRearLED;  // LED-specific implementation
	__LED.public.led[Brake].Shine = &_primitiveBrakeLED;  // LED-specific implementation
	__LED.public.Handler = &_FadeHandler;  // timer-based periodic LED control function

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ818_

#endif /* MJ808_LED_C_ */
