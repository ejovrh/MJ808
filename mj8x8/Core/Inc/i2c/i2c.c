#include "main.h"
#if USE_I2C
#include "i2c.h"

static DMA_HandleTypeDef _hdma_i2c1_rx;  // I2C RX DMA handle
static DMA_HandleTypeDef _hdma_i2c1_tx;  // I2C TX DMA handle

static I2C_HandleTypeDef _hi2c;  //I2C handle

typedef struct	// i2c_t actual
{
	i2c_t public;  // public struct

	uint8_t _CriticalSection :1;
} __i2c_t;

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		uint8_t low_byte;  // low byte
		uint8_t high_byte;  // high byte
	};
	uint16_t two_byte;  // byte-wise representation of the above bitfield
} retval_t;

__i2c_t __I2C __attribute__ ((section (".data")));

inline void Error_Handler(void)
{
	__disable_irq();
	while(1)
		;
}

//
// I2C read function
void _Read(const uint16_t DevAddr, uint16_t RegAddr, uint16_t *data, uint16_t size)
{
	;
}

// I2C write function
void _Write(const uint16_t DevAddr, const uint16_t MemAddr, uint16_t const *data, uint16_t MemAddrSize)
{
	;
}

void _I2C_Stop(void)
{
	__HAL_RCC_DMA1_CLK_DISABLE();
	__HAL_RCC_I2C1_CLK_DISABLE();
}

//
void _I2C_Start(const uint32_t _SDA_Pin, const uint32_t _SCL_Pin, GPIO_TypeDef *_I2C_Port)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock

	GPIO_InitStruct.Pin = _SDA_Pin | _SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
	HAL_GPIO_Init(_I2C_Port, &GPIO_InitStruct);

	__HAL_RCC_I2C1_CLK_ENABLE();  // FIXME - correct position in startup sequence?
	_hi2c.Instance = I2C1;
	_hi2c.Init.Timing = 0x00201D2B;
	_hi2c.Init.OwnAddress1 = 0;
	_hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	_hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	_hi2c.Init.OwnAddress2 = 0;
	_hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	_hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	_hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&_hi2c);

	HAL_I2CEx_ConfigAnalogFilter(&_hi2c, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&_hi2c, 0);

	__HAL_RCC_DMA1_CLK_ENABLE();	// FIXME - correct position in startup sequence?
	// I2C TX Init
	_hdma_i2c1_tx.Instance = DMA1_Channel2;
	_hdma_i2c1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
	_hdma_i2c1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
	_hdma_i2c1_tx.Init.MemInc = DMA_MINC_ENABLE;
	_hdma_i2c1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	_hdma_i2c1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	_hdma_i2c1_tx.Init.Mode = DMA_NORMAL;
	_hdma_i2c1_tx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&_hdma_i2c1_tx);
	__HAL_LINKDMA(&_hi2c, hdmatx, _hdma_i2c1_tx);

	// I2C RX Init
	_hdma_i2c1_rx.Instance = DMA1_Channel3;
	_hdma_i2c1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
	_hdma_i2c1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
	_hdma_i2c1_rx.Init.MemInc = DMA_MINC_ENABLE;
	_hdma_i2c1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	_hdma_i2c1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
	_hdma_i2c1_rx.Init.Mode = DMA_NORMAL;
	_hdma_i2c1_rx.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&_hdma_i2c1_rx);
	__HAL_LINKDMA(&_hi2c, hdmarx, _hdma_i2c1_rx);

}

__i2c_t __I2C =  // instantiate event_handler_t actual and set function pointers
	{  //
	.public.I2C = &_hi2c,  //
	.public.Read = &_Read,  //
	.public.Write = &_Write,  //
	._CriticalSection = 0,  //
	};

void i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port)
{
	_I2C_Start(SDA_Pin, SCL_Pin, I2C_Port);
}

i2c_t *const I2C = &__I2C.public;

//
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&_hdma_i2c1_tx);
	HAL_DMA_IRQHandler(&_hdma_i2c1_rx);
}

//
void I2C1_IRQHandler(void)
{
	if(_hi2c.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
		{
			HAL_I2C_ER_IRQHandler(&_hi2c);
		}
	else
		{
			HAL_I2C_EV_IRQHandler(&_hi2c);
		}
}
#endif
