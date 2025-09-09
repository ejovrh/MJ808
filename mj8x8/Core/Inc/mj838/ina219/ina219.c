#include "main.h"

#if USE_INA219	// if this particular device is active

#include "ina219.h"

#define INA219_I2C_ADDR  (uint16_t) 0x80 // DS. p. 14
#define WRITE 0x00
#define READ 0x01

#define CONFIG_REG 0x00	// POR default 0x399F
#define SHUNT_VOLTAGE_REG 0x01
#define BUS_VOLTAGE_REG 0x02
#define POWER_REG 0x03
#define CURRENT_REG 0x04
#define CALIBRATION_REG 0x05	// POR default 0x0000

#define CALCULATE_SHUNT_VOLTAGE 1

typedef struct	// ina219_t actual
{
	ina219_t public;  // public struct
} __ina219_t;

// private variables for voltage, current and power
#if CALCULATE_SHUNT_VOLTAGE
int16_t _ShuntVoltage;
#endif
uint32_t _Voltage;  // private variable for voltage
int32_t _Current;  // private variable for current
uint32_t _Power;	// private variable for power

// DS. p. 16, figure 15 - write word command
static inline void _WriteWord(const uint8_t RegAddr, uint8_t *data)
{
	uint8_t buffer[3] =
		{0};

	buffer[0] = RegAddr;
	buffer[1] = data[0];
	buffer[2] = data[1];

	Device->mj8x8->i2c->Transmit(INA219_I2C_ADDR, buffer, 3);
}

// DS. p. 16, figure 16 - read word command
static inline void _ReadWord(uint8_t *buffer)
{
	Device->mj8x8->i2c->Receive((INA219_I2C_ADDR | READ), buffer, 2);
}

// DS. p. 17, figure 18 - set internal address register pointer to location
static inline void _RegisterPointerSet(const uint8_t byte)
{
	Device->mj8x8->i2c->Transmit(INA219_I2C_ADDR, (uint8_t*) &byte, 1);
}

static inline void _init(void)
{
	uint8_t temp[2];  // temporary variable for device readout

	/* DS. p. 19 - 8.6.2 - programming the configuration register
	 * 	BRNG = 0 (16V)
	 * 	PG = 01 , gain /2, (80mV)
	 * 	BADC = 0011 - 12 bit
	 * 	SADC = 0011 - 12 bit
	 * 	MODE = 111 (shunt and bus, continuous)
	 */
	temp[0] = 0x09;
	temp[1] = 0x9F;
	_WriteWord(CONFIG_REG, temp);  // write value to configuration register

	/* DS. p. 12 - 8.5.1 - programming the calibration register
	 * current_LSB = maximum_expected_current / 32768
	 * 						 = 2A / 32768 = 0.00006103515625 A
	 *
	 * cal = 0.04096 / (current_LSB * Rshunt)
	 * 		 = 0.04096 / (0.00006103515625 * 0.1)
	 * 		 = 6,710.8864 = 0x1A36 (rounded)
	 * 		 0x1A36 << 1 = 0x346C (to get power register in mW)
	 */
	temp[0] = 0x19;  // cal is adjusted to match actual current measurement via multimeter
	temp[1] = 0x68;
	_WriteWord(CALIBRATION_REG, temp);  // write value to calibration register
}

// convert 2's complement to int16_t
static inline int16_t _2sComplementTo_uint16(const uint8_t *buffer)
{
	int16_t retval = 0;

	if(buffer[0] & 0x80)  // if sign bit is set
		{
			retval = (int16_t) ((~buffer[0] & 0xFF) << 8) | (~buffer[1] & 0xFF);  // invert bits
			retval = (retval + 1) & 0xFFFF;  // add 1
			retval = -retval;  // make negative
		}
	else  // if sign bit is clear
		{
			retval = ((buffer[0] << 8) | (buffer[1]));  // combine bytes
		}

	return retval;
}

static inline void _Measure(void)
{
	uint8_t temp[2];  // temporary variable for device readout

#if CALCULATE_SHUNT_VOLTAGE
	_RegisterPointerSet(SHUNT_VOLTAGE_REG);  // set pointer to shunt voltage register
	_ReadWord(temp);  // read 2 bytes from shunt voltage register
	_ShuntVoltage = _2sComplementTo_uint16(temp);
#endif

	_RegisterPointerSet(BUS_VOLTAGE_REG);  // set pointer to voltage register
	_ReadWord(temp);  // read 2 bytes from voltage register
	_Voltage = (uint32_t) ((((temp[0] << 8) | (temp[1])) >> 3) * 4);  // combine the two bytes into a single 16-bit value, shift away 3 LSBs

	_RegisterPointerSet(CURRENT_REG);  // set pointer to current register
	_ReadWord(temp);  // read 2 bytes from current register
	_Current = (int32_t) ((_2sComplementTo_uint16(temp) * 61) / 1000);  // combine, scale, and convert to mA in one statement

	_RegisterPointerSet(POWER_REG);  // set pointer to power register
	_ReadWord(temp);  // read 2 bytes from power register
	// power LSB is 20 times the current LSB
	_Power = (uint32_t) ((((temp[0] << 8) | (temp[1])) * 1250) >> 10);  // 1250/1024 ≈ 1.220703125f
}

static __ina219_t __INA219 __attribute__ ((section (".data"))) =  // instantiate ina219_t actual and set function pointers
	{  //
	.public.Measure = &_Measure,  // set function pointer
	.public.Voltage = &_Voltage,  // ditto
	.public.Current = &_Current,  // ditto
	.public.Power = &_Power,  // ditto
	};

ina219_t* ina219_ctor(void)  //
{
	_init();  // initialize the device
	return &__INA219.public;  // set pointer to INA219 public part
}

#endif
