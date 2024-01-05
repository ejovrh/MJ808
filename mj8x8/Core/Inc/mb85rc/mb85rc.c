#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "mb85rc.h"
#include "i2c/i2c.h"

#define MB85RC_I2C_ADDR  (uint16_t) 0xA0 // DS. p. 6 - MB85RC 8-bit I2C address

typedef struct	// mb85rc_t actual
{
	mb85rc_t public;  // public struct
} __mb85rc_t;

static __mb85rc_t __MB85RC __attribute__ ((section (".data")));  // preallocate __MB85RC object in .data

static uint8_t _Read(const uint16_t RegAddr)
{
	uint8_t retval;
	I2C->Read(MB85RC_I2C_ADDR, (uint16_t) RegAddr, (uint16_t*) &retval, (uint16_t) 1);
	return retval;
}

static void _Write(const uint16_t RegAddr, uint16_t const *data)
{
	I2C->Write(MB85RC_I2C_ADDR, (uint16_t) RegAddr, data, (uint16_t) 1);
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
