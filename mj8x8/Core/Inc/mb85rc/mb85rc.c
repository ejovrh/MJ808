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
static inline uint32_t _Read(const uint8_t RegAddr, const uint8_t size)
{
	return Device->mj8x8->i2c->Read(MB85RC_I2C_ADDR, RegAddr, size);
}

// writes one byte of data into the FeRAM module at given address
static void _Write(const uint32_t data, const uint8_t RegAddr, const uint8_t size)
{
	Device->mj8x8->i2c->Write(MB85RC_I2C_ADDR, RegAddr, data, size);
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
