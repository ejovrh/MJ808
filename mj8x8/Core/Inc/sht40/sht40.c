#include "main.h"

#if USE_SHT40	// if this particular device is active

#include "sht40.h"

#define SHT40_I2C_ADDR  (uint16_t) 0x88 // DS. p. 1 (0x44 << 1)
#define WRITE 0x00
#define READ 0x01

typedef struct	// mb85rc_t actual
{
	sht40_t public;  // public struct
} __sht40_t;

static __sht40_t __SHT40 __attribute__ ((section (".data")));  // preallocate __SHT40 object in .data

// reads the sensor & computes temperature and humidity
static void _Measure(void)
{
	uint8_t buffer[6] =
		{0};
	buffer[0] = 0xFD;  // command to measure precise

	Device->mj8x8->i2c->Transmit(SHT40_I2C_ADDR, buffer, 1);

	HAL_Delay(10);  // wait 10ms for the sensor to measure

	Device->mj8x8->i2c->Receive((SHT40_I2C_ADDR | READ), buffer, 6);  // read out measurement into buffer

	__SHT40.public.Temp = (int8_t) ((175 * (uint16_t) (buffer[0] << 8 | buffer[1]) + 32768) / 65535 - 45);  // compute temperature
	__SHT40.public.RH = (int8_t) ((125 * (uint16_t) (buffer[3] << 8 | buffer[4]) + 32768) / 65535 - 6);  // compute humidity
}

static __sht40_t __SHT40 =  // instantiate sht40_t actual and set function pointers
	{  //
	.public.Measure = &_Measure  // set function pointer
	};

sht40_t* sht40_ctor(void)  //
{
	return &__SHT40.public;  // set pointer to SHT40 public part
}

#endif
