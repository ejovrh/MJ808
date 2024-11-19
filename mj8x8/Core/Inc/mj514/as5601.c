#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "as5601.h"

#define AS5601_I2C_ADDR  (uint16_t) 0x6C // DS. p. 10 - AS5601 7-bit I2C address is 0x36, left-shifted 0x6C

typedef struct	// as5601c_t actual
{
	as5601_t public;  // public struct

	uint8_t _status;	// STATUS register
	uint16_t _conf;  // CONF register
	uint8_t _abn;  // ABN register
	volatile uint16_t counter;  // TODO - implement rotation counting or whatever
} __as5601_t;

extern TIM_HandleTypeDef htim3;  // timer in encoder mode for rotation detection
extern TIM_HandleTypeDef htim16;  // rotary encoder time base - 10ms

static volatile uint8_t _OVFcnt;	// TODO - implement overflow handling
volatile uint16_t _CurrentRawAngle;
volatile uint16_t _PreviousRawAngle;

static __as5601_t __AS5601 __attribute__ ((section (".data")));  // preallocate __AS5601 object in .data

#define REG_CNT 11	// 11 registers
#define WRITE 0x00	// I2C write bit
#define READ 0x01	// I2C read bit
#define MD 5	// status register, MD bit - DS. p. 20

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

// returns 2 bytes of register values from device register
static inline uint16_t _Read(const as5601_reg_t Register)
{
	return (uint16_t) Device->mj8x8->i2c->Read(AS5601_I2C_ADDR, _RegisterAddress[Register], _RegisterSize[Register]);
}

// writes 2 bytes of data into device register
static inline void _Write(const as5601_reg_t Register, const uint16_t data)
{
	Device->mj8x8->i2c->Write(AS5601_I2C_ADDR, _RegisterAddress[Register], data, _RegisterSize[Register]);
}

// starts timers needed for encoder operation
void _Start(void)
{
	Device->StartTimer(&htim16);	// start timebase and encoder timer
}

// stops timers needed for encoder operation
void _Stop(void)
{
	Device->StopTimer(&htim16);  // stop timebase and encoder timer
}

// TODO - implement rotation counting or whatever
static float _CountRotation(void)
{
	return (__AS5601.counter / FULL_REVOLUTION );
}

static __as5601_t __AS5601 =  // instantiate as5601c_t actual and set function pointers
	{  //
	.public.CountRotation = &_CountRotation,  // set function pointer
	.public.Read = &_Read,	// ditto
	.public.Write = &_Write,  //
	.public.Start = &_Start,	//
	.public.Stop = &_Stop  //
	};

as5601_t* as5601_ctor(void)  //
{
	_OVFcnt = 0;
	__AS5601.public.Rotation = none;

	__AS5601.public.Write(ABN, 0x07);  // DS p. 21 - 1024 w. 7.8kHz
//	HAL_Delay(5);	// mj8x8.c mj8x8_ctor() - HAL_SuspendTick() disables HAL_Delay()
	__AS5601.public.Write(CONF, 0x200F);	// DS p. 21 - LPM3, 3 LSB hysteresis, Watchdog on
//	HAL_Delay(5);	// mj8x8.c mj8x8_ctor() - HAL_SuspendTick() disables HAL_Delay()

	__AS5601._status = __AS5601.public.Read(STATUS);	// read out device status

	if((__AS5601._status & _BV(MD)) == 0)  // if no magnet detected
		Error_Handler();

	__AS5601._abn = __AS5601.public.Read(ABN);	// read out ABN register
	__AS5601._conf = __AS5601.public.Read(CONF);	// read out conf register

	return &__AS5601.public;  // set pointer to AS5601 public part
}

// motor control PWM signal generation
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt
}

// rotary encoder time base - 10ms
void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt
}

//
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim3)
		++_OVFcnt;  // FIXME - implement overflow

	if(htim == &htim16)
		{
			/* definition of CW and CCW rotation
			 * AS5601 is mounted on e.g. board top, the sensing magnet is above the IC (mounted on the so-called "magnetic cog")
			 * 	- clockwise rotation will increase all counters (angle and timer3's CCR1/2)
			 * 	- counterclockwise rotation will do the opposite
			 */

			_PreviousRawAngle = _CurrentRawAngle;  // save previous angle
			_CurrentRawAngle = __AS5601.public.Read(ANGLE);  // read out current angle

			if(_CurrentRawAngle == _PreviousRawAngle)  // if the old and new angles are the same
				__AS5601.public.Rotation = none;	// no rotation
			else	// rotation
				{
					if(__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3))	// CCW rotation
						__AS5601.public.Rotation = CCW;  // e14 magnetic cog CCW rotation
					else
						__AS5601.public.Rotation = CW;	// e14 magnetic cog CW rotation
				}

			//			__AS5601.counter = __HAL_TIM_GET_COUNTER(&htim3);
		}
}

#endif
