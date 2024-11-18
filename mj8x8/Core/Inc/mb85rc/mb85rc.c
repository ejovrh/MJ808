#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "mb85rc.h"

#define MB85RC_I2C_ADDR  (uint16_t) 0xA0 // DS. p. 6 - MB85RC 8-bit I2C address
#define WRITE 0x00
#define READ 0x01

typedef struct	// mb85rc_t actual
{
	mb85rc_t public;  // public struct
} __mb85rc_t;

static __mb85rc_t __MB85RC __attribute__ ((section (".data")));  // preallocate __MB85RC object in .data

// reads one byte of data out of the FeRAM module from given address
static uint8_t _Read(const uint8_t RegAddr)
{
	/* abstract
	 * let's keep it really simple:
	 * 	- i don't need the whole 11-bit address space; 8 bit is enough
	 * 		meaning: uint8_t RegAddr is enough
	 * 	- i will always read out one single byte at a time
	 */

	uint8_t retval;  // container for read out value
	uint8_t ptr = RegAddr;	// container for register address to read from

	// check if the device is ready
	while(HAL_I2C_IsDeviceReady(Device->mj8x8->i2c->I2C, MB85RC_I2C_ADDR, 300, 300) != HAL_OK)
		;

	// send the address we want to read from
	if(HAL_I2C_Master_Transmit_DMA(Device->mj8x8->i2c->I2C, MB85RC_I2C_ADDR, &ptr, 1) != HAL_OK)
		Error_Handler();

	// give the bus time to settle
	while(HAL_I2C_GetState(Device->mj8x8->i2c->I2C) != HAL_I2C_STATE_READY)
		;

	// tell the I2C device that we want to read from it
	if(HAL_I2C_Master_Receive_DMA(Device->mj8x8->i2c->I2C, (MB85RC_I2C_ADDR | READ), &retval, 1) != HAL_OK)
		Error_Handler();

	// give the bus time to settle
	while(HAL_I2C_GetState(Device->mj8x8->i2c->I2C) != HAL_I2C_STATE_READY)
		;

	return retval;
}

// writes one byte of data into the FeRAM module at given address
static void _Write(const uint8_t RegAddr, uint8_t const *data)
{
	/* abstract
	 * let's keep it really simple:
	 * 	- i will only write one single byte at a time into the module
	 * 		meaning: buffer[] with the size of 2 is enough
	 * 	- i don't need the whole 11-bit address space; 8 bit is enough
	 * 		meaning: uint8_t RegAddr is enough
	 */

	uint8_t buffer[2];	// buffer for transfer

	buffer[0] = RegAddr;	// 1st position is the register address we want to write to
	buffer[1] = *data;  // 2nd position is the data we want to write

	// check if the device is ready
	while(HAL_I2C_IsDeviceReady(Device->mj8x8->i2c->I2C, MB85RC_I2C_ADDR, 300, 300) != HAL_OK)
		;

	// transfer the buffer contents & check for return status
	if(HAL_I2C_Master_Transmit_DMA(Device->mj8x8->i2c->I2C, MB85RC_I2C_ADDR, buffer, 2) != HAL_OK)
		Error_Handler();

	// give the bus time to settle
	while(HAL_I2C_GetState(Device->mj8x8->i2c->I2C) != HAL_I2C_STATE_READY)
		;
}

static __mb85rc_t __MB85RC =  // instantiate mb85rc_t actual and set function pointers
	{  //
	.public.Read = &_Read,  // set function pointer
	.public.Write = &_Write  // set function pointer
	};

mb85rc_t* mb85rc_ctor(void)  //
{
	return &__MB85RC.public;  // set pointer to MB85RC public part
}

#endif
