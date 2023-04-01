#include "main.h"

#if defined(MJ818_)	// if this particular device is active

#include "try/try.h"	//
#include "mj818\mj818.h"
#include "mj818\mj818_led.c"	// concrete device-specific LED functions

TIM_HandleTypeDef htim2;	// rear light PWM on channel 1
TIM_HandleTypeDef htim3;	// brake light PWM on channel 4
TIM_HandleTypeDef htim14;  // Timer14 object - LED handling - 20ms

TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 10ms - FIXME - should not be here

// TODO - these shouldn't really be here...
static TIM_ClockConfigTypeDef sClockSourceConfig =
	{0};

static TIM_MasterConfigTypeDef sMasterConfig =
	{0};

static TIM_OC_InitTypeDef sConfigOC =
	{0};

typedef struct	// mj818_t actual
{
	mj818_t public;  // public struct
} __mj818_t;

static __mj818_t __Device __attribute__ ((section (".data")));	// preallocate __Device object in .data

// defines device operation on empty bus
void _EmptyBusOperation(void)
{
	if(REAR_LIGHT_CCR == 0)  // run once: check that e.g. rear light is off (which it is on a lonely power on)
		__Device.public.led->Shine(10);  // operate on component part
}

// dispatches CAN messages to appropriate sub-component on device
void _PopulatedBusOperation(message_handler_t *const in_handler)
{
	branchtable_event(in_handler->GetMessage());
}

// GPIO init - device specific
static inline void _GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock

	GPIO_InitStruct.Pin = TCAN334_Standby_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TCAN334_Standby_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = BrakeLED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // activate pulldown resistor
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;	// alternate function2: timer3 channel 4 PWM output
	HAL_GPIO_Init(BrakeLED_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RearLED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // activate pulldown resistor
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;	// alternate function2: timer2 channel 1 PWM output
	HAL_GPIO_Init(RearLED_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - device specific
static inline void _TimerInit(void)
{
	// Timer2 init - rear light PWM
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;  // scale by 1
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;	// up counting
	htim2.Init.Period = 99;  // count to 100
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;	// no division
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;	// no pre-load

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;	// we shall run from our internal oscillator
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);  // commit it
	HAL_TIM_PWM_Init(&htim2);  // commit it

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);	// commit it

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

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;	// we shall run from our internal oscillator
	HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);  // commit it
	HAL_TIM_PWM_Init(&htim3);  // commit it

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);	// commit it

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = LED_OFF;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_4);  // start the timer

	// timer14 - LED handling - 20ms
	htim14.Instance = TIM14;
	htim14.Init.Prescaler = TIMER14_PRESCALER;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = TIMER14_PERIOD;
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.RepetitionCounter = 0;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim14, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim14);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim14, &sMasterConfig);
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	if(timer->Instance == TIM2)  // led handling
		__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM3)  // led handling
		__HAL_RCC_TIM3_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM14)	// led handling
		__HAL_RCC_TIM14_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM16)	// button handling
		__HAL_RCC_TIM16_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM17)	// event handling
		__HAL_RCC_TIM17_CLK_DISABLE();  // stop the clock
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	if(timer->Instance == TIM2)  // front LED PWM
		{
			// Timer2 init - rear light PWM
			__HAL_RCC_TIM2_CLK_ENABLE();	// start the clock
			HAL_TIM_PWM_Init(timer);  //
			HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_1);  //
			HAL_TIM_PWM_Start(timer, TIM_CHANNEL_1);  // start the timer
			return;
		}

	if(timer->Instance == TIM3)  // brake LED PWM
		{
			// timer3 - brake light PWM
			__HAL_RCC_TIM3_CLK_ENABLE();	// start the clock
			HAL_TIM_PWM_Init(timer);  //
			HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_4);
			HAL_TIM_PWM_Start(timer, TIM_CHANNEL_4);  // start the timer
			return;
		}

	if(timer->Instance == TIM14)	// led handling
		{
			__HAL_RCC_TIM14_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER14_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER14_PERIOD;
		}

	if(timer->Instance == TIM16)	// button handling
		{
			__HAL_RCC_TIM16_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = 799;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = 249;
		}

	if(timer->Instance == TIM17)	// event handling
		{
			__HAL_RCC_TIM17_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = 799;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = 24;
		}

	HAL_TIM_Base_Start_IT(timer);  // start the timer
}

void mj818_ctor(void)
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor(MJ818);	// call base class constructor & initialize own SID

	__Device.public.activity = (mj818_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type

	_TimerInit();  // initialize Timers
	_GPIOInit();	// initialize device-specific GPIOs

	__Device.public.led = _virtual_led_ctorMJ818();  // call virtual constructor & tie in object addresses

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument

	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperation;  // override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperation;  // implements device-specific operation depending on bus activity

	// interrupt init
	HAL_NVIC_SetPriority(TIM14_IRQn, 0, 0);  // charlieplexed LED handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM14_IRQn);
}

// device-specific interrupt handlers
// timer 14 ISR - 20ms interrupt - LED handling (activated on demand)
void TIM14_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim14);  // service the interrupt
	Device->led->Handler();  // handles front LED fading
}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj818_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ818_
