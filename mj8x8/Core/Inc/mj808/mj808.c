#include "main.h"
#if defined(MJ808_)	// if this particular device is active

#include "mj808\mj808.h"
#include "mj808\mj808_led.c"	// concrete device-specific LED functions
#include "mj808\mj808_button.c"	// concrete device-specific button functions

#define EXTI0_1_IRQn 5	// FIXME - EXTI0_1_IRQn should be included somehow, but isnt..

typedef struct	// mj808_t actual
{
	mj808_t public;  // public struct
} __mj808_t;

static __mj808_t __Device __attribute__ ((section (".data")));  // preallocate __Device object in .data

volatile uint8_t state;  // TODO - get rid of this by means of implementing a proper button handler

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void _event_execution_function_mj808(const uint8_t val)
{
	switch(val)
		{
		case 0x01:	// button error: - do the error thing
			Device->led->Shine(0);
			EventHandler->UnSetEvent(val);
			return;

		case 0x02:	// button hold
			Device->led->Shine(Device->button->button[Center].Hold);
			EventHandler->UnSetEvent(val);
			break;

			// next cases: 0x08, 0x16, etc.

		default:	// 0x00
			EventHandler->UnSetEvent(val);
			return;
		}
}

// received MsgHandler object and passes
void _PopulatedBusOperationMJ808(message_handler_t *const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();  // CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if((msg->COMMAND& CMND_UTIL_LED) == CMND_UTIL_LED)	// utility LED command
		{
			__Device.public.led->led[Utility].Shine(msg->COMMAND);	// glowy thingy
			return;
		}

	if(msg->COMMAND== ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT) )	// front positional light - low beam
		{
			// CHECKME - does it work?
			__Device.public.led->led[Front].Shine(msg->ARGUMENT);
			// TODO - access via object
//		_wrapper_fade_mj808(msg->ARGUMENT);	// fade front light to CAN msg. argument value
			return;
		}

	if(msg->COMMAND== ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) )  // front positional light - high beam
		{
			// TODO - implement timer based safeguard when OCR > OCR_MAX
			if (msg->ARGUMENT > OCR_MAX_FRONT_LIGHT)// safeguard against too high a value (heating of MOSFet)
			OCR_FRONT_LIGHT = OCR_MAX_FRONT_LIGHT;
			else
			OCR_FRONT_LIGHT = msg->ARGUMENT;
			return;
		}
}

// GPIO init - device specific
static inline void _GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock

	HAL_GPIO_WritePin(GPIOB, RedLED_Pin, GPIO_PIN_SET);  //	high - LED off
	HAL_GPIO_WritePin(GPIOB, GreenLED_Pin, GPIO_PIN_SET);  // high - LED off
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

	GPIO_InitStruct.Pin = Pushbutton_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(Pushbutton_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = RedLED_Pin | GreenLED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = FrontLED_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;  // activate pulldown resistor
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;	// alternate function2: timer2 channel 2 PWM output
	HAL_GPIO_Init(FrontLED_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - device specific
static inline void _TimerInit(void)
{
	TIM_HandleTypeDef htim2;	// front light PWM on channel 2

	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};
	TIM_MasterConfigTypeDef sMasterConfig =
		{0};
	TIM_OC_InitTypeDef sConfigOC =
		{0};

	// Timer2 init - front light PWM
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
	sConfigOC.Pulse = LED_OFF;	// 0 to 100% duty cycle in decimal numbers
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);  // commit it
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // start the timer
}

// device-specific constructor
void mj808_ctor()
{
	// general device non-specific low-level hardware init & config
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A));	// call base class constructor & initialize own SID

	_TimerInit();  // initialize Timers
	_GPIOInit();	// initialize device-specific GPIOs

	__Device.public.led = _virtual_led_ctorMJ808();  // call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ808();  // call virtual constructor & tie in object addresses

	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ808;	// implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function_mj808;	// implements event hander for this device

	// interrupt init
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 - Pushbutton handling
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	state = 0;	// used for testing the pushbutton

	__enable_irq();  // enable interrupts

	// TODO - access via object
	_util_led_mj808(UTIL_LED_GREEN_BLINK_1X);  // crude "I'm finished" indicator
}

// device-specific interrupt handlers
// pushbutton ISR
void EXTI0_1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(Pushbutton_Pin);  // service the interrupt

	// execute code

	// code to be executed every 25ms
	// TODO - sleep_disable();  // wakey wakey

	// TODO - remove this by means of implementing button handling
	if(state == 0)
		state = 1;
	else
		state = 0;

	Device->led->Shine(state);
	//Device->button->deBounce();  // call the debouncer

	// TODO - sleep_enable();  // back to sleep

	// TODO - implement mj808 pushbutton ISR

}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj808_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ808_
