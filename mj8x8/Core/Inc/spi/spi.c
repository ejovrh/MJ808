#include "main.h"

#if USE_SPI
#include "spi.h"

#if USE_DMA
static DMA_HandleTypeDef _hdma_i2c1_rx;  // I2C RX DMA handle
static DMA_HandleTypeDef _hdma_i2c1_tx;  // I2C TX DMA handle
#endif
static SPI_HandleTypeDef _hspi1;  // SPI handle

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

#define WRITE 0x00
#define READ 0x01

#define USE_WAIT_FOR_DEVICE_READY 0 //
#define USE_DMA 0 // use DMA transfer functions
#define USE_IT 1 // use interrupt transfer functions
#define USE_POLLING 0 // use polling transfer functions

#define FREQ_100KHZ 0
#define FREQ_400KHZ 0
#define FREQ_1MHZ 1

#if USE_IT
//void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//	(void) hi2c;
//	__I2C.public.TXDone = 1;
//}
//
//void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
//{
//	(void) hi2c;
//	__I2C.public.RXDone = 1;
//}
#endif

static inline void _Error_Handler(void)
{
	__disable_irq();
	while(1)
		;
}

// check I2C bus and device for readiness
static inline void _BusCheck(void)
{
	// check if I2C is ready
	while(HAL_SPI_GetState(__SPI._SPI) != HAL_SPI_STATE_READY)
		;
}

#if USE_WAIT_FOR_DEVICE_READY
// check SPI bus and device for readiness
static inline void _BusDeviceCheck(const uint16_t DevAddr)
{
	_BusCheck();  // check if I2C is ready

	//	check if the device is ready
	while(HAL_I2C_IsDeviceReady(__I2C._I2C, DevAddr, 2, 10) != HAL_OK)
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

	__SPI._FlagPowerState = ON;  // flag as on
}

// SPI TX of up to n bytes of data
static inline void _Transmit(const uint16_t DevAddr, uint8_t *buffer, const uint8_t n)
{
	_SPIPowerOn();  // SPI power on

#if USE_WAIT_FOR_DEVICE_READY
	_BusDeviceCheck(DevAddr);  // check if the device is ready)
#endif

#if USE_DMA

#endif
#if USE_IT

#endif
#if USE_POLLING

#endif
	_Error_Handler();

	_BusCheck();  // wait for the write to complete
	_SPIPowerOff();  // I2C power off
}

// SPI RX of up to n bytes of data
static inline void _Receive(const uint16_t DevAddr, uint8_t *buffer, const uint8_t n)
{
	_SPIPowerOn();  // SPI power on

#if USE_WAIT_FOR_DEVICE_READY
	_BusDeviceCheck(DevAddr);  // check if the device is ready)
#endif

#if USE_DMA

#endif
#if USE_IT

#endif
#if USE_POLLING

#endif
	_Error_Handler();

	_BusCheck();  // wait for the write to complete
	_SPIPowerOff();  // I2C power off
}

__spi_t __SPI =  // instantiate event_handler_t actual and set function pointers
	{  //
	.public.Transmit = &_Transmit,  // SPI TX of up to n bytes of data
	.public.Receive = &_Receive,  // SPI RX of up to n bytes of data
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
//
void SPI1_IRQHandler(void)
{
	HAL_SPI_IRQHandler(__SPI._SPI);
}
#endif
#endif
