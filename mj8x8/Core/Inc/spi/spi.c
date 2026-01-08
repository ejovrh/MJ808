#include "main.h"

#if USE_SPI
#include "spi.h"

#if USE_DMA
static DMA_HandleTypeDef _hdma_i2c1_rx;  // I2C RX DMA handle
static DMA_HandleTypeDef _hdma_i2c1_tx;  // I2C TX DMA handle
#endif
SPI_HandleTypeDef _hspi1;  // SPI handle

typedef struct	// spi_t actual
{
	spi_t public;  // public struct

	SPI_HandleTypeDef *_SPI;  // SPI handle
	uint8_t _FlagPowerState :1; 	// SPI is powered on
	uint32_t _MOSI_Pin;
	uint32_t _MISO_Pin;
	uint32_t _SCL_Pin;
	uint32_t _CS_Pin;
	GPIO_TypeDef *_SPI_Port;
} __spi_t;

__spi_t __SPI __attribute__ ((section (".data")));

#define USE_BUSCHECK 0 // use bus check function
#define USE_DMA 0 // use DMA transfer functions
#define USE_IT 0 // use interrupt transfer functions
#define USE_POLLING 1 // use polling transfer functions
#define TIMEOUT 5  // timeout in ms for I2C polling operations

// FIXME - identified a bug/issue when SPI is powered off. a power cycle introduces delays into the whole system (visible in timer16cntr delay)
// perhaps power on SPI in advance - e.g. when the ringbuffer is about 128 free (atm it starts to write when it is less than 64 free)
#define USE_SPI_POWER_MANAGEMENT 0 // power off -> on -> off around each transaction

static inline void _Error_Handler(void)
{
	__disable_irq();
	while(1)
		;
}

#if USE_BUSCHECK
// check I2C bus and device for readiness
static inline void _BusCheck(void)
{
	// check if I2C is ready
	while(HAL_SPI_GetState(__SPI._SPI) != HAL_SPI_STATE_READY)
		;
}
#endif

// Initialize SPI peripheral
static inline void _SPI_Init(const uint32_t _MOSI_pin, const uint32_t _MISO_pin, const uint32_t _SCL_Pin, GPIO_TypeDef *_SPI_Port)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	_hspi1.Instance = SPI1;
	_hspi1.Init.Mode = SPI_MODE_MASTER;
	_hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	_hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
	_hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	_hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	_hspi1.Init.NSS = SPI_NSS_SOFT;
	_hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	_hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	_hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	_hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	_hspi1.Init.CRCPolynomial = 7;
	_hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	_hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	HAL_SPI_Init(&_hspi1);

	GPIO_InitStruct.Pin = _MOSI_pin | _MISO_pin | _SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
	HAL_GPIO_Init(_SPI_Port, &GPIO_InitStruct);

	__HAL_RCC_SPI1_CLK_ENABLE();
}

// power off the SPI infrastructure
static void _SPIPowerOff(void)
{
	if(__SPI._FlagPowerState == OFF)
		return;

#if USE_BUSCHECK
	_BusCheck();  // ensure SPI is ready
#endif

	__HAL_RCC_SPI1_CLK_DISABLE();  // disable I2C peripheral clock
#if USE_DMA
//	__HAL_RCC_DMA1_CLK_DISABLE();
#endif

	__SPI._FlagPowerState = OFF;  // flag as off
}

// power on the SPI infrastructure
static void _SPIPowerOn(void)
{
	if(__SPI._FlagPowerState == ON)
		return;

	__HAL_RCC_SPI1_CLK_ENABLE();
#if USE_DMA
//	__HAL_RCC_DMA1_CLK_ENABLE();
#endif

	_SPI_Init(__SPI._MOSI_Pin, __SPI._MISO_Pin, __SPI._SCL_Pin, __SPI._SPI_Port);  // initialize SPI infrastructure

#if USE_BUSCHECK
	_BusCheck();  // ensure SPI is ready
#endif

	__SPI._FlagPowerState = ON;  // flag as on
}

// SPI TX of up to n bytes of data (Polling)
static inline void _Transmit(uint8_t *txData, const uint16_t n)
{
	HAL_StatusTypeDef result = HAL_OK;

#if USE_SPI_POWER_MANAGEMENT
	_SPIPowerOn();  // SPI power on
#endif

#if USE_DMA
	result = HAL_SPI_Transmit_DMA(__SPI._SPI, txData, n);
#endif
#if USE_IT
	result = HAL_SPI_Transmit_IT(__SPI._SPI, txData, n);
#endif
#if USE_POLLING
	result = HAL_SPI_Transmit(__SPI._SPI, txData, n, TIMEOUT);
#endif

	if(result != HAL_OK)
		_Error_Handler();

#if USE_SPI_POWER_MANAGEMENT
	_SPIPowerOff();  // SPI power off
#endif
}

// SPI RX of up to n bytes of data (Polling)
static inline void _Receive(uint8_t *rxData, const uint16_t n)
{
	HAL_StatusTypeDef result = HAL_OK;

#if USE_SPI_POWER_MANAGEMENT
	_SPIPowerOn();  // SPI power on
#endif

#if USE_DMA
	result = HAL_SPI_Receive_DMA(__SPI._SPI, rxData, n);
#endif
#if USE_IT
	result = HAL_SPI_Receive_IT(__SPI._SPI, rxData, n);
#endif
#if USE_POLLING
	result = HAL_SPI_Receive(__SPI._SPI, rxData, n, TIMEOUT);
#endif

	if(result != HAL_OK)
		_Error_Handler();

#if USE_SPI_POWER_MANAGEMENT
	_SPIPowerOff();  // SPI power off
#endif
}

// SPI TX/RX full-duplex (Polling)
static inline void _TransmitReceive(uint8_t *txData, uint8_t *rxData, const uint16_t n)
{
	HAL_StatusTypeDef result = HAL_OK;

#if USE_SPI_POWER_MANAGEMENT
	_SPIPowerOn();  // SPI power on
#endif

#if USE_DMA
	result = HAL_SPI_TransmitReceive_DMA(__SPI._SPI, txData, rxData, n);
#endif
#if USE_IT
	result = HAL_SPI_TransmitReceive_IT(__SPI._SPI, txData, rxData, n);
#endif
#if USE_POLLING
	result = HAL_SPI_TransmitReceive(__SPI._SPI, txData, rxData, n, TIMEOUT);
#endif

	if(result != HAL_OK)
		_Error_Handler();

#if USE_SPI_POWER_MANAGEMENT
	_SPIPowerOff();  // SPI power off
#endif
}

__spi_t __SPI =  // instantiate event_handler_t actual and set function pointers
	{  //
	.public.Transmit = &_Transmit,  // SPI TX of up to n bytes of data
	.public.Receive = &_Receive,  // SPI RX of up to n bytes of data
	.public.TransmitReceive = &_TransmitReceive,  // SPI TX/RX full-duplex
	};

spi_t* spi_ctor(const uint32_t MOSI_Pin, const uint32_t MISO_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *SPI_Port)
{
	__SPI._SPI = &_hspi1;
	__SPI._MISO_Pin = MISO_Pin;
	__SPI._MOSI_Pin = MOSI_Pin;
	__SPI._SCL_Pin = SCL_Pin;
	__SPI._SPI_Port = SPI_Port;

#if USE_IT
	HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);	// SPI interrupts
	HAL_NVIC_EnableIRQ(SPI1_IRQn);
#endif

	__SPI._FlagPowerState = OFF;  // flag as off

#if !USE_SPI_POWER_MANAGEMENT
	_SPIPowerOn();	// TODO - remove
#endif

#if USE_DMA
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);  // SPI DMA interrupts
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
#endif

	return &__SPI.public;
}

#if USE_DMA
//
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&_hdma_hspi1_tx);
	HAL_DMA_IRQHandler(&_hdma_hspi1_rx);
}
#endif

#if USE_IT
void SPI1_IRQHandler(void)
{
	HAL_SPI_IRQHandler(__SPI._SPI);
}
#endif

#endif // USE_SPI
