#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "try\try.h"	// top-level mj8x8 object for consolidated behaviour code
#include "mj514\mj514.h"

// FIXME - define timer objects
TIM_HandleTypeDef htim17;  // event handling - 2.5ms

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

	GPIO_InitStruct.Pin = Motor_Direction_Up_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Motor_Direction_Up_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Motor_Direction_Down_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Motor_Direction_Down_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Rotary_A_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Rotary_A_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = Rotary_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(Rotary_B_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = FeRAM_WP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(FeRAM_WP_GPIO_Port, &GPIO_InitStruct);

// FIXME - define device GPIOs
}

// Timer init - device specific
static inline void _TimerInit(void)
{
// FIXME - define device timers
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

// FIXME - define timer stop
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
// FIXME - define timer start
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
	__Device.public.gear = gear_ctor();		// electronic gear shifting unit

	__Device.public.mj8x8->EmptyBusOperation = Try->EmptyBusOperation;  // override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = Try->PopulatedBusOperation;  // implements device-specific operation depending on bus activity
	__Device.public.mj8x8->PreSleep = &_PreSleep;  // implements the derived object prepare to sleep
	__Device.public.mj8x8->PreStop = &_PreStop;  // implements the derived object prepare to stop

//	EventHandler->fpointer = Try->EventHandler;  // implements event hander for this device

// interrupt init
// FIXME - define // interrupt init
}

// device-specific interrupt handlers
// FIXME - define device-specific interrupt handlers
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj514_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // mj514_
