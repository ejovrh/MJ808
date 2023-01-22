#include "main.h"
#if defined(MJ818_)	// if this particular device is active

#include "mj818\mj818.h"
#include "mj818\mj818_led.c"	// concrete device-specific LED functions

#define TIM14_IRQn 19	// FIXME - EXTI0_1_IRQn should be included somehow, but isn't..

TIM_HandleTypeDef htim2;	// rear light PWM on channel 1
TIM_HandleTypeDef htim3;	// brake light PWM on channel 4
TIM_HandleTypeDef htim14;  // Timer14 object - LED handling - 20ms

TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 10ms - FIXME - should not be here

typedef struct	// mj818_t actual
{
	mj818_t public;  // public struct
} __mj818_t;

static __mj818_t __Device __attribute__ ((section (".data")));	// preallocate __Device object in .data

// defines device operation on empty bus
void _EmptyBusOperationMJ818(void)
{
	if(OCR_REAR_LIGHT == 0)  // run once
		__Device.public.led->Shine(10);  // operate on component part
}

// dispatches CAN messages to appropriate sub-component on device
void _PopulatedBusOperationMJ818(message_handler_t *const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();  // CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if(msg->COMMAND== (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT))  // rear positional light
		{
			__Device.public.led->led[Rear].Shine(msg->ARGUMENT);	// fade rear light to CAN msg. argument value
			return;
		}

	if(msg->COMMAND== (CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT))	// brake light
		{
			if(msg->ARGUMENT > OCR_MAX_BRAKE_LIGHT)
			OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
			else
			OCR_BRAKE_LIGHT = msg->ARGUMENT;
		}
}

// GPIO init - device specific
static inline void _GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock

	HAL_GPIO_WritePin(TCAN334_Shutdown_GPIO_Port, TCAN334_Shutdown_Pin, GPIO_PIN_SET);	// high - put device into shutdown
	HAL_GPIO_WritePin(TCAN334_Standby_GPIO_Port, TCAN334_Standby_Pin, GPIO_PIN_SET);	// high - put device into standby

	GPIO_InitStruct.Pin = TCAN334_Shutdown_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TCAN334_Shutdown_GPIO_Port, &GPIO_InitStruct);

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
	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};
	TIM_MasterConfigTypeDef sMasterConfig =
		{0};
	TIM_OC_InitTypeDef sConfigOC =
		{0};

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
	htim14.Init.Prescaler = 799;  // 8MHz / 799+1 = 10kHz update rate
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 199;  // with above pre-scaler and a period of 19, we have an 2ms interrupt frequency
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

// interrupt extension, triggered by timer 1 ISR - 2.5ms interrupt in mj8x8
void _SystemInterrupt(void)
{
	;
}

void mj818_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B));	// call base class constructor & initialize own SID

	_TimerInit();  // initialize Timers
	_GPIOInit();	// initialize device-specific GPIOs

	__Device.public.led = _virtual_led_ctorMJ818();  // call virtual constructor & tie in object addresses

	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperationMJ818;	// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ818;	// implements device-specific operation depending on bus activity
	__Device.public.mj8x8->SystemInterrupt = &_SystemInterrupt;  // implement device-specific system interrupt code

	// interrupt init
	HAL_NVIC_SetPriority(TIM14_IRQn, 0, 0);  // charlieplexed LED handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM14_IRQn);

	__enable_irq();  // enable interrupts
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
