#include "main.h"
#if defined(MJ818_)	// if this particular device is active

#include "mj818\mj818.h"
#include "mj818\mj818_led.c"	// concrete device-specific LED functions

typedef struct	// mj818_t actual
{
	mj818_t public;  // public struct
} __mj818_t;

static __mj818_t __Device __attribute__ ((section (".data")));	// preallocate __Device object in .data

// defines device operation on empty bus
void _EmptyBusOperationMJ818(void)
{
	if(OCR_REAR_LIGHT == 00)	// run once
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
	TIM_HandleTypeDef htim2;	// rear light PWM on channel 1
	TIM_HandleTypeDef htim3;	// brake light PWM on channel 4

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
	__HAL_RCC_TIM2_CLK_ENABLE();	// start the clock

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
	__HAL_RCC_TIM3_CLK_ENABLE();	// start the clock

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

	// interrupt init
	// none
}

// device-specific interrupt handlers
// none
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj818_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ818_
