#include "main.h"
#if defined(MJ828_)	// if this particular device is active

#include "mj828\mj828.h"
#include "mj828\mj828_led.c"	// concrete device-specific LED functions
#include "mj828\mj828_button.c"	// concrete device-specific button functions

#define EXTI0_1_IRQn 5	// FIXME - EXTI0_1_IRQn should be included somehow, but isnt..
#define EXTI2_3_IRQn 6	// FIXME - EXTI0_1_IRQn should be included somehow, but isnt..
#define ADC1_IRQn 12	// FIXME - EXTI0_1_IRQn should be included somehow, but isnt..

static ADC_HandleTypeDef hadc;

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

void _event_execution_function_mj828(uint8_t val)
{
	EventHandler->UnSetEvent(val);

	switch(val)
		// based on array value at position #foo of array e.g. FooButtonCaseTable[]
		{
		case 0x01:	// error button press
			// TODO - implement device function on button error press
			break;

		case 0x02:	// lever back - braking action
			Device->led->Shine(Red);
			break;

		case 0x04:	// lever front - high beam
			Device->led->Shine(Blue);
			break;

		case 0x08:	// pushbutton press
			Device->led->Shine(Yellow);
			break;

		case 0x10:	// pushbutton hold
			Device->led->Shine(Green);
			break;

		case 0x20:	// next case
			Device->led->Shine(Battery4);
			break;

//		case 0x20:	// next case
//			break;

//		case 0x80:	// next case
//			break;

		default:	// no value passed
			break;
		}
}

// toggles a bit in the LED flags variable; charlieplexer in turn makes it shine
void _PopulatedBusOperationMJ828(message_handler_t *const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();  // CAN message object

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
	__HAL_RCC_GPIOF_CLK_ENABLE();  // enable peripheral clock
	__HAL_RCC_ADC1_CLK_ENABLE();	// enable ADC clock

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

// ADC init - device specific
void _ADCInit(void)
{
	ADC_ChannelConfTypeDef sConfig =
		{0};

	hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc.Init.LowPowerAutoWait = DISABLE;
	hadc.Init.LowPowerAutoPowerOff = DISABLE;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	HAL_ADC_Init(&hadc);

	sConfig.Channel = ADC_CHANNEL_3;	//	Battery voltage
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_9;	//	Phototransistor
	HAL_ADC_ConfigChannel(&hadc, &sConfig);
}

// interrupt extension, triggered by timer 1 ISR - 2.5ms interrupt in mj8x8
void _SystemInterrupt(void)
{
	Device->led->Handler();  // handles LED charlieplexing for multiple LEDs

	if((Device->mj8x8->SysIRQCounter % 10) == 0)	// 25ms
		Device->button->Handler();	// handle button press
}

void mj828_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D));  // call base class constructor & initialize own SID

	_GPIOInit();	// initialize device-specific GPIOs
	_ADCInit();  // initialize device-specific ADC

	__Device.public.led = _virtual_led_ctorMJ828();  // call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ828();  // call virtual constructor & tie in object addresses

//	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperationMJ828;	// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ828;	// implements device-specific operation depending on bus activity
	__Device.public.mj8x8->SystemInterrupt = &_SystemInterrupt;  // implement device-specific system interrupt code

	EventHandler->fpointer = &_event_execution_function_mj828;	// implements event hander for this device

	// interrupt init
	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 & EXTI1 - Pushbutton & LeverBrake handling
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);  // EXTI2 - LeverBrake handling
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	HAL_NVIC_SetPriority(ADC1_IRQn, 0, 0);	// ADC interrupt handling
	HAL_NVIC_EnableIRQ(ADC1_IRQn);

	__enable_irq();  // enable interrupts
}

// device-specific interrupt handlers
// pushbutton & lever front ISR
void EXTI0_1_IRQHandler(void)
{
	/* multiple EXTIs share this ISR
	 * pushbutton: released - pin high, pressed - pin low
	 * lever front: magnet - high, no magnet - low
	 */

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
	if(__HAL_GPIO_EXTI_GET_IT(LeverBrake_Pin))	// interrupt source detection
// lever brake: released (no magnet) - pin high, pressed (magnet) - pin low
		Device->button->button[LeverBrake]->Mark(!(HAL_GPIO_ReadPin(LeverBrake_GPIO_Port, LeverBrake_Pin)));  // mark state change

	HAL_GPIO_EXTI_IRQHandler(LeverBrake_Pin);  // service the interrupt
}

// ADC ISR
void ADC1_IRQHandler(void)
{
	// execute code
	// TODO - implement ADC code

	HAL_ADC_IRQHandler(&hadc);  // service the interrupt
}
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj828_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ828_
