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
#define VACC1 0x03 // accumulator count for channel 1
#define VACC2 0x04 // accumulator count for channel 2
#define VACC3 0x05 // accumulator count for channel 3
#define VACC4 0x06 // accumulator count for channel 4
#define VBUS1 0x07 // bus voltage for channel 1
#define VBUS2 0x08 // bus voltage for channel 2
#define VBUS3 0x09 // bus voltage for channel 3
#define VBUS4 0x0A // bus voltage for channel 4
#define VSENSE1 0x0B // sense voltage for channel 1
#define VSENSE2 0x0C // sense voltage for channel 2
#define VSENSE3 0x0D // sense voltage for channel 3
#define VSENSE4 0x0E // sense voltage for channel 4
#define VBUS1_AVG 0x0F // rolling average of the eight most recent bus voltage measurements
#define VBUS2_AVG 0x10 // rolling average of the eight most recent bus voltage measurements
#define VBUS3_AVG 0x11 // rolling average of the eight most recent bus voltage measurements
#define VBUS4_AVG 0x12 // rolling average of the eight most recent bus voltage measurements
#define VSENSE1_AVG 0x13 // rolling average of the eight most recent sense voltage measurements
#define VSENSE2_AVG 0x14 // rolling average of the eight most recent sense voltage measurements
#define VSENSE3_AVG 0x15 // rolling average of the eight most recent sense voltage measurements
#define VSENSE4_AVG 0x16 // rolling average of the eight most recent sense voltage measurements
#define VPOWER1 0x17 // Vsense x Vbus for channel 1
#define VPOWER2 0x18 // Vsense x Vbus for channel 2
#define VPOWER3 0x19 // Vsense x Vbus for channel 3
#define VPOWER4 0x1A // Vsense x Vbus for channel 4
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
#define OC_LIMIT1 0x30 // over-current limit for channel 1
#define OC_LIMIT2 0x31 // over-current limit for channel 2
#define OC_LIMIT3 0x32 // over-current limit for channel 3
#define OC_LIMIT4 0x33 // over-current limit for channel 4
#define UC_LIMIT1 0x34 // under-current limit for channel 1
#define UC_LIMIT2 0x35 // under-current limit for channel 2
#define UC_LIMIT3 0x36 // under-current limit for channel 3
#define UC_LIMIT4 0x37 // under-current limit for channel 4
#define	OP_LIMIT1 0x38 // overpower limit for channel 1
#define	OP_LIMIT2 0x39 // overpower limit for channel 2
#define	OP_LIMIT3 0x3A // overpower limit for channel 3
#define	OP_LIMIT4 0x3B // overpower limit for channel 4
#define	OV_LIMIT1 0x3C // over-voltage limit for channel 1
#define	OV_LIMIT2 0x3D // over-voltage limit for channel 2
#define	OV_LIMIT3 0x3E // over-voltage limit for channel 3
#define	OV_LIMIT4 0x3F // over-voltage limit for channel 4
#define	UV_LIMIT1 0x40 // under-voltage limit for channel 1
#define	UV_LIMIT2 0x41 // under-voltage limit for channel 2
#define	UV_LIMIT3 0x42 // under-voltage limit for channel 3
#define	UV_LIMIT4 0x43 // under-voltage limit for channel 4
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

static uint8_t _Vbus[8];
static uint8_t _Vsense[8];
static uint8_t _Vpower[16];

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
//static inline uint8_t _ReadByte(const uint8_t RegAddr)
//{
//	uint8_t retval = 0;
//
//	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, (uint8_t*) &RegAddr, 1);
//	Device->mj8x8->i2c->Receive((PAC1952_I2C_ADDR | READ), &retval, 1);
//
//	return retval;
//}

// 6.6.5 - send byte - set internal address register pointer to location
static inline void _SendByte(const uint8_t byte)
{
	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, (uint8_t*) &byte, 1);
}

// 6.6.6 - receive byte - read byte from register pointer at location
//static inline uint8_t _ReceiveByte(void)
//{
//	uint8_t retval = 0;
//
//	Device->mj8x8->i2c->Receive((PAC1952_I2C_ADDR | READ), &retval, 1);
//
//	return retval;
//}

// 6.6.7 - block write
//static inline void _BlockWrite(const uint8_t RegAddr, uint8_t *data, const uint8_t len)
//{
//	if(len > 8)
//		return;
//
//	uint8_t buffer[8] =
//		{0};
//
//	buffer[0] = RegAddr;
//	for(uint8_t i = 0; i < len; ++i)
//		buffer[i + 1] = data[i];
//
//	Device->mj8x8->i2c->Transmit(PAC1952_I2C_ADDR, buffer, len + 1);
//}

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

static inline void _Measure(void)
{
	_RefreshV();

	_BlockRead(0x07, _Vbus, 8);  // read Vbus
	_BlockRead(0x0B, _Vsense, 8);  // read Vsense
	_BlockRead(0x17, _Vpower, 16);  // read Vpower
}

//
static inline void _Power(const uint8_t state)
{
	if(state == ON)
		{
			HAL_GPIO_WritePin(PowerMonitorPower_GPIO_Port, PowerMonitorPower_Pin, GPIO_PIN_SET);	// power on the power monitor

			HAL_Delay(50);	// wait for the power monitor to power up

			_init();	// initialize the PAC1952
		}
	else
		HAL_GPIO_WritePin(PowerMonitorPower_GPIO_Port, PowerMonitorPower_Pin, GPIO_PIN_RESET);	// power off the power monitor
}

//
static inline float _GetVbus(const uint8_t channel)
{
	return 0;
}

//
static inline float _GetVsense(const uint8_t channel)
{
	return 0;
}

//
static inline float _GetVpower(const uint8_t channel)
{
	return 0;
}

static __pac1952_t __PAC1952 =  // instantiate sht40_t actual and set function pointers
	{  //
	.public.Power = &_Power,  // set function pointer
	.public.Measure = &_Measure,  // ditto
	.public.GetVbus = &_GetVbus,  // ditto
	.public.GetVsense = &_GetVsense,  // ditto
	.public.GetVpower = &_GetVpower,  // ditto
	};

pac1952_t* pac1952_ctor(void)  //
{
	HAL_GPIO_WritePin(PowerMonitorPower_GPIO_Port, PowerMonitorPower_Pin, GPIO_PIN_RESET);	// power monitor off

	return &__PAC1952.public;  // set pointer to PAC1952 public part
}

#endif
