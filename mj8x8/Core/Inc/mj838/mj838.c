#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "try\try.h"	// top-level mj8x8 object for consolidated behaviour code
#include "mj838\mj838.h"
#include "mj838\mj838_zerocross.c"	// concrete device-specific zero-cross functions

#include "mj838\autodrive.h"	// auto-drive detection functionality

TIM_HandleTypeDef htim17;  // event handling - 2.5ms
TIM_HandleTypeDef htim2;  // input capture of zero-cross signal on rising edge
TIM_HandleTypeDef htim3;  // measurement interval of timer2 data - default 250ms

typedef struct	// mj838_t actual
{
	mj838_t public;  // public struct
} __mj838_t;

static __mj838_t __Device __attribute__ ((section (".data")));  // preallocate __Device object in .data

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

	// GPIO EXTI0 mode
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  // catch zero cross activity (idle to first impulse and rolling)
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - device specific
static inline void _TimerInit(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};

	TIM_MasterConfigTypeDef sMasterConfig =
		{0};

	TIM_IC_InitTypeDef sConfigIC =
		{0};

	// input capture of zero-cross signal on rising edge
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = TIMER_PRESCALER;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = TIMER2_PERIOD;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	HAL_TIM_IC_Init(&htim2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
	sConfigIC.ICFilter = 0x0F;  // odd filter config
	HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);

	// timer 3 - measurement interval of timer2 data - default 250ms
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = TIMER_PRESCALER;  // 8MHz / 799+1 = 10kHz update rate
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = TIMER3_PERIOD;  // with above pre-scaler and a period of 24, we have an 2.5ms interrupt frequency
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.RepetitionCounter = 0;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim3);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

	// timer 17 - event handling - 2.5ms
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = TIMER_PRESCALER;  // 8MHz / 799+1 = 10kHz update rate
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = TIMER17_PERIOD;  // with above pre-scaler and a period of 24, we have an 2.5ms interrupt frequency
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.RepetitionCounter = 0;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim17, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim17);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim17, &sMasterConfig);
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	if(timer->Instance == TIM2)  // ZeroCross frequency measurement
		__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM3)  // frequency measurement timer
		__HAL_RCC_TIM3_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM17)	// event handling
		__HAL_RCC_TIM17_CLK_DISABLE();  // stop the clock
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	if(timer->Instance == TIM2)  // ZeroCross frequency measurement
		{
			TIM_IC_InitTypeDef sConfigIC =
				{0};

			__HAL_RCC_TIM2_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER2_PERIOD;

			HAL_TIM_IC_Init(&htim2);	// activate input capture

			sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
			sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
			sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
			sConfigIC.ICFilter = 0x0F;  // odd filter config
			HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1);

			return;  // get out since we don't need HAL_TIM_Base_Start_IT() as other timers do
		}

	if(timer->Instance == TIM3)  // event handling
		{
			__HAL_RCC_TIM3_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER3_PERIOD;
		}

	if(timer->Instance == TIM17)	// event handling
		{
			__HAL_RCC_TIM17_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER17_PERIOD;
		}

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
void mj838_ctor(void)
{
	// general device non-specific low-level hardware init & config
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor(CANID_MJ838);	// call base class constructor & initialize own SID

	__Device.public.activity = (mj838_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type

	_GPIOInit();	// initialize device-specific GPIOs
	_TimerInit();  // initialize Timers

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument
	__Device.public.ZeroCross = zerocross_ctor();  // call zero-crossconstructor
	__Device.public.AutoDrive = autodrive_ctor();  // call AutoDrive constructor

	__Device.public.mj8x8->EmptyBusOperation = Try->EmptyBusOperation;  // override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = Try->PopulatedBusOperation;  // implements device-specific operation depending on bus activity
	__Device.public.mj8x8->PreSleep = &_PreSleep;  // implements the derived object prepare to sleep
	__Device.public.mj8x8->PreStop = &_PreStop;  // implements the derived object prepare to stop

	EventHandler->fpointer = Try->EventHandler;  // implements event hander for this device

// interrupt init
	HAL_NVIC_SetPriority(TIM17_IRQn, 3, 0);  // event handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM17_IRQn);
}

// device-specific interrupt handlers
// timer 17 ISR - 10ms interrupt - event handler (activated on demand)
void TIM17_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim17);  // service the interrupt
	EventHandler->HandleEvent();	// execute the event handling function with argument taken from case table
}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj838_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ838_
