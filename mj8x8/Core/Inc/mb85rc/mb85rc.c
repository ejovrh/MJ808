#include "main.h"

#if USE_FERAM	// if this particular device is active

#include "mb85rc.h"

#define MB85RC_I2C_ADDR  (uint16_t) 0xA0 // DS. p. 6 - MB85RC 8-bit I2C address
#define WRITE 0x00
#define READ 0x01
#define RAM_END	0x7FF // the 2048th word

typedef struct	// mb85rc_t actual
{
	mb85rc_t public;  // public struct
} __mb85rc_t;

static __mb85rc_t __MB85RC __attribute__ ((section (".data")));  // preallocate __MB85RC object in .data

// sets RegAddr high byte into device address - DS. p. 6
static inline uint16_t _DevAddress(const uint16_t Addr)
{
	uint16_t foo = Addr;
	foo = (foo >> 8);  // get high byte
	foo = foo & 0x07;  // get rightmost 3 bits
	foo = (foo << 1);  // shift 1 left to make room for the RW bit
	foo = foo | MB85RC_I2C_ADDR;  // set I2C device address

	return foo;
}

// read up to 4 bytes from FeRAM at address
static inline uint32_t _Read(const uint16_t RegAddr, const uint8_t size)
{
	if(size > 4)
		return 0xBEEF;

	if(size == 0)
		return 0xFEEB;

	uint8_t buffer[4] =
		{0};

	Device->mj8x8->i2c->Read((_DevAddress(RegAddr) | READ), RegAddr, buffer, size);  // read the data

	return ((uint32_t) buffer[0] << 24) | ((uint32_t) buffer[1] << 16) | ((uint32_t) buffer[2] << 8) | ((uint32_t) buffer[3]);
}

// writes one byte of data into the FeRAM module at given address
static void _Write(const uint32_t data, const uint16_t RegAddr, const uint8_t size)
{
	if(size > 4)
		return;

	if(size == 0)
		return;

	uint8_t buffer[5];
	uint32_t tmp = data;

	// Extract each byte in the correct order - e.g. 0x139E8C61
	buffer[4] = (uint8_t) (tmp);  // MSB - 13
	buffer[3] = (uint8_t) (tmp >> 8);
	buffer[2] = (uint8_t) (tmp >> 16);
	buffer[1] = (uint8_t) (tmp >> 24);  // LSB - 61
	buffer[0] = (uint8_t) RegAddr;

	Device->mj8x8->i2c->Transmit(_DevAddress(RegAddr), buffer, size + 1);  // write the data
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
