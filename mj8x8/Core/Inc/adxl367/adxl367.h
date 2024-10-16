#ifndef CORE_INC_ADXL367_ADXL367_H_
#define CORE_INC_ADXL367_ADXL367_H_

#include "main.h"

#if defined(USE_ADXL367)	// if this particular device is active

typedef enum adxl367_reg_t  // register to array index translation
{
	  DEVID_AD = 0,  //  Analog Devices device ID, DS. p. 31
	  DEVID_MST = 1,  // Analog Devices MEMS device ID, DS. p. 31
	  PART_ID = 2,  // device ID, DS. p. 31
	  REV_ID = 3,  // product revision ID, DS. p. 31
	  SERIAL_NUMBER_3 = 4,	// 31-bit product serial number, DS. p. 32
	  SERIAL_NUMBER_2 = 5,	// 31-bit product serial number, DS. p. 32
	  SERIAL_NUMBER_1 = 6,	// 31-bit product serial number, DS. p. 32
	  SERIAL_NUMBER_0 = 7,  // 31-bit product serial number, DS. p. 32
	  XDATA = 8,	// eight most significant bits of the x-axis acceleration data, DS. p. 32
	  YDATA = 9,	// eight most significant bits of the y-axis acceleration data, DS. p. 33
	  ZDATA = 10,  // eight most significant bits of the z-axis acceleration data, DS. p. 33
	  STATUS = 11,  // device status register, DS. p. 33
	  FIFO_ENTRIES_L = 12,  // number of valid data samples present in the FIFO buffer, DS. p. 34
	  FIFO_ENTRIES_H = 13,  // number of valid data samples present in the FIFO buffer, DS. p. 34
	  XDATA_H = 14,  // sign extended (X) x-axis acceleration data, DS. p. 35
	  XDATA_L = 15,  //
	  YDATA_H = 16,  // sign extended (X) y-axis acceleration data, DS. p. 35
	  YDATA_L = 17,  //
	  ZDATA_H = 18,  // sign extended (X) z-axis acceleration data, DS. p. 36
	  ZDATA_L = 19,  //
	  TEMP_H = 20,  // sign extended (T) temperate data, DS. p. 36
	  TEMP_L = 21,  //
	  EX_ADC_H = 22,  //  sign extended (ADC) external connected input ADC data, DS. p. 37
	  EX_ADC_L = 23,  //
	  I2C_FIFO_DATA = 24,  // I2C FIFO Data Read Address, DS. p. 37
	  SOFT_RESET = 25,  // write 0x52 to immediately reset the device, DS. p. 37
	  THRESH_ACT_H = 26,  // 13-bit unsigned threshold activity value,  DS. p. 38
	  THRESH_ACT_L = 27,  //
	  TIME_ACT = 28,  // 16-bit time inactivity value,  DS. p. 38
	  THRESH_INACT_H = 29,  // 13-bit unsigned threshold activity value,  DS. p. 39
	  THRESH_INACT_L = 30,  //
	  TIME_INACT_H = 31,  // 16-bit time inactivity value,  DS. p. 39
	  TIME_INACT_L = 32,  //
	  ACT_INACT_CTL = 33,  // configuration settings for linking or looping detection modes,  DS. p. 40
	  FIFO_CONTROL = 34,  // FIFO control register, DS. p. 41
	  FIFO_SAMPLES = 35,  // number of samples sets to store in the FIFO, DS. p. 42
	  INTMAP1_LOWER = 36,  // interrupt pin 1 event mapping configuration, DS. p. 42
	  INTMAP2_LOWER = 37,  // interrupt pin 1 event mapping configuration, DS. p. 43
	  FILTER_CTL = 38,  // filter control register, DS. p. 44
	  POWER_CTL = 39,  // power control register, DS. p. 44
	  SELF_TEST = 40,  // self test register, DS. p. 45
	  TAP_THRESH = 41,  // tap threshold configuration, DS. p. 45
	  TAP_DUR = 42,  // tap duration configuration, DS. p. 45
	  TAP_LATENT = 43,  // tap latency configuration, DS. p. 46
	  TAP_WINDOW = 44,  // tap window configuration, DS. p. 46
	  X_OFFSET = 45,  // user x-axis offset calibration, DS. p. 46
	  Y_OFFSET = 46,  // user y-axis offset calibration, DS. p. 47
	  Z_OFFSET = 47,  // user z-axis offset calibration, DS. p. 47
	  X_SENS = 48,  // user x-axis gain calibration, DS. p. 47
	  Y_SENS = 49,  // user y-axis gain calibration, DS. p. 47
	  Z_SENS = 50,  // user z-axis gain calibration, DS. p. 48
	  TIMER_CTL = 51,  // timer configuration, DS. p. 48
	  INTMAP1_UPPER = 52,  // interrupt pin 1 event mapping configuration, DS. p. 49
	  INTMAP2_UPPER = 53,  // interrupt pin 2 event mapping configuration, DS. p. 49
	  ADC_CTL = 54,  // ADC configuration, DS. p. 50
	  TEMP_CTL = 55,  // temperature channel/ADC configuration, DS. p. 51
	  TEMP_ADC_OVER_THRSH_H = 56,  // temperature channel/ADC threshold configuration, DS. p. 51
	  TEMP_ADC_OVER_THRSH_L = 57,  //
	  TEMP_ADC_UNDER_THRSH_H = 58,  // temperature channel/ADC threshold configuration, DS. p. 52
	  TEMP_ADC_UNDER_THRSH_L = 59,  //
	  TEMP_ADC_TIMER = 60,  // temperature channel/ADC timer configuration, DS. p. 53
	  AXIS_MASK = 61,  //  axis mask configuration, DS. p. 53
	  STATUS_COPY = 62,  // status copy register, DS. p. 53
	  STATUS_2 = 63  // status2 register, DS. p. 54
} adxl367_reg_t;

typedef struct	// struct describing the Rotary Encoder functionality
{
	void (*ReadFIFO)(void);  //
	void (*Read)(const adxl367_reg_t register, uint8_t *data, const uint8_t len);  //
	void (*Write)(const adxl367_reg_t register, const uint8_t *data, const uint8_t len);  //
} adxl367_t;

adxl367_t* adxl367_ctor(GPIO_TypeDef *_SPI_CS_Port, const uint32_t _SPI_CS_Pin);  // the ADXL367 constructor

#endif

#endif /* CORE_INC_ADXL367_ADXL367_H_ */
