#include "main.h"

#if USE_PAC1952	// if this particular device is active

#include "pac1952.h"

#define PAC1952_I2C_ADDR  (uint16_t) 0x20 // DS. p. 34
#define WRITE 0x00
#define READ 0x01

#define USE_REFRESH_G 0	// use general call to refresh all values

#define REFRESH 0x00	// refresh command
#define CTRL 0x01 // configures sample modes and alert pins
#define ACC_COUNT 0x02  // accumulator count for all channels
#define VACCN 0x03 // accumulator count for channel n
#define VBUSN 0x07 // bus voltage for channel n
#define VSENSEN 0x0B // sense voltage for channel n
#define VBUSN_AVG 0x0F // rolling average of the eight most recent bus voltage measurements
#define VSENSEN_AVG 0x13 // rolling average of the eight most recent sense voltage measurements
#define VPOWERN 0x17 // Vsense x Vbus for channel n
#define SMBUS_SETTINGS 0x1C // SMBus settings
#define NEG_PWR_FSR 0x1D // configuration control for bidirectional current
#define REFRESG_G 0x1E // REFRESH response for general call address
#define	REFRESH_V 0x1F // refresh Vbus and Vsense data only
#define	SLOW 0x20 // status and control for slow pin functions
#define CTRL_ACT 0x21 // currently active value of CTRL register
#define NEG_PWR_FSR_ACT 0x22 // currently active value of NEG_PWR_FSR
#define CTRL_LAT 0x23  // latched value of CTRL register
#define NEG_PWR_FSR_LAT 0x24 // latched active value of NEG_PWR_FSR
#define ACCUM_CONFIG 0x25 // enable Vsense and Vbus accumulation
#define ALERT_STATUS 0x26 // what triggered the ALERT pin
#define SLOW_ALERT1 0x27 // assigns specific alert to alert n/slow pin
#define GPIO_ALERT2 0x28 // assigns specific alert to alert n/IO pin
#define ACC_FULLNESS_LIMITS 0x29 // ACC and ACC count fullness limits
#define OC_LIMITN 0x30 // over-current limit for channel n
#define UC_LIMITN 0x34 // under-current limit for channel n
#define	OP_LIMITN 0x38 // overpower limit for channel n
#define	OV_LIMITN 0x3C // over-voltage limit for channel n
#define	UV_LIMITN 0x40 // under-voltage limit for channel n
#define OC_LIMIT_NSAMPLES 0x44 // consecutive over-current samples over threshold for alert
#define UC_LIMIT_NSAMPLES 0x45 // consecutive under-current samples over threshold for alert
#define OP_LIMIT_NSAMPLES 0x46 // consecutive overpower samples over threshold for alert
#define OV_LIMIT_NSAMPLES 0x47 // consecutive over-voltage samples over threshold for alert
#define UV_LIMIT_NSAMPLES 0x48 // consecutive under-voltage samples over threshold for alert
#define ALERT_ENABLE 0x49 // enable alerts
#define ACCUM_CONFIG_ACT 0x4A // currently active value of ACCUM_CONFIG
#define ACCUM_CONFIG_LAT 0x4B // latched value of ACCUM_CONFIG
#define PRODUCT_ID 0xFD // product ID - 0x79 for PAC1952-1
#define MANUFACTURER_ID 0xFE // manufacturer ID - 0x54
#define REVISION_ID 0xFF // revision ID - 0x02

#define REFRESH_TIME 1 // approx. 1ms to refresh all values

typedef struct	// pac1952_t actual
{
	pac1952_t public;  // public struct
} __pac1952_t;

static __pac1952_t __PAC1952 __attribute__ ((section (".data")));  // preallocate __PAC1952 object in .data

#if USE_REFRESH_G
// 6.6.2 - general call
static inline void _Refresh_G(void)
{
	if(Device->mj8x8->i2c->FlagPoweredOn == OFF)  // if I2C is off
		return;

#if USE_DMA
	HAL_I2C_Master_Transmit_DMA(Device->mj8x8->i2c->I2C, 0x00, (uint8_t*) REFRESG_G, 1);
#else
	HAL_I2C_Master_Transmit_IT(Device->mj8x8->i2c->I2C, 0x00, (uint8_t*) REFRESG_G, 1);
#endif
	// wait for the write to complete
	while(HAL_I2C_GetState(Device->mj8x8->i2c->I2C) != HAL_I2C_STATE_READY)
		;
}
#endif

// 6.6.3 - write one byte to register
static inline void _WriteByte(const uint8_t RegAddr, const uint8_t data)
{
	uint8_t buffer[2] =
		{0};

	buffer[0] = RegAddr;
	buffer[1] = data;

	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, buffer, 2);
}

// 6.6.4 - read one byte from register
static inline uint8_t _ReadByte(const uint8_t RegAddr)
{
	uint8_t retval = 0;

	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, (uint8_t*) &RegAddr, 1);
	Device->mj8x8->i2c->Receive((PAC1952_I2C_ADDR | READ), &retval, 1);

	return retval;
}

// 6.6.5 - send byte - set internal address register pointer to location
static inline void _SendByte(const uint8_t byte)
{
	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, (uint8_t*) &byte, 1);
}

// 6.6.6 - receive byte - read byte from register pointer at location
static inline uint8_t _ReceiveByte(void)
{
	uint8_t retval = 0;

	Device->mj8x8->i2c->Receive((PAC1952_I2C_ADDR | READ), &retval, 1);

	return retval;
}

// 6.6.7 - block write
static inline void _BlockWrite(const uint8_t RegAddr, uint8_t *data, const uint8_t len)
{
	if(len > 8)
		return;

	uint8_t buffer[8] =
		{0};

	buffer[0] = RegAddr;
	for(uint8_t i = 0; i < len; ++i)
		buffer[i + 1] = data[i];

	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, buffer, len + 1);
}

// 6.6.8 - block read
static inline void _BlockRead(const uint8_t RegAddr, uint8_t *buffer, const uint8_t len)
{
	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, (uint8_t*) &RegAddr, 1);
	Device->mj8x8->i2c->Receive((PAC1952_I2C_ADDR | READ), buffer, len);
}

// 6.6.1 - refresh & refresh_v
static inline void _Refresh(void)
{
	_SendByte(REFRESH);  // reset the refresh command
	HAL_Delay(REFRESH_TIME);  // wait for the refresh to complete
}

static inline void _RefreshV(void)
{
	_SendByte(REFRESH_V);  // reset the refresh_v command
	HAL_Delay(REFRESH_TIME);  // wait for the refresh to complete
}

static inline void _init(void)
{
	_WriteByte(SMBUS_SETTINGS, 0x00);  // reset the POR bit
	_Refresh();  // refresh the device
}

static inline void _PowerOn(void)
{
	HAL_GPIO_WritePin(PowerMonitorPower_GPIO_Port, PowerMonitorPower_Pin, GPIO_PIN_SET);	// power on the power monitor

	HAL_Delay(50);	// wait for the power monitor to power up

	_init();	// initialize the PAC1952
}

static inline void _PowerOff(void)
{
	HAL_GPIO_WritePin(PowerMonitorPower_GPIO_Port, PowerMonitorPower_Pin, GPIO_PIN_RESET);	// power off the power monitor
}

static __pac1952_t __PAC1952 =  // instantiate sht40_t actual and set function pointers
	{  //
	.public.ReadByte = &_ReadByte,  // set function pointer
	.public.WriteByte = &_WriteByte,  // ditto
	.public.ReceiveByte = &_ReceiveByte,  //
	.public.SendByte = &_SendByte,  //
	.public.BlockWrite = &_BlockWrite,	//
	.public.BlockRead = &_BlockRead,  //
	.public.PowerOn = &_PowerOn,  //
	.public.PowerOff = &_PowerOff,  //
	.public.Refresh = &_Refresh,  //
	.public.RefreshV = &_RefreshV  //
	};

pac1952_t* pac1952_ctor(void)  //
{
	HAL_GPIO_WritePin(PowerMonitorPower_GPIO_Port, PowerMonitorPower_Pin, GPIO_PIN_RESET);	// power monitor off

	return &__PAC1952.public;  // set pointer to PAC1952 public part
}

#endif
