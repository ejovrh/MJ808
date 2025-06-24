#include "main.h"

#if USE_FERAM	// if this particular device is active

#include "fm24cl.h"

#define FM24CL_I2C_ADDR  (uint16_t) 0xA0 // DS. p. 6 - FM24CL 8-bit I2C address
#define WRITE 0x00
#define READ 0x01
#define RAM_END	0x7FF // the 2048th word

typedef struct	// fm24cl_t actual
{
	fm24cl_t public;  // public struct
} __fm24cl_t;

static __fm24cl_t __FM24CL __attribute__ ((section (".data")));  // preallocate __FM24CL object in .data

// read up to 4 bytes from FeRAM at address
static inline uint32_t _Read(const uint16_t RegAddr, const uint8_t size)
{
	if(size > 4)
		return 0xBEEF;

	if(size == 0)
		return 0xFEEB;

	uint8_t buffer[4] =
		{0};

	uint8_t addr_buf[2];

	addr_buf[1] = (uint8_t) (RegAddr & 0xFF);
	addr_buf[0] = (uint8_t) (RegAddr >> 8);

	Device->mj8x8->i2c->Transmit(FM24CL_I2C_ADDR, addr_buf, 2);  // set the internal address register pointer
	Device->mj8x8->i2c->Receive(FM24CL_I2C_ADDR, buffer, size);  // read the data

	return ((uint32_t) buffer[0] << 24) | ((uint32_t) buffer[1] << 16) | ((uint32_t) buffer[2] << 8) | ((uint32_t) buffer[3]);
}

// writes one byte of data into the FeRAM module at given address
static void _Write(const uint32_t data, const uint16_t RegAddr, const uint8_t size)
{
	if(size > 4)
		return;

	if(size == 0)
		return;

	uint8_t buffer[6];
	uint32_t tmp = data;

	// Extract each byte in the correct order - e.g. 0x139E8C61
	buffer[5] = (uint8_t) (tmp);  // MSB - 13
	buffer[4] = (uint8_t) (tmp >> 8);
	buffer[3] = (uint8_t) (tmp >> 16);
	buffer[2] = (uint8_t) (tmp >> 24);  // LSB - 61
	buffer[1] = (uint8_t) (RegAddr & 0xFF);
	buffer[0] = (uint8_t) (RegAddr >> 8);

	Device->mj8x8->i2c->Transmit(FM24CL_I2C_ADDR, buffer, size + 2);  // write the data
}

static __fm24cl_t __FM24CL =  // instantiate fm24cl_t actual and set function pointers
	{  //
	.public.Read = &_Read,  // set function pointer
	.public.Write = &_Write  // set function pointer
	};

fm24cl_t* fm24cl_ctor(void)  //
{
	return &__FM24CL.public;  // set pointer to FM24CL public part
}

#endif
