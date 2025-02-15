#include "main.h"

#if USE_DAC121C081	// if this particular device is active

#include "dac121c081.h"

#define DAC121C081_I2C_ADDR  (uint16_t) 0x1A // DS. p. 19
#define WRITE 0x00
#define READ 0x01

typedef struct	// dac121c081_t actual
{
	dac121c081_t public;  // public struct
} __dac121c081_t;

static __dac121c081_t __DAC121C081 __attribute__ ((section (".data")));  // preallocate __PAC1952 object in .data

// writes 2 bytes to DAC register
static inline void _Write(const uint16_t *data)
{
	uint8_t buffer[2];

	uint16_t tmp = *data;  // cast to byte pointer
	buffer[1] = (uint8_t) tmp;	// take lower byte
	buffer[0] = (uint8_t) (tmp >> 8);  // take upper byte

	Device->mj8x8->i2c->Transmit(DAC121C081_I2C_ADDR, buffer, 2);
}

// reads 2 bytes from DAC register
static inline uint16_t _Read(void)
{
	uint8_t buffer[2];
	Device->mj8x8->i2c->Receive((DAC121C081_I2C_ADDR | READ), buffer, 2);  // read 2 bytes into buffer

	return ((uint16_t) buffer[1] << 8) & (buffer[0]);  // combine the two bytes into a single 16-bit value
}

// power off & activate 100k pulldown
static inline void _PowerOff(void)
{
	_Write((const uint16_t*) 0x2000);  // set DAC to zero & 100k pulldown
}

static __dac121c081_t __DAC121C081 =  // instantiate sht40_t actual and set function pointers
	{  //
	.public.Read = &_Read,  // set function pointer
	.public.Write = &_Write,  // ditto
	.public.PowerOff = &_PowerOff,  //
	};

dac121c081_t* dac121c081_ctor(void)  //
{
	_PowerOff();  // power off the device

	return &__DAC121C081.public;  // set pointer to DAC121C081 public part
}

#endif
