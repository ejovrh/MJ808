#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include <stdlib.h>

#include "try/try.h"  // top-level mj8x8 object for consolidated behaviour code
#include "mj838/mj838.h"
#include "mj838/mj838_zerocross.c"  // concrete device-specific zero-cross functions
#include "mj838/autodrive.h"  // auto-drive detection functionality
#include "mj838/autocharge.h"  // automatic charger functionality - including load control

TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 2.5ms
TIM_HandleTypeDef htim16;  // Timer16 object - odometer & co. 1s
TIM_HandleTypeDef htim14;  // Timer14 object - power measurement time base - 10ms
TIM_HandleTypeDef htim2;  // Timer2 object - periodic frequency measurement of timer3 data - default 250ms
TIM_HandleTypeDef htim3;  // Timer3 object - input capture of zero-cross signal on rising edge

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

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock

	GPIO_InitStruct.Pin = TCAN334_Standby_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TCAN334_Standby_GPIO_Port, &GPIO_InitStruct);

	// GPIO EXTI0 mode - state change from idle to measurement mode
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // catch zero cross activity (idle to first impulse and rolling)
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = AppLoadFet_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;    // keep the load switch off
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(AppLoadFet_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = PowerMonitorPower_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // keep the power monitor off
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(PowerMonitorPower_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED_Reset_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // reset the LED driver
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_Reset_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LED2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

	// explicitly set pin states
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

#if GPIO_DEBUG_OUT
	GPIO_InitStruct.Pin = YellowTestPad_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(YellowTestPad_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = BlueTestPad_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(BlueTestPad_GPIO_Port, &GPIO_InitStruct);

	HAL_GPIO_WritePin(YellowTestPad_GPIO_Port, YellowTestPad_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(BlueTestPad_GPIO_Port, BlueTestPad_Pin, GPIO_PIN_RESET);
#endif
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

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

	// timer3 - input capture of zero-cross signal on rising edge
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = TIMER_PRESCALER;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = TIMER3_PERIOD;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

	HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);
	HAL_TIM_IC_Init(&htim3);
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

	sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
	sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
	sConfigIC.ICPrescaler = TIM_ICPSC_DIV8;
	sConfigIC.ICFilter = TIMER3_IC_FILTER;
	HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_3);

	// timer 2 - measurement interval of timer3 data - default 250ms
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = TIMER_PRESCALER;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = TIMER2_PERIOD;  // with above pre-scaler and a period of 2499, we have an 250ms interrupt period
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.RepetitionCounter = 0;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim2);
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

	// timer 14 - power measurement time base - 10ms
	htim14.Instance = TIM14;
	htim14.Init.Prescaler = TIMER_PRESCALER;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = TIMER14_PERIOD;  // with above pre-scaler and a period of 1, we have a 10ms period
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.RepetitionCounter = 0;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_ConfigClockSource(&htim14, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim14);
	HAL_TIMEx_MasterConfigSynchronization(&htim14, &sMasterConfig);

	// timer 16 - odometer & co. 1s
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = TIMER_PRESCALER;
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = TIMER16_PERIOD;  // with above pre-scaler and a period of 9999, we have a 1s period
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_ConfigClockSource(&htim16, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim16);
	HAL_TIMEx_MasterConfigSynchronization(&htim16, &sMasterConfig);

	// timer 17 - event handling - 2.5ms
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = TIMER_PRESCALER;
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = TIMER17_PERIOD;  // with above pre-scaler and a period of 24, we have an 2.5ms interrupt period
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.RepetitionCounter = 0;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	HAL_TIM_ConfigClockSource(&htim17, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim17);
	HAL_TIMEx_MasterConfigSynchronization(&htim17, &sMasterConfig);
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	if(timer->Instance == TIM3)  // ZeroCross frequency measurement
		__HAL_RCC_TIM3_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM2)  // frequency measurement timer
		__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM14)  // power measurement time base - 10ms
		__HAL_RCC_TIM14_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM16)  // odometer & co. 1s
		__HAL_RCC_TIM16_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM17)	// event handling
		__HAL_RCC_TIM17_CLK_DISABLE();  // stop the clock
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	if(timer->Instance == TIM3)  // input capture ZeroCross frequency measurement
		{
			TIM_IC_InitTypeDef sConfigIC =
				{0};

			__HAL_RCC_TIM3_CLK_ENABLE();  // start the clock

			timer->Instance->PSC = 0;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER3_PERIOD;

			HAL_TIM_IC_Init(&htim3);	// activate input capture

			sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
			sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
			sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
			sConfigIC.ICFilter = TIMER3_IC_FILTER;
			HAL_TIM_IC_ConfigChannel(&htim3, &sConfigIC, TIM_CHANNEL_3);

			return;  // early return so that the timer isn't started with interrupt enabled
		}

	if(timer->Instance == TIM2)  // measurement/calculation interval of timer2 data - default 250ms
		{
			__HAL_RCC_TIM2_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER2_PERIOD;
		}

	if(timer->Instance == TIM14)  // power measurement time base - 10ms
		{
			__HAL_RCC_TIM14_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER14_PERIOD;
		}

	if(timer->Instance == TIM16)  // odometer & co. 1s
		{
			__HAL_RCC_TIM16_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER16_PERIOD;
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
	;  // TODO - mj838 _PreSleep switchover of capacitor control to standstill
}

// device-specific pre stop
static inline void _PreStop(void)
{
	;  // TODO - mj838 _PreStop switchover of capacitor control to standstill
}

// device-specific constructor
void mj838_ctor(void)
{
	// general device non-specific low-level hardware init & config
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor(mj838);	// call base class constructor & initialize own SID

	__Device.public.activity = (mj838_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type

	_GPIOInit();	// initialize device-specific GPIOs
	_TimerInit();  // initialize Timers

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument

	__Device.public.mj8x8->i2c = i2c_ctor(I2C_SDA_Pin, I2C_SCL_Pin, I2C_GPIO_Port);  // call I2C constructor

	__Device.public.mj8x8->EmptyBusOperation = Try->EmptyBusOperation;  // override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = Try->PopulatedBusOperation;  // implements device-specific operation depending on bus activity
	__Device.public.mj8x8->PreSleep = &_PreSleep;  // implements the derived object prepare to sleep
	__Device.public.mj8x8->PreStop = &_PreStop;  // implements the derived object prepare to stop

	EventHandler->fpointer = Try->EventHandler;  // implements event hander for this device

	// application part
	for(IRQn_Type irq = NonMaskableInt_IRQn; irq <= USB_IRQn; irq++)
		NVIC_ClearPendingIRQ(irq);

	__enable_irq();  // PARTLY!!! enable interrupts -- essential for I2C

	__Device.public.FeRAM = mb85rc_ctor();  // tie in FeRAM object
	__Device.public.Humidity = sht40_ctor();  // tie in humidity sensor object

	__Device.public.ZeroCross = zerocross_ctor();  // call zero-cross constructor
	__Device.public.AutoDrive = autodrive_ctor();  // call AutoDrive constructor
	__Device.public.AutoCharge = autocharge_ctor();  // call AutoCharge constructor

	__disable_irq();	// disable interrupts for the remainder of initialization

	// interrupt init
	HAL_NVIC_SetPriority(TIM14_IRQn, 3, 0);  // power measurement timer
	HAL_NVIC_EnableIRQ(TIM14_IRQn);

	HAL_NVIC_SetPriority(TIM16_IRQn, 3, 0);  // odometer & co. timer
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

	HAL_NVIC_SetPriority(TIM17_IRQn, 3, 0);  // event handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM17_IRQn);

	// system interrupts
	// normally defined in mj8x8.c mj8x8_ctor() - #define USE_I2C 1 controls it there
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2, 0);  // heartbeat timer
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

	// normally activated in can.c _CANInit() - #define USE_I2C 1 controls it there
	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}

// device-specific interrupt handlers
// EXTI0 ISR - standstill to first impulse: wakeup and activate zero-cross functionality
void EXTI0_1_IRQHandler(void)
{
#if WIPE_FRAM
	__Device.public.FeRAM->Write(0, ODOMETER_ADDR);

	__disable_irq();
	while(1)
		;
#endif

	Device->mj8x8->StartCoreTimer();  // start core timer

	if(__HAL_GPIO_EXTI_GET_IT(ZeroCross_Pin))  // interrupt source detection
		{
			Device->ZeroCross->Start();  // start zero-cross detection
			HAL_GPIO_EXTI_IRQHandler(ZeroCross_Pin);  // service the interrupt
		}
}

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
