#include "main.h"

#if USE_TLC59208	// if this particular device is active

#include "tlc59208.h"

#define TLC59208_I2C_ADDR  (uint16_t) 0x40 // DS. p. 16
#define WRITE 0x00
#define READ 0x01

typedef struct	// tlc59208_t actual
{
	uint8_t _PowerState;  // power state
	tlc59208_t public;  // public struct
} __tlc59208_t;

static __tlc59208_t __TLC59208 __attribute__ ((section (".data")));  // preallocate __TLC59208 object in .data

// writes 2 bytes of data to address n and n+1 using auto-increment
static inline void _Write(const uint8_t RegAddr, const uint16_t *data)
{
	uint8_t buffer[2];
	buffer[0] = (RegAddr | 0x80);  // set MSB to 1 for auto increment

	uint16_t tmp = *data;  // cast to byte pointer
	buffer[2] = (uint8_t) tmp;	// take lower byte
	buffer[1] = (uint8_t) (tmp >> 8);  // take upper byte

	Device->mj8x8->i2c->Transmit(TLC59208_I2C_ADDR, buffer, 3);
}

// set LED Driver GPIO to val
static inline void _Power(uint8_t val)
{
	HAL_GPIO_WritePin(LED_Reset_GPIO_Port, LED_Reset_Pin, val);  // drive GPIO to val

	if(val)  // if power on
		{
			if(__TLC59208._PowerState == ON)  // if already on
				return;  // do nothing

			// initialize LED driver
			__TLC59208._PowerState = ON;  // set power state

			uint16_t foo = 0x0008;  //
			__TLC59208.public.Write(0x00, &foo);	// mode 1
		}
	else
		__TLC59208._PowerState = OFF;  // set power state
}

static __tlc59208_t __TLC59208 =  // instantiate sht40_t actual and set function pointers
	{  //
	.public.Write = &_Write,  // set function pointer
	.public.Power = &_Power,	// ditto
	};

tlc59208_t* tlc59208_ctor(void)  //
{
	return &__TLC59208.public;  // set pointer to PAC1952 public part
}

#endif
