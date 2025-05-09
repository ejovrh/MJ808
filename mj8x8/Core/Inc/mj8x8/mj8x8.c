#include "main.h"
#include "mj8x8.h"
#include "can.h"

static TIM_HandleTypeDef htim1;  // Timer1 object
//static IWDG_HandleTypeDef hiwdg;  // Independent Watchdog object

typedef struct	// mj8x8_t actual
{
	mj8x8_t public;  // public struct

	uint8_t __NumericalCAN_ID;	// private - ordered device number - A0 (0th device) until 3C (15th device), used in Heartbeat()
	uint8_t __foo :1;  // placeholder
	uint8_t __HeartBeatCounter :5;	// private - counts the heartbeat from 0 to 31 (essentially counts how many times timer1's ISR executed)
	uint8_t __HeartBeatIterationCounter :2;  // we are alone on the bus - shall we do our device-specific default operation?
} __mj8x8_t;

extern __mj8x8_t __MJ8x8;	// declare mj8x8_t actual

static inline void _DoNothing(void)  // a function that does nothing
{
	return;
}

// provides a periodic heartbeat based on the timer1 interrupt
static void _Heartbeat(message_handler_t *const msg)
{
	/* the purpose of heartbeat is ...
	 * ... to provide a means for other devices to get aware of each other on power-on.
	 * when a device is powered on, by default it doesn't know whether it is alone on the bus or not.
	 * on power on, the device starts with CAN enabled and in full wake mode.
	 * timer1 is set to execute its ISR every 125ms and thereby the heartbeat routine - increment a counter and compare that counter to an ID.
	 *
	 * each device has the corresponding unique CAN ID translated into a number - 0 to 15 (16 devices, currently).
	 * if the counter equals the device's numerical value, a CAN message is sent out and received by other devices.
	 * the act of receiving a message from some other device, the receiving device gains knowledge about the sender's existence.
	 *
	 * if after a few iterations no devices are discovered, a bus-off routine is executed.
	 * e.g. in the case of mj818 (the rear light without switches), it turns itself on automatically and just shines.
	 */

	uint8_t *_payload = &Device->activity->byte;  // set the HeartBeat (aka. CMND_ANNOUNCE) argument

#if BROADCAST_HEARTBEAT
	if(__MJ8x8.__HeartBeatCounter == __MJ8x8.__NumericalCAN_ID)  // see if this counter iteration is our turn
		msg->SendMessage(ALL, CMND_ANNOUNCE, _payload, 2);  // if so, broadcast CAN heartbeat message and disguise device status in it
#endif

	++__MJ8x8.__HeartBeatCounter;  // increment the iteration counter

	if(__MJ8x8.__HeartBeatCounter == 0)  // counter roll-over - one complete heartbeat iteration
		{
			++__MJ8x8.__HeartBeatIterationCounter;  // essentially count how many times we are in non-heartbeat count mode

			if(__MJ8x8.__HeartBeatIterationCounter > 2)  // two complete heartbeat iterations have passed
				{
					__MJ8x8.public.can->activity->DoHeartbeat = 0;  // mark as complete and allow sleep

					if(MsgHandler->Devices->byte == 0)	// if we are alone on the bus
						__MJ8x8.public.EmptyBusOperation();  // perform the device-specific default operation (is overridden in specific device constructor)
				}
		}
}

// stops timer1
static void _StopTimer1(void)
{
	HAL_TIM_Base_Stop_IT(&htim1);  // stop the timer
	__HAL_RCC_TIM1_CLK_DISABLE();  // stop the clock
}

// starts timer1 - used by can_t
static void _StartTimer1(void)
{
	__HAL_RCC_TIM1_CLK_ENABLE();  // start the clock
	htim1.Instance->PSC = TIMER1_PRESCALER;  // reconfigure after peripheral was powered down
	htim1.Instance->ARR = TIMER1_PERIOD;
	HAL_TIM_Base_Start_IT(&htim1);  // stop the timer
}

// updates a particular activity and notifies the bus
static void _UpdateActivity(const uint8_t act, const uint8_t val)
{
	uint8_t *_payload = &Device->activity->byte;  // set the HeartBeat (aka. CMND_ANNOUNCE) argument

	if(val == OFF)
		Device->activity->byte &= (uint8_t) ~_BV(act);  // clear the bit

	if(val == ON)
		Device->activity->byte |= (uint8_t) _BV(act);  // set it

	MsgHandler->SendMessage(ALL, CMND_ANNOUNCE, _payload, 2);  // notify the bus of the change
}

// returns whether some activity is ON (1) or OFF(0)
static inline uint8_t _GetActivity(const uint8_t act)
{
	return (Device->activity->byte & _BV(act));
}

__mj8x8_t __MJ8x8 =  // instantiate mj8x8_t actual and set function pointers
	{  //
	.public.StartCoreTimer = &_StartTimer1,  // timer1 stop - see can_t for start
	.public.StopCoreTimer = &_StopTimer1,  // timer1 stop - see can_t for start
	.public.HeartBeat = &_Heartbeat,  // implement device-agnostic default behaviour - heartbeat
	.public.EmptyBusOperation = &_DoNothing,  // implement device-agnostic default behaviour - do nothing, usually an override happens
	.public.UpdateActivity = &_UpdateActivity,  // updates a particular activity and notifies the bus
	.public.GetActivity = &_GetActivity  // returns whether some activity is ON (1) or OFF(0)
	};

// system clock config
static inline void _SystemClockConfig(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
		{0};  // instantiate and initialize
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
		{0};  // instantiate and initialize

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSI14;  // no external crystal
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;	// turn it on
	RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;  //
	RCC_OscInitStruct.HSI14CalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;  // no PLL
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;  //
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;	//
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;	//
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;  //
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0);  //
}

// IWDG init
//void _IWDGInit(void)
//{
//	hiwdg.Instance = IWDG;
//	hiwdg.Init.Prescaler = IWDG_PRESCALER_4;	// 40kHz / 4 = 10kHz
//	hiwdg.Init.Window = 4095;
//	hiwdg.Init.Reload = 4095;  // 4095/10kHz = 409.5ms
//	HAL_IWDG_Init(&hiwdg);
//}

// GPIO init - device non-specific
static inline void _GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_GPIOA_CLK_ENABLE();  // enable peripheral clock

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
	htim1.Init.Prescaler = TIMER1_PRESCALER;  // 8MHz / 799+1 = 10kHz update rate
	htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim1.Init.Period = TIMER1_PERIOD;  // with above pre-scaler and a period of 1249, we have an 125ms interrupt frequency
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
	// called periodically by TIM1_BRK_UP_TRG_COM_IRQHandler()
	// wakeup events are only EXTIs

#if USE_HAL_STOPMODE
	if(**__MJ8x8.public.activity & POWERSAVE_DEVICE_SLEEPONEXIT_ACTIVE_MASK)  // true if device is active in some form (see actual device implementation)
		{
#if USE_CAN_BUSACTIVE
			if(**__MJ8x8.public.activity & POWERSAVE_CANBUS_ACTIVE_MASK)  // upper nibble indicates activity
				__MJ8x8.public.can->GoBusActive(1);  // put CAN infrastructure into active state
			else
				__MJ8x8.public.can->GoBusActive(0);  // put CAN infrastructure into standby state
#endif

			__MJ8x8.public.PreSleep();	// call the derived object's sleep implementation

#if USE_HAL_SLEEPONEXIT
			HAL_PWR_EnableSleepOnExit();	// go to sleep once any ISR finishes
#endif

			;
		}
	else	// if device is not active - i.e the (activity_byte & 0x3F == 0)
		{
			__MJ8x8.public.PreStop();  // call the derived object's stop implementation

#if USE_CAN_BUSACTIVE
			__MJ8x8.public.can->GoBusActive(0);  // put CAN infrastructure into standby state
#endif

			__MJ8x8.public.StopCoreTimer();  // stop timer1

#if USE_HAL_SLEEPONEXIT
			HAL_PWR_DisableSleepOnExit();  // go to sleep once any ISR finishes
#endif

			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);  // go into stop mode
		}
#else
#if USE_HAL_SLEEPONEXIT
	HAL_PWR_EnableSleepOnExit();	// go to sleep once any ISR finishes
#endif
#endif

	;
}

// general device non-specific low-level hardware init & config
mj8x8_t* mj8x8_ctor(const mj8x8_Devices_t in_MJ8x8_ID)
{
	HAL_Init();  // systick is needed for CAN

	_SystemClockConfig();  // initialize the system clock
//	_IWDGInit();	// initialise the independent watchdog
	_GPIOInit();	// initialize device non-specific GPIOs
	_TimerInit();  // initialize Timer - heartbeat

	__MJ8x8.__NumericalCAN_ID = in_MJ8x8_ID;	// set the numerical CAN ID - 0 through 15.

	__MJ8x8.public.can = can_ctor(in_MJ8x8_ID);  // tie in CAN public part & pass down devise's own CAN ID for filter config
	__MJ8x8.public.activity = (uint8_t**) &__MJ8x8.public.can->activity;  // tie in can_t activity into mj8x8_t activity  (is tied in again one level up)
	__MJ8x8.public.can->own_sid = (in_MJ8x8_ID << 4);  // translate MJ8x8 device ID to sender format for a mj8x8-CAN standard ID frame
	__MJ8x8.public.can->activity->DoHeartbeat = 1;	// start up with heartbeat enabled
	__MJ8x8.public.can->Timer1Start = &_StartTimer1;  //

	__MJ8x8.public.Sleep = &_Sleep;  // puts device to sleep
	__MJ8x8.public.PreSleep = &_DoNothing;  // the derived object implements its own special sleep method

#if USE_I2C
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SysTick_IRQn);
#else
	HAL_SuspendTick();
#endif

	// interrupt init
	// control for interrupt operation prior to init finished
	// e.g. mj514's I2C init and this interrupt cause hardfaults
#if USE_I2C == 0
	HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
#endif

//	HAL_DBGMCU_EnableDBGStopMode();

	return &__MJ8x8.public;  // return address of public part; calling code accesses it via pointer
}

// device non-specific interrupt handlers
// timer 1 ISR - 125ms interrupt - CAN bus heartbeat
void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
//	HAL_IWDG_Refresh(&hiwdg);  // refresh the watchdog

	HAL_TIM_IRQHandler(&htim1);  // service the interrupt

	__MJ8x8.public.HeartBeat(MsgHandler);  // execute the heartbeat

#if EXECUTE_SLEEP
	__MJ8x8.public.Sleep();  // go into an appropriate sleep state as allowed by FlagActive
#endif
}
