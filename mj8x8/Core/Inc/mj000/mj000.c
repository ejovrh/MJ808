#include "main.h"

#if defined(MJ000_)	// if this particular device is active

#include "try/try.h"	// top-level mj8x8 object for consolidated behaviour code
#include "mj000/mj000.h"
#include "automotion.h"	// automatic motion detection

TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 10ms - FIXME - should not be here

typedef struct	// mj000_t actual
{
	mj000_t public;  // public struct
} __mj000_t;

static __mj000_t                __Device                __attribute__ ((section (".data")));	// preallocate __Device object in .data

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

	GPIO_InitStruct.Pin = SD_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = ADXL367_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(ADXL367_CS_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = ADXL367_INT1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(ADXL367_INT1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SD_DET_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(SD_DET_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_MISO_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(SPI_MISO_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_MOSI_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(SPI_MOSI_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = SPI_SCK_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(SPI_SCK_GPIO_Port, &GPIO_InitStruct);
}

// Timer init - device specific
static inline void _TimerInit(void)
{
	// TODO - mj000 - timer init
	;
}

// stops timer identified by argument
static void _StopTimer(TIM_HandleTypeDef *timer)
{
	HAL_TIM_Base_Stop_IT(timer);  // stop the timer

	// TODO - mj000 - timer stop
}

// starts timer identified by argument
static void _StartTimer(TIM_HandleTypeDef *timer)
{
	// TODO - mj000 - timer start

	HAL_TIM_Base_Start_IT(timer);  // start the timer
}

// device-specific sleep
static inline void _DerivedSleep(void)
{
	// TODO - mj000 - derived sleep
}

void mj000_ctor(void)
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor(mj000);  // call base class constructor & initialize own SID

	__Device.public.activity = (mj000_activity_t*) *__Device.public.mj8x8->activity;  // tie in activity from the depths of mj8x8_t and redefine type
	Device->activity->CANActive = 1;
	_GPIOInit();	// initialize device-specific GPIOs
	_TimerInit();  // initialize timers

	__Device.public.automotion = automotion_ctor();  // call AutoMotion constructor

	__Device.public.StopTimer = &_StopTimer;	// stops timer identified by argument
	__Device.public.StartTimer = &_StartTimer;	// starts timer identified by argument

	__Device.public.mj8x8->EmptyBusOperation = Try->EmptyBusOperation;	// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = Try->PopulatedBusOperation;  // implements device-specific operation depending on bus activity
//	__Device.public.mj8x8->DerivedSleep = &_DerivedSleep;  // implements the derived object sleep

	// interrupt init
//	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);  // SPI DMA interrupts
//	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);	// SPI interrupts
	HAL_NVIC_EnableIRQ(SPI1_IRQn);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // ADXL367 activity interrupts

	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
	// TODO - mj000 - interrupt init
}

// device-specific interrupt handlers
// TODO - mj000 - interrupt ISRs
// device-specific interrupt handlers

// all devices have the object name "Device", hence the preprocessor macro
mj000_t *const Device = &__Device.public;  // set pointer to MsgHandler public part

#endif // MJ000_
