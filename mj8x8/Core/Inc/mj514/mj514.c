#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "try/try.h"	// top-level mj8x8 object for consolidated behaviour code
#include "mj514/mj514.h"

#include "motor.h"	// TODO - remove once development is complete

TIM_HandleTypeDef htim2;	// motor control PWM signal generation
#if USE_AS5601_PULSE
TIM_HandleTypeDef htim3;  // timer in encoder mode for rotation detection
#endif
TIM_HandleTypeDef htim16;  // motor time base - 1ms
//TIM_HandleTypeDef htim17;  //

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

#if USE_AS5601_PULSE
	GPIO_InitStruct.Pin = Rotary_A_Pin | Rotary_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
	HAL_GPIO_Init(Rotary_A_GPIO_Port, &GPIO_InitStruct);
#endif

	GPIO_InitStruct.Pin = Motor_IN1_Pin | Motor_IN2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
	HAL_GPIO_Init(Motor_IN1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Motor_FLT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(Motor_FLT_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Motor_SLP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Motor_SLP_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Motor_12V0_SHDN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Motor_12V0_SHDN_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Motor_IPROP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(Motor_IPROP_GPIO_Port, &GPIO_InitStruct);

	__HAL_RCC_GPIOF_CLK_ENABLE();  // TODO - remove debug pin when done
	GPIO_InitStruct.Pin = Debug0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Debug0_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Debug1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Debug1_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - device specific
static inline void _TimerInit(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};

	TIM_MasterConfigTypeDef sMasterConfig =
		{0};

	TIM_OC_InitTypeDef sConfigOC =
		{0};

#if USE_AS5601_PULSE
	TIM_Encoder_InitTypeDef sConfig =
		{0};
#endif

	TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig =
		{0};

	// timer2 - motor control PWM signal generation
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 0;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = TIMER2_PERIOD;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim2);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	HAL_TIM_PWM_Init(&htim2);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = MOTOR_OFF;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1);

	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = MOTOR_OFF;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);

#if USE_AS5601_PULSE
	// timer3 - rotary encoder handling
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 65535;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
	sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC1Filter = TIMER3_IC1_FILTER;
	sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
	sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
	sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
	sConfig.IC2Filter = TIMER3_IC2_FILTER;
	HAL_TIM_Encoder_Init(&htim3, &sConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);
#endif

	// timer16 - rotary encoder time base - 50ms
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = TIMER_PRESCALER;
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = TIMER16_PERIOD;  // with above pre-scaler and a period of 9, we have an 1ms interrupt frequency
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	HAL_TIM_Base_Init(&htim16);

	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
	sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
	HAL_TIM_OC_ConfigChannel(&htim16, &sConfigOC, TIM_CHANNEL_1);

	sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
	sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
	sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
	sBreakDeadTimeConfig.DeadTime = 0;
	sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
	sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
	sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
	HAL_TIMEx_ConfigBreakDeadTime(&htim16, &sBreakDeadTimeConfig);

//	// timer17 - ADC time base - 10ms
//	htim17.Instance = TIM17;
//	htim17.Init.Prescaler = TIMER_PRESCALER;
//	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
//	htim17.Init.Period = TIMER17_PERIOD;
//	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
//	htim17.Init.RepetitionCounter = 0;
//	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
//	HAL_TIM_Base_Init(&htim17);
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	if(timer->Instance == TIM2)  // motor control PWM signal generation
		__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM16)  // motor time base - 1ms
		{
			__HAL_RCC_TIM16_CLK_DISABLE();  // stop the clock
#if USE_AS5601_PULSE
			__HAL_RCC_TIM3_CLK_DISABLE();  // stop the clock - is co-dependant on timer16
#endif
		}

//	if(timer->Instance == TIM17)  // ADC time base - 10ms
//		__HAL_RCC_TIM17_CLK_DISABLE();  // stop the clock
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	if(timer->Instance == TIM2)  // motor control PWM signal generation
		{
			static TIM_OC_InitTypeDef sConfigOC =
				{0};

			sConfigOC.OCMode = TIM_OCMODE_PWM1;
			sConfigOC.Pulse = MOTOR_OFF;
			sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
			sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

			__HAL_RCC_TIM2_CLK_ENABLE();  // start the clock
			HAL_TIM_PWM_Init(timer);  //
			HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_1);  //
			HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_2);  //
			return;
		}

	if(timer->Instance == TIM16)  // motor time base - 1ms
		{
#if USE_AS5601_PULSE
			// timer 3 is co-dependant on timer16 - see motor.c

			// timer3 in encoder mode for rotation detection
			TIM_Encoder_InitTypeDef sConfig =
				{0};

			sConfig.EncoderMode = TIM_ENCODERMODE_TI12;

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
			HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);  // start the timer in encoder mode
			_StartTimer(&htim3);  // dependency - start timer3 in encoder mode so that rotation direction can be detected
#endif

			// timer16 as a normal 1ms periodic timer
			__HAL_RCC_TIM16_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER16_PERIOD;
		}

//	if(timer->Instance == TIM17)  // ADC time base - 10ms
//		{
//			__HAL_RCC_TIM17_CLK_ENABLE();  // start the clock
//			timer->Instance->PSC = TIMER_PRESCALER;  // reconfigure after peripheral was powered down
//			timer->Instance->ARR = TIMER17_PERIOD;
//		}

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

	__Device.public.mj8x8->i2c = i2c_ctor(I2C_SDA_Pin, I2C_SCL_Pin, I2C_GPIO_Port);  // call I2C constructor

	__Device.public.mj8x8->EmptyBusOperation = Try->EmptyBusOperation;  // override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = Try->PopulatedBusOperation;  // implements device-specific operation depending on bus activity
	__Device.public.mj8x8->PreSleep = &_PreSleep;  // implements the derived object prepare to sleep
	__Device.public.mj8x8->PreStop = &_PreStop;  // implements the derived object prepare to stop

//	EventHandler->fpointer = Try->EventHandler;  // implements event hander for this device

	// application part
	__Device.public.gear = gear_ctor();		// electronic gear shifting unit

	// interrupt init
	// application interrupts
#if USE_AS5601_PULSE
	HAL_NVIC_SetPriority(TIM3_IRQn, 1, 0);	// rotary encoder handling
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
#endif

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);  // DMA interrupt - ADC
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	HAL_NVIC_SetPriority(TIM16_IRQn, 2, 0);  // motor time base - 1ms
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

	// system interrupts
	// normally defined in mj8x8.c mj8x8_ctor() - #define USE_I2C 1 controls it there
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2, 0);  // heartbeat timer
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

	// normally activated in can.c _CANInit() - #define USE_I2C 1 controls it there
	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj514_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // mj514_
