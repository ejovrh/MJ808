#include "main.h"
#if defined(MJ828_)	// if this particular device is active

#include "mj828\mj828.h"
#include "mj828\mj828_led.c"	// concrete device-specific LED functions
#include "mj828\mj828_button.c"	// concrete device-specific button functions
#include "mj828\mj828_adc.c"	// concrete device-specific ADC functions

TIM_HandleTypeDef htim2;  // Timer2 object - ADC conversion - 500ms
TIM_HandleTypeDef htim14;  // Timer14 object - charlieplexed LED handling - 2ms
TIM_HandleTypeDef htim16;  // Timer16 object - button handling - 25ms
TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 10ms
extern ADC_HandleTypeDef hadc;  // ADC object

typedef struct	// mj828_t actual
{
	mj828_t public;  // public struct
} __mj828_t;

static __mj828_t __Device __attribute__ ((section (".data")));	// preallocate __Device object in .data

// defines device operation on empty bus
void _EmptyBusOperationMJ828(void)
{
	;  // TODO - define mj828 empty bus operation
}

// display battery charge status depending on ADC read
void _DisplayBatteryVoltage(void)
{
	volatile uint16_t temp = Device->adc->GetVal(Vbat);

	if(temp <= 1302 && temp > 1)	// 4.2V
		Device->led->Shine(Red);

	if(temp > 1302)  // 4.2V
		Device->led->Shine(Battery1);

	if(temp > 1675)  // 5.4V
		Device->led->Shine(Battery2);

	if(temp > 2048)  // 6.6V
		Device->led->Shine(Battery3);

	if(temp > 2421)  // 7.8V
		Device->led->Shine(Battery4);
}

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void _event_execution_function(uint8_t val)
{
	EventHandler->UnSetEvent(val);	//

	Device->activity->ButtonPessed = (  //	set device to state according to button press
	(  //
	Device->button->button[PushButton]->byte ||  // ORed byte values indicate _some_ button press is active
	Device->button->button[LeverFront]->byte ||  //
	Device->button->button[LeverBrake]->byte) > 0  // translate the above fact into true or false
	//
	);

	switch(val)
		// based on array value at position #foo of array e.g. FooButtonCaseTable[]
		{
		case 0x00:	// not defined
			return;

		case 0x01:	// button error: - do the error thing
			__Device.public.led = _virtual_led_ctorMJ828();  // reset the LED component
			return;

		case 0x02:	// lever back - braking action
			Device->led->Shine(Red);

			if(Device->button->button[LeverBrake]->Momentary)
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT), 250, 2);  // turn on (250 is a special value)
			else
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT), 200, 2);  // turn off (200 is a special value)

			break;

		case 0x04:	// lever front - high beam
			Device->led->Shine(Blue);

			if(Device->button->button[LeverFront]->Momentary)
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH), 250, 2);	// turn on (250 is a special value)
			else
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH), 200, 2);	// turn off (200 is a special value)

			break;

		case 0x08:	// pushbutton press
			_DisplayBatteryVoltage();  // light up BatteryX LEDs according to voltage read at Vbat
			break;

		case 0x10:	// pushbutton hold
			Device->led->Shine(Green);

			if(Device->button->button[PushButton]->Hold)
				{
					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT), 20, 2);
					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 20, 2);
				}
			else
				{
					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT), 0, 2);
					MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0, 2);
				}

			break;

		case 0x20:	// next case
			Device->led->Shine(Yellow);

			if(Device->button->button[PushButton]->Toggle)
				MsgHandler->SendMessage((CMND_DEVICE | UTIL_LED_RED_ON), 0, 1);
			else
				MsgHandler->SendMessage((CMND_DEVICE | UTIL_LED_RED_OFF), 0, 1);

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

// toggles a bit in the LED flags variable; charlieplexer in turn makes it shine
void _PopulatedBusOperation(message_handler_t *const in_handler)
{
	volatile can_msg_t *msg = in_handler->GetMessage();  // CAN message object

// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if((msg->COMMAND& MASK_COMMAND) == CMND_DASHBOARD )  // dashboard command
		{
			__Device.public.led->Shine(((msg->COMMAND & 0x0E) >> 1));  // flag LED at appropriate index as whatever the command says

			return;
		}
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

	GPIO_InitStruct.Pin = Pushbutton_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  // catch button press and release
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(Pushbutton_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = LeverFront_Pin | LeverBrake_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  // catch button press and release
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP1_Pin;	// set into hi-Z state
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CP1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP2_Pin;	// set into hi-Z state
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CP2_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP3_Pin;	// set into hi-Z state
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CP3_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP4_Pin;	// set into hi-Z state
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(CP4_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = VBat_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(VBat_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Phototransistor_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(Phototransistor_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - timer17 10ms periodic (event handler),
static inline void _TimerInit(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};
	TIM_MasterConfigTypeDef sMasterConfig =
		{0};

	// timer 17 - event handling - 10ms
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

	// timer14 - charlieplexed LED handling - 2ms
	htim14.Instance = TIM14;
	htim14.Init.Prescaler = 799;  // 8MHz / 799+1 = 10kHz update rate
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 19;  // with above pre-scaler and a period of 19, we have an 2ms interrupt frequency
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.RepetitionCounter = 0;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim14, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim14);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim14, &sMasterConfig);

	// timer2 - ADC sampling - 500ms
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 799;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 5000;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	__HAL_RCC_TIM2_CLK_ENABLE();
	HAL_TIM_Base_Init(&htim2);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	if(timer->Instance == TIM14)	// charlieplexing
		__HAL_RCC_TIM14_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM16)	// button handling
		__HAL_RCC_TIM16_CLK_DISABLE();  // stop the clock

	if(timer->Instance == TIM17)	// event handling
		__HAL_RCC_TIM17_CLK_DISABLE();  // stop the clock
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	if(timer->Instance == TIM14)	// charlieplexing
		{
			__HAL_RCC_TIM14_CLK_ENABLE();  // start the clock
			timer->Instance->PSC = 799;  // reconfigure after peripheral was powered down
			timer->Instance->ARR = 19;
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

void mj828_ctor(void)
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D));  // call base class constructor & initialize own SID

	__Device.public.activity = (mj828_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type

	_GPIOInit();	// initialize device-specific GPIOs
	_TimerInit();  // initialize timers

	__Device.public.led = _virtual_led_ctorMJ828();  // call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ828();  // call virtual constructor & tie in object addresses
	__Device.public.adc = adc_ctor();  // call ADC constructor

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument

//	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperation;	// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperation;	// implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function;	// implements event hander for this device

	// interrupt init
	HAL_NVIC_SetPriority(TIM14_IRQn, 4, 0);  // charlieplexed LED handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM14_IRQn);

	HAL_NVIC_SetPriority(TIM16_IRQn, 2, 0);  // button handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

	HAL_NVIC_SetPriority(TIM17_IRQn, 3, 0);  // event handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM17_IRQn);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 & EXTI1 - Pushbutton & LeverBrake handling
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);  // EXTI2 - LeverBrake handling
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

//	HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);  //
//	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	HAL_TIM_Base_Start(&htim2);  // ADC timer
}

// device-specific interrupt handlers
// timer 14 ISR - 2ms interrupt - charlieplexed LED handling (activated on demand)
void TIM14_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim14);  // service the interrupt
	Device->led->Handler();  // handles LED charlieplexing for multiple LEDs
}

// timer 16 ISR - 25ms interrupt - button handling (activated on demand)
void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt

	Device->button->Handler();	// handle button press

	if(!Device->button->button[PushButton]->Momentary)	// if button not pressed
		Device->StopTimer(&htim16);  // stop the timer

	if(Device->button->button[LeverFront]->Momentary)  // if button not pressed
		Device->StopTimer(&htim16);  // stop the timer

	if(Device->button->button[LeverBrake]->Momentary)
		Device->StopTimer(&htim16);  // stop the timer
}

// timer 17 ISR - 10ms interrupt - event handler (activated on demand)
void TIM17_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim17);  // service the interrupt
	EventHandler->HandleEvent();	// execute the event handling function with argument taken from case table
}

// EXTI pushbutton & lever front ISR
void EXTI0_1_IRQHandler(void)
{
	/* multiple EXTIs share this ISR
	 * pushbutton: released - pin high, pressed - pin low
	 * lever front: magnet - high, no magnet - low
	 */

	Device->StartTimer(&htim16);  // start the button handling timer
	Device->mj8x8->StartCoreTimer();  // start time core timer

	if(__HAL_GPIO_EXTI_GET_IT(Pushbutton_Pin))	// interrupt source detection
// Pushbutton: released - pin high, pressed - pin low
		Device->button->button[PushButton]->Mark(!(HAL_GPIO_ReadPin(Pushbutton_GPIO_Port, Pushbutton_Pin)));  // mark state change

	if(__HAL_GPIO_EXTI_GET_IT(LeverFront_Pin))	// interrupt source detection
// front lever: released (no magnet) - pin high, pressed (magnet) - pin low
		Device->button->button[LeverFront]->Mark(!(HAL_GPIO_ReadPin(LeverFront_GPIO_Port, LeverFront_Pin)));  // mark state change

	HAL_GPIO_EXTI_IRQHandler(Pushbutton_Pin);  // service the interrupt
	HAL_GPIO_EXTI_IRQHandler(LeverFront_Pin);  // service the interrupt
}

// lever brake ISR
void EXTI2_3_IRQHandler(void)
{
	Device->StartTimer(&htim16);  // start the button handling timer
	Device->mj8x8->StartCoreTimer();  // start time core timer

	if(__HAL_GPIO_EXTI_GET_IT(LeverBrake_Pin))	// interrupt source detection
// lever brake: released (no magnet) - pin high, pressed (magnet) - pin low
		Device->button->button[LeverBrake]->Mark(!(HAL_GPIO_ReadPin(LeverBrake_GPIO_Port, LeverBrake_Pin)));  // mark state change

	HAL_GPIO_EXTI_IRQHandler(LeverBrake_Pin);  // service the interrupt
}

// ADC ISR
void ADC1_IRQHandler(void)
{
	if((__HAL_ADC_GET_FLAG(&hadc, ADC_FLAG_EOC) && __HAL_ADC_GET_IT_SOURCE(&hadc, ADC_IT_EOC)))
		Device->adc->ReadChannels();	// on every ISR iteration, read out and store in ADC object

	HAL_ADC_IRQHandler(&hadc);  // service the interrupt
}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj828_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ828_
