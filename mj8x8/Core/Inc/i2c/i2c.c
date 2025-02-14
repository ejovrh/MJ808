#include "main.h"

#if USE_I2C
#include "i2c.h"

#if USE_DMA
static DMA_HandleTypeDef _hdma_i2c1_rx;  // I2C RX DMA handle
static DMA_HandleTypeDef _hdma_i2c1_tx;  // I2C TX DMA handle
#endif
static I2C_HandleTypeDef _hi2c;  //I2C handle

typedef struct	// i2c_t actual
{
	i2c_t public;  // public struct

	I2C_HandleTypeDef *_I2C;  // I2C handle
	uint8_t _FlagPowerState :1; 	// I2C is powered on
	uint32_t _SDA_Pin;
	uint32_t _SCL_Pin;
	GPIO_TypeDef *_I2C_Port;
} __i2c_t;

__i2c_t __I2C __attribute__ ((section (".data")));

#define WRITE 0x00
#define READ 0x01

#define USE_WAIT_FOR_DEVICE_READY 0 //
#define USE_DMA 0 // use DMA transfer functions
#define USE_IT 1 // use interrupt transfer functions
#define USE_POLLING 0 // use polling transfer functions

#define FREQ_100KHZ 1
#define FREQ_400KHZ 0
#define FREQ_1MHZ 0

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
	while(HAL_I2C_GetState(__I2C._I2C) != HAL_I2C_STATE_READY)
		;
}

#if USE_WAIT_FOR_DEVICE_READY
// check I2C bus and device for readiness
static inline void _BusDeviceCheck(const uint16_t DevAddr)
{
	_BusCheck();  // check if I2C is ready

	//	check if the device is ready
	while(HAL_I2C_IsDeviceReady(__I2C._I2C, DevAddr, 2, 10) != HAL_OK)
		;
}
#endif

// Initialize I2C peripheral
static inline void _I2C_Init(const uint32_t _SDA_Pin, const uint32_t _SCL_Pin, GPIO_TypeDef *_I2C_Port)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit =
		{0};

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

#if USE_DMA
	__HAL_RCC_DMA1_CLK_ENABLE();
#endif

	_hi2c.Instance = I2C1;
#if FREQ_100KHZ
	_hi2c.Init.Timing = 0x00201D2B;  /// 100kHz
#endif
#if FREQ_400KHZ
	_hi2c.Init.Timing = 0x0000020A;  // 400kHz - fast mode
#endif
#if FREQ_1MHZ
	_hi2c.Init.Timing = 0x00100001;	// 1MHz - fast mode plus
#endif
	_hi2c.Init.OwnAddress1 = 0;
	_hi2c.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	_hi2c.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	_hi2c.Init.OwnAddress2 = 0;
	_hi2c.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	_hi2c.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	_hi2c.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	HAL_I2C_Init(&_hi2c);

	__HAL_RCC_GPIOF_CLK_ENABLE();  // enable peripheral clock
	GPIO_InitStruct.Pin = _SDA_Pin | _SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
	HAL_GPIO_Init(_I2C_Port, &GPIO_InitStruct);

	__HAL_RCC_I2C1_CLK_ENABLE();

#if USE_DMA
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
#endif

	HAL_I2CEx_ConfigAnalogFilter(&_hi2c, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&_hi2c, 0);

#if FREQ_1MHZ
	__HAL_SYSCFG_FASTMODEPLUS_ENABLE(I2C_FASTMODEPLUS_I2C1);	// needed for fast mode plus
#endif
}

// power off the I2C infrastructure
static void _I2CPowerOff(void)
{
	if(__I2C._FlagPowerState == OFF)
		return;

	__HAL_RCC_I2C1_CLK_DISABLE();  // disable I2C peripheral clock
	__HAL_RCC_GPIOF_CLK_DISABLE();
#if USE_DMA
//	__HAL_RCC_DMA1_CLK_DISABLE();
#endif

	__I2C._FlagPowerState = OFF;  // flag as off
}

// power on the I2C infrastructure
static void _I2CPowerOn(void)
{
	if(__I2C._FlagPowerState == ON)
		return;

	__HAL_RCC_GPIOF_CLK_ENABLE();  // enable peripheral clock
	__HAL_RCC_I2C1_CLK_ENABLE();
#if USE_DMA
//	__HAL_RCC_DMA1_CLK_ENABLE();
#endif

	_I2C_Init(__I2C._SDA_Pin, __I2C._SCL_Pin, __I2C._I2C_Port);  // initialize I2C infrastructure

	__I2C._FlagPowerState = ON;  // flag as on
}

// I2C mem. read n bytes from device into *buffer
static inline void _Read(const uint16_t DevAddr, const uint16_t RegAddr, uint8_t *buffer, const uint8_t n)
{
	_I2CPowerOn();  // I2C power on

#if USE_WAIT_FOR_DEVICE_READY
	_BusDeviceCheck(DevAddr);  // check if the device is ready)
#endif

#if USE_DMA
	if(HAL_I2C_Mem_Read_DMA(__I2C.public.I2C, (DevAddr | READ), RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, size) != HAL_OK)
#endif
#if USE_IT
	if(HAL_I2C_Mem_Read_IT(__I2C._I2C, (DevAddr | READ), RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, n) != HAL_OK)
#endif
#if USE_POLLING
	if(HAL_I2C_Mem_Read(__I2C._I2C, (DevAddr | READ), RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, n, 2) != HAL_OK)
#endif
		_Error_Handler();

	_BusCheck();  // wait for the read to complete
	_I2CPowerOff();  // I2C power off
}

// I2C mem. write n bytes from *buffer into device
static inline void _Write(const uint16_t DevAddr, const uint16_t RegAddr, uint8_t *buffer, const uint8_t n)
{
	_I2CPowerOn();  // I2C power on

#if USE_WAIT_FOR_DEVICE_READY
	_BusDeviceCheck(DevAddr);  // check if the device is ready)
#endif

#if USE_DMA
	if(HAL_I2C_Mem_Write_DMA(__I2C.public.I2C, DevAddr, RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, size) != HAL_OK)
#endif
#if USE_IT
	if(HAL_I2C_Mem_Write_IT(__I2C._I2C, DevAddr, RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, n) != HAL_OK)
#endif
#if USE_POLLING
	if(HAL_I2C_Mem_Write(__I2C._I2C, DevAddr, RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, n, 2) != HAL_OK)
#endif
		_Error_Handler();

	_BusCheck();  // wait for the read to complete
	_I2CPowerOff();  // I2C power off
}

// I2C master TX of up to 4 bytes of data into device
static inline void _Transmit(const uint16_t DevAddr, uint8_t *buffer, const uint8_t n)
{
	_I2CPowerOn();  // I2C power on

#if USE_WAIT_FOR_DEVICE_READY
	_BusDeviceCheck(DevAddr);  // check if the device is ready)
#endif

#if USE_DMA
	if(HAL_I2C_Master_Transmit_DMA(__I2C.public.I2C, DevAddr, buffer, size) != HAL_OK)
#endif
#if USE_IT
	if(HAL_I2C_Master_Transmit_IT(__I2C._I2C, DevAddr, buffer, n) != HAL_OK)
#endif
#if USE_POLLING
	if(HAL_I2C_Master_Transmit(__I2C._I2C, DevAddr, buffer, n, 2) != HAL_OK)
#endif
		_Error_Handler();

	_BusCheck();  // wait for the write to complete
	_I2CPowerOff();  // I2C power off
}

// I2C master RX of up to 4 bytes of data from device
static inline void _Receive(const uint16_t DevAddr, uint8_t *buffer, const uint8_t n)
{
	_I2CPowerOn();  // I2C power on

#if USE_WAIT_FOR_DEVICE_READY
	_BusDeviceCheck(DevAddr);  // check if the device is ready)
#endif

#if USE_DMA
	if(HAL_I2C_Master_Receive_DMA(__I2C.public.I2C, DevAddr, buffer, size) != HAL_OK)
#endif
#if USE_IT
	if(HAL_I2C_Master_Receive_IT(__I2C._I2C, DevAddr, buffer, n) != HAL_OK)
#endif
#if USE_POLLING
	if(HAL_I2C_Master_Receive(__I2C._I2C, DevAddr, buffer, n, 2) != HAL_OK)
#endif
		_Error_Handler();

	_BusCheck();  // wait for the write to complete
	_I2CPowerOff();  // I2C power off
}

__i2c_t __I2C =  // instantiate event_handler_t actual and set function pointers
	{  //
	.public.Read = &_Read,  //
	.public.Write = &_Write,  //
	.public.Transmit = &_Transmit,  //
	.public.Receive = &_Receive,  //
	};

i2c_t* i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port)
{
	__I2C._I2C = &_hi2c;
	__I2C._SDA_Pin = SDA_Pin;
	__I2C._SCL_Pin = SCL_Pin;
	__I2C._I2C_Port = I2C_Port;

#if USE_IT
	HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);	// I2C interrupts
	HAL_NVIC_EnableIRQ(I2C1_IRQn);
#endif

	__I2C._FlagPowerState = OFF;  // flag as off

#if USE_DMA
	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);  // I2C DMA interrupts
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
#endif

	return &__I2C.public;
}

#if USE_DMA
//
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&_hdma_i2c1_tx);
	HAL_DMA_IRQHandler(&_hdma_i2c1_rx);
}
#endif

#if USE_IT
//
void I2C1_IRQHandler(void)
{
	if(_hi2c.Instance->ISR & (I2C_FLAG_BERR | I2C_FLAG_ARLO | I2C_FLAG_OVR))
		HAL_I2C_ER_IRQHandler(&_hi2c);
	else
		HAL_I2C_EV_IRQHandler(&_hi2c);
}
#endif
#endif
