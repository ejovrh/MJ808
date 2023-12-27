#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "as5601.h"

typedef struct	// as5601c_t actual
{
	as5601_t public;  // public struct

	volatile uint32_t *RotaryEncoderBuffer;  //
	volatile uint16_t counter;  //
} __as5601_t;

extern TIM_HandleTypeDef htim3;  // rotary encoder handling
static DMA_HandleTypeDef hdma_tim3_ch4_up;  // rotary encoder handling

static __as5601_t __AS5601 __attribute__ ((section (".data")));  // preallocate __AS5601 object in .data

// FIXME - write primitive read()/write() using DMA

static float _CountRotation(void)
{
	return (__AS5601.counter / FULL_REVOLUTION);
}

// DMA init - device specific
static inline void __DMAInit(void)  // TODO - call function from somewhere
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	hdma_tim3_ch4_up.Instance = DMA1_Channel3;
	hdma_tim3_ch4_up.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_tim3_ch4_up.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_tim3_ch4_up.Init.MemInc = DMA_MINC_ENABLE;
	hdma_tim3_ch4_up.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_tim3_ch4_up.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
	hdma_tim3_ch4_up.Init.Mode = DMA_NORMAL;
	hdma_tim3_ch4_up.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_tim3_ch4_up);

	__HAL_LINKDMA(&htim3, hdma[TIM_DMA_ID_CC4], hdma_tim3_ch4_up);
	__HAL_LINKDMA(&htim3, hdma[TIM_DMA_ID_UPDATE], hdma_tim3_ch4_up);

	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	HAL_DMA_Start_IT(&hdma_tim3_ch4_up, (uint32_t) &htim3.Instance->CNT, (uint32_t) &__AS5601.RotaryEncoderBuffer, 1);
}

static __as5601_t __AS5601 =  // instantiate as5601c_t actual and set function pointers
	{  //
	.public.CountRotation = &_CountRotation,  // set function pointer
	};

as5601_t* as5601_ctor(void)  //
{
	__DMAInit();	//

	__AS5601.counter = 0;  //

	return &__AS5601.public;  // set pointer to AS5601 public part
}

// DMA ISR - rotary encoder handling
void HAL_TIM_DMACompletCallback(DMA_HandleTypeDef *hdma)
{
	if(*__AS5601.RotaryEncoderBuffer > 0xFFFF / 2)  // FIXME - up or downshift?
		{
			__AS5601.public.FlagRotatingCW = 1;  // FIXME - VALIDATE - gear turned clockwise
			__AS5601.public.FlagRotatingCCW = 0;

			++__AS5601.counter;
		}

	if(*__AS5601.RotaryEncoderBuffer < 0xFFFF / 2)  // FIXME - up or downshift?
		{
			__AS5601.public.FlagRotatingCW = 0;
			__AS5601.public.FlagRotatingCCW = 1;	// FIXME - VALIDATE - gear turned counter-clockwise

			--__AS5601.counter;  // gear turned counterclockwise
		}
}

#endif
