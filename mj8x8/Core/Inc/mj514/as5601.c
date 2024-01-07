#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "as5601.h"
#include "i2c/i2c.h"

#define AS5601_I2C_ADDR  (uint16_t) 0x6C // DS. p. 10 - AS5601 7-bit I2C address is 0x36, left-shifted 0x6C

typedef struct	// as5601c_t actual
{
	as5601_t public;  // public struct

	volatile uint16_t counter;  //
} __as5601_t;

extern TIM_HandleTypeDef htim3;  // rotary encoder handling
extern TIM_HandleTypeDef htim16;  // rotary encoder time-base

static volatile uint8_t _OVFcnt;

static __as5601_t __AS5601 __attribute__ ((section (".data")));  // preallocate __AS5601 object in .data

#define REG_CNT 11	// 10 registers

static const uint8_t _RegisterAddress[REG_CNT] =  // offset of each register address
	{  //
	0x00,  // ZMCO
	0x01,  // ZPOS
	0x07,  // CONF
	0x09,  // ABN
	0x0A,  // PUSHTHR
	0x0C,  // RAW_ANGLE
	0x0E,  // ANGLE
	0x0B,  // STATUS
	0x1A,  // AGC
	0x1B,  // MAGNITUDE
	0xFF,  // BURN
	};

static const uint8_t _RegisterSize[REG_CNT] =  // size of each register address
	{  // 1 or 2 bytes
	1,  // ZMCO
	2,  // ZPOS
	2,  // CONF
	1,  // ABN
	1,  // PUSHTHR
	2,  // RAW_ANGLE
	2,  // ANGLE
	1,  // STATUS
	1,  // AGC
	2,  // MAGNITUDE
	1,  // BURN
	};

static uint16_t _Read(const as5601_reg_t Register)
{
	uint16_t retval;
	I2C->Read(AS5601_I2C_ADDR, _RegisterAddress[Register], &retval, _RegisterSize[Register]);
	return retval;
}

static void _Write(const as5601_reg_t Register, const uint16_t *data)
{
	I2C->Write(AS5601_I2C_ADDR, _RegisterAddress[Register], data, _RegisterSize[Register]);
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

	// FIXME - start timer as reaction to command once functionality is implemented
	Device->StartTimer(&htim3);  // timer3 encoder handling and timer2 time base

	return &__AS5601.public;  // set pointer to AS5601 public part
}

//
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt
}

void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt
}

//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim3)
		++_OVFcnt;

	if(htim == &htim16)
		{
			__AS5601.public.Rotation = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3);	// FIXME - validate - 0 - CW, 1 - CCW, see rotation_t
			__AS5601.counter = __HAL_TIM_GET_COUNTER(&htim3);
		}
}

#endif
