#include "main.h"
#if defined(MJ808_)	// if this particular device is active

#include "mj808\mj808.h"
#include "mj808\mj808_led.c"	// concrete device-specific LED functions
#include "mj808\mj808_button.c"	// concrete device-specific button functions

TIM_HandleTypeDef htim2;	// front light PWM on channel 2
TIM_HandleTypeDef htim14;  // Timer14 object - LED handling - 20ms
TIM_HandleTypeDef htim16;  // Timer16 object - button handling - 25ms
TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 10ms

// TODO - these shouldn't really be here...
static TIM_ClockConfigTypeDef sClockSourceConfig =
	{0};

static TIM_MasterConfigTypeDef sMasterConfig =
	{0};

static TIM_OC_InitTypeDef sConfigOC =
	{0};

typedef struct	// mj808_t actual
{
	mj808_t public;  // public struct
} __mj808_t;

static __mj808_t __Device __attribute__ ((section (".data")));  // preallocate __Device object in .data

// defines device operation on empty bus
void _EmptyBusOperation(void)
{
	;  // TODO - define mj828 empty bus operation
}

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void _event_execution_function(const uint8_t val)
{
	EventHandler->UnSetEvent(val);	//

	Device->activity->ButtonPessed = ((Device->button->button[PushButton]->byte) > 0);	// translate button press into true or false

	switch(val)
		{
		case 0x00:	// not defined
			return;

		case 0x01:	// button error: - do the error thing
			__Device.public.led = _virtual_led_ctorMJ808();  // reset the LED component
			return;

		case 0x02:	// button hold
			Device->led->Shine(Device->button->button[PushButton]->Hold);  // turn the device on/off

			if(Device->button->button[PushButton]->Hold)
				{
					//send the messages out, UDP-style. no need to check if the device is actually online
					MsgHandler->SendMessage(MSG_BUTTON_EVENT_BUTTON0_ON, 0x00, 1);	// convey button press via CAN and the logic unit will do its own thing
					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 75, 2);  // turn on rear light
//					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT), 20, 2);  // turn on brake light
					MsgHandler->SendMessage(DASHBOARD_LED_GREEN_ON, 0x00, 1);  // turn on yellow LED
				}
			else
				{
					// send the messages out, UDP-style. no need to check if the device is actually online
					MsgHandler->SendMessage(MSG_BUTTON_EVENT_BUTTON0_OFF, 0x00, 1);  // convey button press via CAN and the logic unit will tell me what to do
					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0, 2);  // turn off rear light
//					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT), 0, 2);  // turn on brake light
					MsgHandler->SendMessage(DASHBOARD_LED_GREEN_OFF, 0x00, 1);  // turn off yellow LED
				}

			break;

		case 0x04:	// button toggle
			if(Device->button->button[PushButton]->Toggle)  // do something
				Device->led->led[Utility].Shine(UTIL_LED_RED_ON);
			else
				Device->led->led[Utility].Shine(UTIL_LED_RED_OFF);
			break;

//		case 0x08:	// next case
//			break;

//		case 0x10:	// next case
//			break;

//		case 0x20:	// next case
//			break;

//		case 0x20:	// next case
//			break;

//		case 0x80:	// next case
//			break;

//		case 0x80:	// next case
//			break;

		default:	// no value passed
			break;
		}
}

// received MsgHandler object and passes
void _PopulatedBusOperation(message_handler_t *const in_handler)
{
	can_msg_t *msg = in_handler->GetMessage();  // CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if((msg->COMMAND& CMND_UTIL_LED) == CMND_UTIL_LED)  // utility LED command
		{
			__Device.public.led->led[Utility].Shine(msg->COMMAND);	// glowy thingy
			return;
		}

	if(msg->COMMAND== (CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH))  // front positional light - high beam
		{
			__Device.public.led->led[Front].Shine(msg->ARGUMENT);  // high beam
			return;
		}

	if(msg->COMMAND== (CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT))  // front positional light - low beam
		{
			__Device.public.led->led[Front].Shine(msg->ARGUMENT);
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

	GPIO_InitStruct.Pin = TCAN334_Standby_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(TCAN334_Standby_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Pushbutton_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  // catch button press and release
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
	// Timer2 init - front light PWM
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
	sConfigOC.Pulse = LED_OFF;	// 0 to 100% duty cycle in decimal numbers
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);  // commit it
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);  // start the timer

	// timer 17 - event handling - 2.5ms
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = 799;  // 8MHz / 799+1 = 10kHz update rate
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = 24;  // with above pre-scaler and a period of 24, we have an 2.5ms interrupt frequency
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.RepetitionCounter = 0;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim17, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim17);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim17, &sMasterConfig);

	// timer 16 - button handling - 25ms
	htim16.Instance = TIM16;
	htim16.Init.Prescaler = 799;  // 8MHz / 799+1 = 10kHz update rate
	htim16.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim16.Init.Period = 249;  // with above pre-scaler and a period of 249, we have an 25ms interrupt frequency
	htim16.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim16.Init.RepetitionCounter = 0;
	htim16.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim16, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim16);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim16, &sMasterConfig);

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

	if(timer->Instance == TIM2)  // front led PWM
		__HAL_RCC_TIM2_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM14)  // led handling
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
			// Timer2 init - front light PWM
			__HAL_RCC_TIM2_CLK_ENABLE();  // start the clock
			HAL_TIM_PWM_Init(timer);  //
			HAL_TIM_PWM_ConfigChannel(timer, &sConfigOC, TIM_CHANNEL_2);  //
			HAL_TIM_PWM_Start(timer, TIM_CHANNEL_2);  // start the timer
			return;
		}

	if(timer->Instance == TIM14)	// LED fading
		{
			__HAL_RCC_TIM14_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = TIMER14_PRESCALER;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = TIMER14_PERIOD;
		}

	if(timer->Instance == TIM16)	// button handling
		{
			__HAL_RCC_TIM16_CLK_ENABLE();  // start the clock
			htim16.Instance->PSC = 799;  // reconfigure after peripheral was powered down
			htim16.Instance->ARR = 249;
		}

	if(timer->Instance == TIM17)	// event handling
		{
			__HAL_RCC_TIM17_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = 799;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = 24;
		}

	HAL_TIM_Base_Start_IT(timer);  // start the timer
}

// device-specific constructor
void mj808_ctor(void)
{
	// general device non-specific low-level hardware init & config
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A));	// call base class constructor & initialize own SID

	__Device.public.activity = (mj808_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type

	_GPIOInit();	// initialize device-specific GPIOs
	_TimerInit();  // initialize Timers

	__Device.public.led = _virtual_led_ctorMJ808();  // call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ808();  // call virtual constructor & tie in object addresses

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument

//	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperation;	// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperation;  // implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function;	// implements event hander for this device

// interrupt init
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 - Pushbutton handling
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(TIM14_IRQn, 0, 0);  // LED fade timer (on demand)
	HAL_NVIC_EnableIRQ(TIM14_IRQn);

	HAL_NVIC_SetPriority(TIM16_IRQn, 2, 0);  // button handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

	HAL_NVIC_SetPriority(TIM17_IRQn, 3, 0);  // event handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM17_IRQn);
}

// device-specific interrupt handlers
// pushbutton ISR
void EXTI0_1_IRQHandler(void)
{
	Device->StartTimer(&htim16);  // start the button handling timer
	Device->mj8x8->StartCoreTimer();  // start time core timer

	if(__HAL_GPIO_EXTI_GET_IT(Pushbutton_Pin))	// interrupt source detection
		// Pushbutton: released - pin high, pressed - pin low
		Device->button->button[PushButton]->Mark(!(HAL_GPIO_ReadPin(Pushbutton_GPIO_Port, Pushbutton_Pin)));  // mark state change

	HAL_GPIO_EXTI_IRQHandler(Pushbutton_Pin);  // service the interrupt
}

// timer 14 ISR - 20ms interrupt - LED handling (activated on demand)
void TIM14_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim14);  // service the interrupt
	Device->led->Handler();  // handles front LED fading
}

// timer 16 ISR - 25ms interrupt - button handling (activated on demand)
void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt

	Device->button->Handler();	// handle button press

	if(!Device->button->button[PushButton]->Momentary)	// if button not pressed
		Device->StopTimer(&htim16);  // stop the timer
}

// timer 17 ISR - 10ms interrupt - event handler (activated on demand)
void TIM17_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim17);  // service the interrupt
	EventHandler->HandleEvent();	// execute the event handling function with argument taken from case table
}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj808_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ808_
