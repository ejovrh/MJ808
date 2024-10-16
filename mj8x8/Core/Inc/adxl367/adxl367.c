#include "main.h"

#if defined(USE_ADXL367)	// if this particular device is active

#include "adxl367/adxl367.h"
#include "spi/spi.h"

typedef struct	// adxl367c_t actual
{
	adxl367_t public;  // public struct

	GPIO_TypeDef *_CS_Port;  // SPI chip select GPIO port
	uint32_t _CS_Pin;  // SPI chip select GPIO pin
} __adxl367_t;

extern SPI_HandleTypeDef hspi;

static __adxl367_t                                                           __ADXL367                                                           __attribute__ ((section (".data")));  // preallocate __ADXL367 object in .data

#define REG_CNT 64	// 64 registers
#define COMMAND_WRITE 0x0A	// SPI write command, DS. p. 27
#define COMMAND_READ 0x0B	// SPI read command, DS. p. 27
#define COMMAND_READ_FIFO 0x0D	// SPI read FIFO command, DS. p. 27

static const uint8_t _RegisterAddress[REG_CNT] =  // offset of each register address
	{  //
	0x00,  // DEVID_AD
	0x01,  // DEVID_MST
	0x02,  // PART_ID
	0x03,  // REV_ID
	0x04,  // SERIAL_NUMBER_3
	0x05,  // SERIAL_NUMBER_2
	0x06,  // SERIAL_NUMBER_1
	0x07,  // SERIAL_NUMBER_0
	0x08,  // XDATA
	0x09,  // YDATA
	0x0A,  // ZDATA
	0x0B,  // STATUS
	0x0C,  // FIFO_ENTRIES_L
	0x0D,  // FIFO_ENTRIES_H
	0x0E,  // XDATA_H
	0x0F,  // XDATA_L
	0x10,  // YDATA_H
	0x11,  // YDATA_L
	0x12,  // ZDATA_H
	0x13,  // ZDATA_L
	0x14,  // TEMP_H
	0x15,  // TEMP_L
	0x16,  // EX_ADC_H
	0x17,  // EX_ADC_L
	0x18,  // I2C_FIFO_DATA
	0x1F,  // SOFT_RESET
	0x20,  // THRESH_ACT_H
	0x21,  // THRESH_ACT_L
	0x22,  // TIME_ACT
	0x23,  // THRESH_INACT_H
	0x24,  // THRESH_INACT_L
	0x25,  // TIME_INACT_H
	0x26,  // TIME_INACT_L
	0x27,  // ACT_INACT_CTL
	0x28,  // FIFO_CONTROL
	0x29,  // FIFO_SAMPLES
	0x2A,  // INTMAP1_LOWER
	0x2B,  // INTMAP2_LOWER
	0x2C,  // FILTER_CTL
	0x2D,  // POWER_CTL
	0x2E,  // SELF_TEST
	0x2F,  // TAP_THRESH
	0x30,  // TAP_DUR
	0x31,  // TAP_LATENT
	0x32,  // TAP_WINDOW
	0x33,  // X_OFFSET
	0x34,  // Y_OFFSET
	0x35,  // Z_OFFSET
	0x36,  // X_SENS
	0x37,  // Y_SENS
	0x38,  // Z_SENS
	0x39,  // TIMER_CTL
	0x3A,  // INTMAP1_UPPER
	0x3B,  // INTMAP2_UPPER
	0x3C,  // ADC_CTL
	0x3D,  // TEMP_CTL
	0x3E,  // TEMP_ADC_OVER_THRSH_H
	0x3F,  // TEMP_ADC_OVER_THRSH_L
	0x40,  // TEMP_ADC_UNDER_THRSH_H
	0x41,  // TEMP_ADC_UNDER_THRSH_L
	0x42,  // TEMP_ADC_TIMER
	0x43,  // AXIS_MASK
	0x44,  // STATUS_COPY
	0x45,  // STATUS_2
	};

//
static void _Read(const adxl367_reg_t in_register, uint8_t *data, const uint8_t len)
{
	volatile uint8_t retval = 0;
	uint8_t command = COMMAND_READ;
	uint8_t address = _RegisterAddress[in_register];

	HAL_GPIO_WritePin(__ADXL367._CS_Port, __ADXL367._CS_Pin, GPIO_PIN_RESET);  // select the slave

//	retval = HAL_SPI_TransmitReceive_DMA(&SPI->hspi, &command, data, 1);  // send read command
	retval = HAL_SPI_Transmit_IT(&hspi, &command, 1);  // send read command
	while(HAL_SPI_GetState(&hspi) != HAL_SPI_STATE_READY)
		;

//	retval = HAL_SPI_TransmitReceive_DMA(&SPI->hspi, &address, data, 1);  // send register address to read from
	retval = HAL_SPI_Transmit_IT(&hspi, &address, 1);  // send register address to read from
	while(HAL_SPI_GetState(&hspi) != HAL_SPI_STATE_READY)
		;

	retval = HAL_SPI_Receive_IT(&hspi, data, len);  // send register address to read from

	HAL_GPIO_WritePin(__ADXL367._CS_Port, __ADXL367._CS_Pin, GPIO_PIN_SET);  // de-select the slave
}

//
static void _ReadFIFO(void)
{
	;
}

//
static void _Write(const adxl367_reg_t in_register, uint8_t *data, const uint8_t len)
{
	uint8_t command = COMMAND_WRITE;
	uint8_t address = _RegisterAddress[in_register];

	HAL_GPIO_WritePin(__ADXL367._CS_Port, __ADXL367._CS_Pin, GPIO_PIN_RESET);  // select the slave

	HAL_SPI_TransmitReceive_DMA(&hspi, &command, data, 1);  // send write command
	HAL_SPI_TransmitReceive_DMA(&hspi, &address, data, len);  // send register address to read from

	HAL_GPIO_WritePin(__ADXL367._CS_Port, __ADXL367._CS_Pin, GPIO_PIN_SET);  // de-select the slave
}

static __adxl367_t                                                          __ADXL367 =  // instantiate adxl367c_t actual and set function pointers
	{  //
	.public.ReadFIFO = &_ReadFIFO,	//
	.public.Read = &_Read,	//
	.public.Write = &_Write  //
	};

adxl367_t* adxl367_ctor(GPIO_TypeDef *_SPI_CS_Port, const uint32_t _SPI_CS_Pin)  //
{
	__ADXL367._CS_Port = _SPI_CS_Port;	//
	__ADXL367._CS_Pin = _SPI_CS_Pin;  //

	HAL_GPIO_WritePin(__ADXL367._CS_Port, __ADXL367._CS_Pin, GPIO_PIN_SET);  // chip select is active low

	return &__ADXL367.public;  // set pointer to ADXL367 public part
}

void EXTI0_1_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(ADXL367_INT1_Pin);

	Device->automotion->Do();  //
}

#endif
