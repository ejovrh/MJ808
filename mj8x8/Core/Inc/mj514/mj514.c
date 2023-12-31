#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "try\try.h"	// top-level mj8x8 object for consolidated behaviour code
#include "mj514\mj514.h"

// FIXME - define timer objects
TIM_HandleTypeDef htim2;  // rotary encoder time base
TIM_HandleTypeDef htim3;  // rotary encoder handling
TIM_HandleTypeDef htim17;  // event handling - 2.5ms

typedef struct	// mj514_t actual
{
	mj514_t public;  // public struct
} __mj514_t;

static __mj514_t __Device __attribute__ ((section (".data")));  // preallocate __Device object in .data

// GPIO init - device specific
static inline void _GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = TCAN334_Standby_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TCAN334_Standby_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Rotary_A_Pin | Rotary_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
	HAL_GPIO_Init(Rotary_A_GPIO_Port, &GPIO_InitStruct);

	// FIXME - define device GPIOs
	GPIO_InitStruct.Pin = Motor_Direction_Up_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Motor_Direction_Up_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Motor_Direction_Down_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Motor_Direction_Down_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = FeRAM_WP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(FeRAM_WP_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - device specific
static inline void _TimerInit(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};

	TIM_MasterConfigTypeDef sMasterConfig =
		{0};

	// timer2 - rotary encoder time base
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = TIMER_PRESCALER;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = TIMER2_PERIOD;  // with above pre-scaler and a period of 99, we have an 10ms interrupt frequency
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.RepetitionCounter = 0;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

	// timer3 - rotary encoder handling
	htim3.Instance = TIM3;	// rotary encoder handling
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 0xFF;  // TODO - use this: TIMER3_PERIOD;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

	// FIXME - define device timers
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	if(timer->Instance == TIM2)  // rotary encoder time base
		__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM3)  // rotary encoder handling
		__HAL_RCC_TIM3_CLK_DISABLE();  // stop the clock

	// FIXME - define timer stop
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	if(timer->Instance == TIM2)  // rotary encoder time base
		{
			__HAL_RCC_TIM2_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER2_PERIOD;
		}

	if(timer->Instance == TIM3)  // rotary encoder handling
		{
			TIM_Encoder_InitTypeDef sConfig =
				{0};

			sConfig.EncoderMode = TIM_ENCODERMODE_TI12;  // TODO: test TIM_ENCODERMODE_TI1, TIM_ENCODERMODE_TI2, TIM_ENCODERMODE_TI12
			sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC1Filter = TIMER3_IC1_FILTER;

			sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
			sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
			sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
			sConfig.IC2Filter = TIMER3_IC2_FILTER;

			__HAL_RCC_TIM3_CLK_ENABLE();
			HAL_TIM_Encoder_Init(&htim3, &sConfig);
			HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
		}

	// FIXME - define timer start
	;

	HAL_TIM_Base_Start_IT(timer);  // start the timer
}

// device-specific pre sleep
static inline void _PreSleep(void)
{
	;
}

// device-specific pre stop
static inline void _PreStop(void)
{
	;
}

// device-specific constructor
void mj514_ctor(void)
{
	// general device non-specific low-level hardware init & config
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor(mj514);	// call base class constructor & initialize own SID

	__Device.public.activity = (mj514_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type

	_GPIOInit();	// initialize device-specific GPIOs
	_TimerInit();  // initialize Timers

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument
	__Device.public.gear = gear_ctor();		// electronic gear shifting unit

	__Device.public.mj8x8->EmptyBusOperation = Try->EmptyBusOperation;  // override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = Try->PopulatedBusOperation;  // implements device-specific operation depending on bus activity
	__Device.public.mj8x8->PreSleep = &_PreSleep;  // implements the derived object prepare to sleep
	__Device.public.mj8x8->PreStop = &_PreStop;  // implements the derived object prepare to stop

//	EventHandler->fpointer = Try->EventHandler;  // implements event hander for this device

// interrupt init
	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

// FIXME - define // interrupt init
}

// device-specific interrupt handlers
// FIXME - define device-specific interrupt handlers
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj514_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // mj514_
