#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "as5601.h"

typedef struct	// as5601c_t actual
{
	as5601_t public;  // public struct

	volatile uint16_t counter;  //
} __as5601_t;

extern TIM_HandleTypeDef htim2;  // rotary encoder time-base
extern TIM_HandleTypeDef htim3;  // rotary encoder handling

static volatile uint8_t _OVFcnt;

static __as5601_t __AS5601 __attribute__ ((section (".data")));  // preallocate __AS5601 object in .data

// FIXME - write primitive read()/write() using DMA
static void _Read(void)
{
	;
}

static void _Write(void)
{
	;
}

static float _CountRotation(void)
{
	return (__AS5601.counter / FULL_REVOLUTION);
}

// DMA init - device specific
static inline void __DMAInit(void)  // TODO - call function from somewhere
{
	;
}

static __as5601_t __AS5601 =  // instantiate as5601c_t actual and set function pointers
	{  //
	.public.CountRotation = &_CountRotation,  // set function pointer
	.public.Read = &_Read,	//
	.public.Write = &_Write  //
	};

as5601_t* as5601_ctor(void)  //
{
	_OVFcnt = 0;

	__DMAInit();	//

	Device->StartTimer(&htim2);
	Device->StartTimer(&htim3);

	return &__AS5601.public;  // set pointer to AS5601 public part
}

//
void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim2);  // service the interrupt
}

void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt
}

//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim2)
		{
			__AS5601.public.Rotation = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);	// 0 - CW, 1 - CCW, see rotation_t
			__AS5601.counter = __HAL_TIM_GET_COUNTER(&htim3);
		}

	if(htim == &htim3)
		++_OVFcnt;
}

#endif
