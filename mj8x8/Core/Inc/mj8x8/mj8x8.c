#include "mj8x8.h"

static TIM_HandleTypeDef htim1;  // Timer1 object
static IWDG_HandleTypeDef hiwdg;  // Independent Watchdog object

typedef struct	// mj8x8_t actual
{
	mj8x8_t public;  // public struct

	uint8_t __NumericalCAN_ID;	// private - ordered device number - A0 (0th device) until 3C (15th device), used in Heartbeat()
	uint8_t __FlagDoHeartbeat :1;  // private - shall the heartbeat be initiated?
	uint8_t __BeatIterationCount :4;	// private - how many times did we wakeup, sleep and wakeup again
	uint8_t __FlagDoDefaultOperation :2;	// we are alone on the bus - shall we do our device-specific default operation?
} __mj8x8_t;

extern __mj8x8_t __MJ8x8;	// declare mj8x8_t actual

static inline void _DoNothing(void)  // a function that does nothing
{
	return;
}

// provides a periodic heartbeat based on the watchdog timer interrupt
static void _Heartbeat(message_handler_t *const msg)
{
	if(__MJ8x8.__FlagDoHeartbeat)  // if we are in heartbeat mode
		{
			if(__MJ8x8.__BeatIterationCount == __MJ8x8.__NumericalCAN_ID)  // see if this counter iteration is our turn
				{
					msg->SendMessage(CMND_ANNOUNCE, 0x00, 1);  // broadcast CAN heartbeat message

					__MJ8x8.__FlagDoHeartbeat = 0;	// heartbeat mode of for the remaining counter iterations
				}
		}

	if((!__MJ8x8.__BeatIterationCount) && (!__MJ8x8.__FlagDoHeartbeat))  // counter roll-over, change from slow to fast
		{
			__MJ8x8.__FlagDoHeartbeat = 1;	// set heartbeat mode on
			++__MJ8x8.__FlagDoDefaultOperation;  // essentially count how many times we are in non-heartbeat count mode
		}
	++__MJ8x8.__BeatIterationCount;  // increment the iteration counter
}

__mj8x8_t __MJ8x8 =  // instantiate mj8x8_t actual and set function pointers
	{  //
	.public.HeartBeat = &_Heartbeat,  // implement device-agnostic default behaviour - heartbeat
	.public.EmptyBusOperation = &_DoNothing,  // implement device-agnostic default behaviour - do nothing, usually an override happens
	.__FlagDoHeartbeat = 1,  // start with discovery mode
	.__FlagDoDefaultOperation = 0  // control flag
	};

// system clock config
static inline void _SystemClockConfig(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
		{0};  // instantiate and initialize
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
		{0};  // instantiate and initialize

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;	// no external crystal
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;	// turn it on
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;  //
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;	// no PLL
	HAL_RCC_OscConfig(&RCC_OscInitStruct);  //

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;  //
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;	//
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;	//
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  //
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);  //
}

// IWDG init
void _IWDGInit(void)
{
	hiwdg.Instance = IWDG;
	hiwdg.Init.Prescaler = IWDG_PRESCALER_4;	// 40kHz / 4 = 10kHz
	hiwdg.Init.Window = 4095;
	hiwdg.Init.Reload = 4095;  // 4095/10kHz = 409.5ms
	HAL_IWDG_Init(&hiwdg);
}

// GPIO init - device non-specific
static inline void _GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_GPIOA_CLK_ENABLE();  // enable peripheral clock
//	__HAL_RCC_CAN1_CLK_ENABLE();	// enable CAN bus clock

	// GPIO init is done in two steps:
	//	1. set all to analog in order to reduce power consumption (done here)
	//	2. init actual used pins on specific device (done in device-specific constructor)

	// port A
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_15;  // all pins minus SWD
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;  // set to analog
	GPIO_InitStruct.Pull = GPIO_NOPULL;  // no Push-Pull
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);  // commit it

	// port B
	GPIO_InitStruct.Pin = GPIO_PIN_All;  // all pins on port
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// port F
	GPIO_InitStruct.Pin = GPIO_PIN_All;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	// CAN pins
	GPIO_InitStruct.Pin = CAN_RX_Pin | CAN_TX_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_CAN;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

// Timer init - CAN bus heartbeat - 125ms
static inline void _TimerInit(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig =
		{0};
	TIM_MasterConfigTypeDef sMasterConfig =
		{0};

	htim1.Instance = TIM1;
	htim1.Init.Prescaler = 799;  // 8MHz / 799+1 = 10kHz update rate
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = 1249;  // with above pre-scaler and a period of 1249, we have an 125ms interrupt frequency
	htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim1.Init.RepetitionCounter = 0;
	htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	__HAL_RCC_TIM1_CLK_ENABLE();	// start the clock

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig);
	HAL_TIM_OC_Init(&htim1);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig);

	HAL_TIM_Base_Start_IT(&htim1);	// start the timer
}

// puts device to sleep
static void _Sleep(void)
{
	if(__MJ8x8.public.FlagActive)
		HAL_PWR_EnableSleepOnExit();	// go to sleep once any ISR finishes
	else
		{
			HAL_PWR_DisableSleepOnExit();
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);  // go into stop mode
		}
}

// general device non-specific low-level hardware init & config
mj8x8_t* mj8x8_ctor(const uint8_t in_own_sidh)
{
	_SystemClockConfig();  // initialize the system clock
//	_IWDGInit();	// initialise the independent watchdog
	_GPIOInit();	// initialize device non-specific GPIOs
	_TimerInit();  // initialize Timer - heartbeat

	__MJ8x8.__NumericalCAN_ID = (uint8_t) ((in_own_sidh >> 2) & 0x0F);	// set the CAN id.

	__MJ8x8.public.can = can_ctor();	// pass on CAN public part

	__MJ8x8.public.Sleep = &_Sleep;  // puts device to sleep

	__MJ8x8.public.FlagActive = 0;	// mark the device as inactive by default (will go immediately into sleep/stop move after initialisation)

	__MJ8x8.public.can->own_sidh = in_own_sidh;  // high byte
	__MJ8x8.public.can->own_sidl = (RCPT_DEV_BLANK | BLANK);	// low byte

	HAL_NVIC_DisableIRQ(SysTick_IRQn);	// get rid of SysTick
	HAL_NVIC_ClearPendingIRQ(SysTick_IRQn);
	HAL_SuspendTick();

	// interrupt init
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

	HAL_NVIC_SetPriority(CEC_CAN_IRQn, 1, 1);
	HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);

//	HAL_DBGMCU_EnableDBGStopMode();

	return &__MJ8x8.public;  // return address of public part; calling code accesses it via pointer
}

// timer 1 ISR - 125ms interrupt - CAN bus heartbeat
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
//	HAL_IWDG_Refresh(&hiwdg);  // refresh the watchdog

	HAL_TIM_IRQHandler(&htim1);  // service the interrupt
	// TODO - implement sleep cycles for processor and CAN bus hardware

	/* heartbeat of device on bus - aka. active CAN bus device discovery
	 *
	 *	- on the CAN bus there is room for max. 16 devices - in numeric form devices #0 up to #15
	 *	- each device has its own unique NumericalCAN_ID, which is derived from its unique CAN bus ID
	 *
	 *	- on each watchdog timer iteration, BeatIterationCount is incremented (it is able to roll over from 0xf to 0x0)
	 *	- when BeatIterationCount is equal to the device's NumericalCAN_ID, a heartbeat message is sent, otherwise nothing is done
	 *
	 *	- there are two modes: fast count and slow count - each set via WDTCR, the WatchDog Timer Control Register
	 *		- fast count is performed when a heartbeat is supposed to be sent
	 *			this mode speeds up the heartbeat procedure itself
	 *
	 *		- slow count is performed when no heartbeat is supposed to be sent
	 *			this mode acts as a delay for heartbeat messages themselves
	 *
	 *	i.e. start in heartbeat mode, count fast to one's own ID, send the message and then exit heartbeat mode,
	 *		continue counting slow until a counter rollover occurs and enter heartbeat mode again.
	 *
	 *	each device on the bus does this procedure.
	 *	after one complete iteration each device should have received some other device's heartbeat message.
	 *	on each and every message reception the senders ID is recorded in the canbus_t struct. thereby one device keeps track of its neighbors on the bus.
	 *
	 *	if there are no devices on the bus, a device-specific default operation is executed.
	 */
	__MJ8x8.public.EmptyBusOperation();  // perform the device-specific default operation (is overridden in specific device constructor)
	__MJ8x8.public.HeartBeat(MsgHandler);  // execute the heartbeat

	if((!MsgHandler->Devices) && (__MJ8x8.__FlagDoDefaultOperation > 1))	// if we have passed one iteration of non-heartbeat mode and we are alone on the bus
		__MJ8x8.public.EmptyBusOperation();  // perform the device-specific default operation (is overridden in specific device constructor)

	__MJ8x8.public.Sleep();  // go into an appropriate sleep state as allowed by FlagActive
}

// device non-specific interrupt handlers
