#include "main.h"

#if USE_SPI
#include "spi.h"

SPI_HandleTypeDef hspi;  // SPI handle
//static DMA_HandleTypeDef _hdma_spi_rx;  // SPI RX DMA handle
//static DMA_HandleTypeDef _hdma_spi_tx;  // SPI TX DMA handle

typedef struct	// spi_t actual
{
	spi_t public;  // public struct

	uint8_t _CriticalSection :1;
} __spi_t;

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		uint8_t low_byte;  // low byte
		uint8_t high_byte;  // high byte
	};
	uint16_t two_byte;  // byte-wise representation of the above bitfield
} retval_t;

__spi_t                                              __SPI                                              __attribute__ ((section (".data")));

inline void Error_Handler(void)
{
	__disable_irq();
	while(1)
		;
}

void _SPI_Stop(void)
{
	__HAL_RCC_DMA1_CLK_DISABLE();
	__HAL_RCC_SPI1_CLK_DISABLE();
}

void _SPI_Start(void)
{
//	GPIO_InitTypeDef GPIO_InitStruct =
//		{0};

//	__HAL_RCC_GPIOA_CLK_ENABLE();
//	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();

	// SPI DMA Init
	// SPI_RX Init
//	_hdma_spi_rx.Instance = DMA1_Channel2;
//	_hdma_spi_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
//	_hdma_spi_rx.Init.PeriphInc = DMA_PINC_DISABLE;
//	_hdma_spi_rx.Init.MemInc = DMA_MINC_ENABLE;
//	_hdma_spi_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//	_hdma_spi_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
//	_hdma_spi_rx.Init.Mode = DMA_NORMAL;
//	_hdma_spi_rx.Init.Priority = DMA_PRIORITY_LOW;
//	HAL_DMA_Init(&_hdma_spi_rx);
//	__HAL_LINKDMA(&__SPI.public.hspi, hdmarx, _hdma_spi_rx);
//
//	// SPI1_TX Init
//	_hdma_spi_tx.Instance = DMA1_Channel3;
//	_hdma_spi_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
//	_hdma_spi_tx.Init.PeriphInc = DMA_PINC_DISABLE;
//	_hdma_spi_tx.Init.MemInc = DMA_MINC_ENABLE;
//	_hdma_spi_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
//	_hdma_spi_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
//	_hdma_spi_tx.Init.Mode = DMA_NORMAL;
//	_hdma_spi_tx.Init.Priority = DMA_PRIORITY_LOW;
//	HAL_DMA_Init(&_hdma_spi_tx);
//	__HAL_LINKDMA(&__SPI.public.hspi, hdmatx, _hdma_spi_tx);

//	GPIO_InitStruct.Pin = SPI_MISO_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
//	HAL_GPIO_Init(SPI_MISO_GPIO_Port, &GPIO_InitStruct);
//
//	GPIO_InitStruct.Pin = SPI_MOSI_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
//	HAL_GPIO_Init(SPI_MOSI_GPIO_Port, &GPIO_InitStruct);
//
//	GPIO_InitStruct.Pin = SPI_SCK_Pin;
//	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
//	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
//	HAL_GPIO_Init(SPI_SCK_GPIO_Port, &GPIO_InitStruct);

	// SPI peripheral init
	hspi.Instance = SPI1;
	hspi.Init.Mode = SPI_MODE_MASTER;  // we are the master
	hspi.Init.Direction = SPI_DIRECTION_2LINES;
	hspi.Init.DataSize = SPI_DATASIZE_8BIT;
	hspi.Init.CLKPolarity = SPI_POLARITY_LOW;  // SPI_POLARITY_LOW && SPI_PHASE_1EDGE == SPI mode 0
	hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi.Init.NSS = SPI_NSS_SOFT;
	hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi.Init.CRCPolynomial = 7;
	hspi.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
	HAL_SPI_Init(&hspi);
}

__spi_t                                              __SPI =  // instantiate event_handler_t actual and set function pointers
	{  //
	._CriticalSection = 0,  //
	};

void spi_ctor(void)
{
//	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 0);  // SPI DMA interrupts
//	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);	// SPI interrupts
	HAL_NVIC_EnableIRQ(SPI1_IRQn);

	_SPI_Start();
}

spi_t *const SPI = &__SPI.public;

//void SPIx_DMA_RX_IRQHandler(void)
//{
//	HAL_DMA_IRQHandler(__SPI.public.hspi.hdmarx);
//}
//
//void SPIx_DMA_TX_IRQHandler(void)
//{
//	HAL_DMA_IRQHandler(__SPI.public.hspi.hdmatx);
//}
//
////
//void DMA1_Channel2_3_IRQHandler(void)
//{
//	HAL_DMA_IRQHandler(&_hdma_spi_rx);
//	HAL_DMA_IRQHandler(&_hdma_spi_tx);
//}

//
void SPI1_IRQHandler(void)
{
	HAL_SPI_IRQHandler(&hspi);
}
#endif
