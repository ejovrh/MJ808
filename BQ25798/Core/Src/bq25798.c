#include "main.h"
#include "bq25798.h"
#include <stdio.h>

typedef struct	// bq25798_t actual
{
	bq25798_t public;  // publicly visible struct

	uint16_t *__buffer;  // internal buffer for I2C device address and payload data (both r and w)
	I2C_HandleTypeDef *__hi2c;	// I2C HAL handler
} __bq25798_t;

static __bq25798_t __BQ25798 __attribute__ ((section (".data")));  // preallocate __Device object in .data

extern void Error_Handler(void);

uint16_t _buf[REG_CNT] =
	{0};  // internal array for register address & payload

static const uint8_t _RegOffset[REG_CNT] =  // offset of each register address
	{  //
	0x0, 0x1, 0x3, 0x5, 0x6, 0x8, 0x9, 0xA, 0xB, 0xD,  // REG00 - REG0D
	0xE, 0xF, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,  // REG0E - REG17
	0x18, 0x19, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22,  // REG18 - REG22
	0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C,  // REG23 - REG2C
	0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x33, 0x35, 0x37, 0x39, 0x3B,  // REG2D - REG3B
	0x3D, 0x3F, 0x41, 0x43, 0x45, 0x47, 0x48	// REG3D - REG48
	};

static const uint8_t _RegSize[REG_CNT] =  // size of each register address
	{  // 1 or 2 bytes
	1, 2, 2, 1, 2, 1, 1, 1, 2, 1,  // 01 - 10 -- REG 0x00 - REG 0x0D
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 11 - 20 -- REG 0x0E - REG 0x17
	1, 2, 1, 1, 1, 1, 1, 1, 1, 1,  // 21 - 30 -- REG 0x18 - REG 0x22
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 31 - 40 -- REG 0x23 - REG 0x2C
	1, 1, 1, 1, 2, 2, 2, 2, 2, 2,  // 41 - 50 -- REG 0x2D - REG 0x3B
	2, 2, 2, 2, 2, 1, 1  //						51 - 57 -- REG 0x3D - REG 0x48
	};

// wrapper for I2C read procedure
static void __ReadWrapper(uint16_t MemAddress, uint16_t MemAddSize, uint8_t *pData)
{
	do  // read from device
		{
			if(HAL_I2C_Mem_Read_DMA(__BQ25798.__hi2c, BQ25798_I2C_ADDR, MemAddress, MemAddSize, pData, MemAddSize) != HAL_OK)  // transmit onto the I2C bus
				Error_Handler();

			// see if the target device is ready
			while(HAL_I2C_IsDeviceReady(__BQ25798.__hi2c, BQ25798_I2C_ADDR, 5, 50) == HAL_TIMEOUT)
				;

			// see if the bus is ready
			while(HAL_I2C_GetState(__BQ25798.__hi2c) != HAL_I2C_STATE_READY)
				;  // wait for end of transfer

		}
	while(HAL_I2C_GetError(__BQ25798.__hi2c) == HAL_I2C_ERROR_AF);  // retry on acknowledge failure
}

// I2C read function
uint16_t _Read(const uint8_t RegAddr)
{
	uint8_t temp;  // holds a byte value

	if(_RegSize[RegAddr] == 1)  // 1 byte register value - reads uint8_t once
		{
			__ReadWrapper(_RegOffset[RegAddr], 1, &temp);  // read one byte from device and put into temp

			_buf[RegAddr] = temp;  // store in register buffer
		}

	if(_RegSize[RegAddr] == 2)	// 2 byte register value - reads 2 uint8_t in a row and constructs one uint16_t
		{
			uint16_t bb;	// holds a 2 byte value
			__ReadWrapper(_RegOffset[RegAddr], 1, &temp);  // read upper byte from device and put into temp
			bb = (temp << 8);  // lsh into upper uint16_t byte
			__ReadWrapper(_RegOffset[RegAddr] + 1, 1, &temp);  // read lower byte from device and put into temp
			bb |= temp;  // OR temp into lower uint16_t byte

			_buf[RegAddr] = bb;  // store in register buffer
		}

	return _buf[RegAddr];
}

#if UART_DUMP
// printout of all register values to uart
void _Print(void)
{
	for(uint8_t i = 0; i < REG_CNT; ++i)
		printf("%x ", __BQ25798.__buffer[i]);  // then print all the registers

	printf("%d %d %d ", __BQ25798.public.pg, __BQ25798.public.irq, __BQ25798.public.stat);	// first print charger status

	printf("\r\n");
}
#endif

// reads out the whole device register
void _Dump(void)
{
	for(uint8_t i = 0; i < REG_CNT; ++i)	// loop over all registers
		_buf[i] = _Read(i);  // and store in register buffer

#if UART_DUMP
	_Print();
#endif
}

// I2C write function
void _Write(const uint8_t RegAddr, const uint16_t val)
{
	do  // write to device
		{
			if(HAL_I2C_Mem_Write_DMA(__BQ25798.__hi2c, BQ25798_I2C_ADDR, _RegOffset[RegAddr], (uint16_t) _RegSize[RegAddr], (uint8_t*) &val, (uint16_t) _RegSize[RegAddr]) != HAL_OK)  // transmit onto the I2C bus
				Error_Handler();

			// see if the target device is ready
			while(HAL_I2C_IsDeviceReady(__BQ25798.__hi2c, BQ25798_I2C_ADDR, 5, 50) == HAL_TIMEOUT)
				;

			// see if the bus is ready
			while(HAL_I2C_GetState(__BQ25798.__hi2c) != HAL_I2C_STATE_READY)
				;		// wait for end of transfer

		}
	while(HAL_I2C_GetError(__BQ25798.__hi2c) == HAL_I2C_ERROR_AF);  // retry on acknowledge failure
}

bq25798_t* bq25798_ctor(I2C_HandleTypeDef *const in_hi2c)
{
#if UART_DUMP
	_Print();
#endif

	__BQ25798.__buffer = _buf;  // point array pointer to internal array
	__BQ25798.__hi2c = in_hi2c;  // HAL's I2C handler

	__BQ25798.public.Read = &_Read;  // I2C read method
	__BQ25798.public.Write = &_Write;  // I2C write method
	__BQ25798.public.Dump = &_Dump;  // sequential read of all device registers

	__BQ25798.public.Write(REG08, 0xc3);
	__BQ25798.public.Write(REG09, 0x03);  // configure
	__BQ25798.public.Write(REG10, (VBUS_BACKUP_80 | VAC_OVP_22V | _BV(WD_RST) | WATCHDOG_TIMER_1));  // configure
	__BQ25798.public.Write(REG0F, ( _BV(EN_AUTO_IBATDIS) | _BV(EN_CHG) | _BV(EN_ICO) | _BV(FORCE_ICO) | _BV(EN_BACKUP)));  // configure
	__BQ25798.public.Write(REG11, ( SDRV_CTRL_IDLE | _BV(SDRV_DLY)));  // TODO - REG11_Charger_Control_2 DP/DM detection
	__BQ25798.public.Write(REG12, (0x00));  // configure
	__BQ25798.public.Write(REG13, ( _BV(EN_ACDRV2) | _BV(EN_ACDRV1) | _BV(PWM_FREQ) | _BV(EN_IBUS_OCP)));  // configure
	__BQ25798.public.Write(REG14, ( _BV(SFET_PRESENT) | _BV(EN_IBAT) | IBAT_REG_3A | _BV(EN_IINDPM)));  // configure
	__BQ25798.public.Write(REG15, (_BV(EN_MPPT) | VOC_RATE_2MIN | VOC_DLY_50MS | VOC_PCT_0875));  //
//	__BQ25798.public.Write(REG18, (TS_COOL_10DEG | TS_WARM_55DEG | BHOT_65DEG));
	__BQ25798.public.Write(REG2E, (_BV(ADC_EN) | _BV(ADC_AVG_INIT)));

	HAL_GPIO_WritePin(BQ25798_CE_GPIO_Port, BQ25798_CE_Pin, GPIO_PIN_RESET);
//	HAL_Delay(500);
	HAL_GPIO_WritePin(BQ25798_CE_GPIO_Port, BQ25798_CE_Pin, GPIO_PIN_SET);
//	HAL_Delay(500);
	HAL_GPIO_WritePin(BQ25798_CE_GPIO_Port, BQ25798_CE_Pin, GPIO_PIN_RESET);

	return (&__BQ25798.public);
}

