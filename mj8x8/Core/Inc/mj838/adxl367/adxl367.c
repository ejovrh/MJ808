#include "main.h"

#if USE_ADXL367

#include "adxl367.h"

#define ADXL367_I2C_ADDR  (uint16_t) 0x53 // DS. p. 29
#define WRITE 0x00
#define READ 0x01

typedef struct	// adxl367_t actual
{
	adxl367_t public;  // public struct
} __adxl367_t;

static __adxl367_t __ADXL367 __attribute__ ((section (".data")));  // preallocate __ADXL367 object in .data

// writes up to 2 bytes to accelerometer register
static inline void _Write(const uint8_t reg_addr, const uint8_t size, uint16_t value)
{
	uint8_t buffer[3] =
		{0};

	buffer[0] = reg_addr;

	if(size == 1)
		buffer[1] = (uint8_t)(value & 0xFF);
	else if(size == 2)
		{
			buffer[1] = (uint8_t)(value & 0xFF);        // LSB
			buffer[2] = (uint8_t)((value >> 8) & 0xFF);  // MSB
		}

	Device->mj8x8->i2c->Transmit(ADXL367_I2C_ADDR, buffer, size + 1);
}

// reads up to 2 bytes from accelerometer register and returns the value as uint16_t
static inline uint16_t _Read(const uint8_t reg_addr, const uint8_t size)
{
	uint8_t buffer[2] =
		{0};

	Device->mj8x8->i2c->Read(ADXL367_I2C_ADDR, reg_addr, buffer, size);

	if(size == 2)
		return ((uint16_t) buffer[0] << 8) | buffer[1];
	else
		return buffer[0];
}

static __adxl367_t __ADXL367 =  // instantiate adxl367_t actual and set function pointers
	{  //
	.public.Read = &_Read,  // set function pointer
	.public.Write = &_Write,  // ditto
	};

static inline void _init(void)
{
	// 1. Software reset: 0x52 triggers internal reset
	__ADXL367.public.Write(SOFT_RESET, 0x52);  // SOFT_RESET (0x1F, 1)

	// 2. Ensure standby: 0x00 disables measurement mode
	__ADXL367.public.Write(POWER_CTL, 0x00);  // POWER_CTL (0x2D, 1), bit0=0 (standby)

	// 3. Configure FILTER_CTL for 4g range, 12.5 Hz ODR
	// FILTER_CTL (0x2C, 1):
	// [ODR2:ODR0]=000 (12.5Hz), [Range1:Range0]=10 (4g), [ExtSample]=0, [HalfBW]=0
	// 0b00010000 = 0x10
	__ADXL367.public.Write(FILTER_CTL, 0x10);

	// 4. Configure ACT_INACT_CTL for referenced detection, all axes
	// ACT_INACT_CTL (0x27, 1):
	// bit7=0 (reserved), bit6=1 (activity referenced), bit5=1 (activity Z), bit4=1 (activity Y), bit3=1 (activity X),
	// bit2=1 (inactivity referenced), bit1=1 (inactivity Z), bit0=1 (inactivity Y)
	// 0b01111011 = 0x7B
	__ADXL367.public.Write(ACT_INACT_CTL, 0x7B);

	// 5. Set thresholds and times (example values)
	// THRESH_ACT (0x20, 2): 0x0020 = 32 LSB (8mg for 4g range)
	__ADXL367.public.Write(THRESH_ACT, 0x0020);
	// TIME_ACT (0x22, 1): 0x01 = 1 sample
	__ADXL367.public.Write(TIME_ACT, 0x01);
	// THRESH_INACT (0x23, 2): 0x0020 = 32 LSB
	__ADXL367.public.Write(THRESH_INACT, 0x0020);
	// TIME_INACT (0x25, 2): 0x0001 = 1 sample
	__ADXL367.public.Write(TIME_INACT, 0x0001);

	// 6. Disable tap detection: AXIS_MASK (0x43, 1) = 0x00 disables tap on all axes
	__ADXL367.public.Write(AXIS_MASK, 0x00);
	// 7. Disable ADC: ADC_CTL (0x3C, 1) = 0x00 disables ADC
	__ADXL367.public.Write(ADC_CTL, 0x00);

	// 8. Map activity/inactivity interrupts to INT1 only
	// INT_MAP1_L (0x2A, 1): bit0=1 (activity to INT1), bit1=1 (inactivity to INT1)
	__ADXL367.public.Write(INT_MAP1_L, 0x03);
	// INT_MAP2_L (0x2B, 1): all bits 0 (no INT2 mapping)
	__ADXL367.public.Write(INT_MAP2_L, 0x00);

	// 9. Enable measurement: POWER_CTL (0x2D, 1), bit0=1 (measurement mode)
	__ADXL367.public.Write(POWER_CTL, 0x01);
}

adxl367_t* adxl367_ctor(void)
{
	_init();
	return &__ADXL367.public;  // set pointer to ADXL367 public part
}

#endif
