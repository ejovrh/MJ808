#include "main.h"

#if USE_I2C
#include "i2c.h"

static DMA_HandleTypeDef _hdma_i2c1_rx;  // I2C RX DMA handle
static DMA_HandleTypeDef _hdma_i2c1_tx;  // I2C TX DMA handle
static I2C_HandleTypeDef _hi2c;  //I2C handle

typedef struct	// i2c_t actual
{
	i2c_t public;  // public struct

	uint8_t _FlagPoweredOn :1;	// flag if I2C is already powered on
} __i2c_t;

__i2c_t __I2C __attribute__ ((section (".data")));

#define WRITE 0x00
#define READ 0x01

inline void Error_Handler(void)
{
	__disable_irq();
	while(1)
		;
}

// power off the I2C infrastructure
static void _Stop(void)
{
	if(__I2C._FlagPoweredOn == 0)
		return;

	// give the bus time to settle
	while(HAL_I2C_GetState(Device->mj8x8->i2c->I2C) != HAL_I2C_STATE_READY)
		;

	// TODO - account for ADC DMA running
	__HAL_RCC_I2C1_CLK_DISABLE();// enable peripheral clocks
	__HAL_RCC_GPIOB_CLK_DISABLE();
//	__HAL_RCC_DMA1_CLK_DISABLE();

	__I2C._FlagPoweredOn = 0;  // flag for on state
}

// power on the I2C infrastructure
static void _Start(void)
{
	if(__I2C._FlagPoweredOn == 1)
		return;

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock
	__HAL_RCC_I2C1_CLK_ENABLE();
//	__HAL_RCC_DMA1_CLK_ENABLE();

	__I2C._FlagPoweredOn = 1;  // flag for on state
}

// I2C read of up to 4 bytes of data from device
uint32_t _Read(const uint16_t DevAddr, const uint16_t RegAddr, const uint8_t size)
{
	if(size > 4)
		return 0xBEEF;

	if(size == 0)
		return 0xFEEB;

	uint32_t retval = 0;
	uint8_t buffer[4] =
		{0};

	_Start();  // I2C power on

	//	check if the device is ready
	while(HAL_I2C_IsDeviceReady(__I2C.public.I2C, DevAddr, 2, 10) != HAL_OK)
		;

	// send device address w. read command, address we want to read from, along with how many bytes to read
	if(HAL_I2C_Mem_Read_DMA(Device->mj8x8->i2c->I2C, (DevAddr | READ), RegAddr, I2C_MEMADD_SIZE_8BIT, buffer, size) != HAL_OK)
		Error_Handler();

	_Stop();  // I2C power off

	for(uint8_t i = 0; i < size; i++)
		retval = (retval << 8) | buffer[i];

	return retval;
}

// I2C write of up to 4 bytes of data into device
void _Write(const uint16_t DevAddr, const uint16_t RegAddr, const uint32_t data, const uint8_t size)
{
	if(size > 4)
		return;

	if(size == 0)
		return;

	uint8_t buffer[5];
	uint32_t tmp = data;

	buffer[0] = RegAddr;

	// Fill the buffer based on the size
	for(uint8_t i = 0; i < size; i++)
		buffer[i + 1] = (uint8_t) (tmp >> (8 * (size - 1 - i)));  // Extract each byte in the correct order

	_Start();  // I2C power on

	//	check if the device is ready
	while(HAL_I2C_IsDeviceReady(__I2C.public.I2C, DevAddr, 2, 10) != HAL_OK)
		;

	//
	if(HAL_I2C_Master_Transmit_DMA(Device->mj8x8->i2c->I2C, DevAddr, buffer, size + 1) != HAL_OK)
		Error_Handler();

	_Stop();  // I2C power off
}

//
void _I2C_Init(const uint32_t _SDA_Pin, const uint32_t _SCL_Pin, GPIO_TypeDef *_I2C_Port)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit =
		{0};

	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__HAL_RCC_DMA1_CLK_ENABLE();

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

	__HAL_RCC_GPIOB_CLK_ENABLE();  // enable peripheral clock
	GPIO_InitStruct.Pin = _SDA_Pin | _SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF1_I2C1;
	HAL_GPIO_Init(_I2C_Port, &GPIO_InitStruct);

	__HAL_RCC_I2C1_CLK_ENABLE();

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

	HAL_I2CEx_ConfigAnalogFilter(&_hi2c, I2C_ANALOGFILTER_ENABLE);
	HAL_I2CEx_ConfigDigitalFilter(&_hi2c, 0);

	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_I2C1_CLK_DISABLE();

	__I2C._FlagPoweredOn = 0;
}

__i2c_t __I2C =  // instantiate event_handler_t actual and set function pointers
	{  //
	.public.I2C = &_hi2c,  //
	.public.Read = &_Read,  //
	.public.Write = &_Write,  //
	._FlagPoweredOn = 0,	//
	};

i2c_t* i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port)
{
	_I2C_Init(SDA_Pin, SCL_Pin, I2C_Port);	// initialize I2C infrastructure

	HAL_NVIC_SetPriority(I2C1_IRQn, 0, 0);	// I2C interrupts
	HAL_NVIC_EnableIRQ(I2C1_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 0, 0);  // I2C DMA interrupts
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	return &__I2C.public;
}

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
		HAL_I2C_ER_IRQHandler(&_hi2c);
	else
		HAL_I2C_EV_IRQHandler(&_hi2c);
}
#endif
