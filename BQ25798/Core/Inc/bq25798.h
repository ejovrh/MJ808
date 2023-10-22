//#include "stm32f0xx_hal_i2c.h"
#include "stm32f0xx_hal.h"

#ifndef INC_BQ25798_H_
#define INC_BQ25798_H_

/*	BQ25798 I2C address is 0x6B
 * 		(DS. p. 49)
 * 		(0x6B << 1) = 0xD6
 */
#define BQ25798_I2C_ADDR  (uint16_t) 0xD6 // DS. p. 50 - left-shifted 0x6B
#define REG_CNT 57	 // count of registers

typedef enum	// register to array index translation
{
	  REG00 = 0,	//  Minimal System Voltage - 1B	RW	DS. p. 55 - 7.5.1.1
	  REG01 = 1,	//	Charge Voltage Limit - 2B	RW	DS. p. 56 - 7.5.1.2
	  REG03 = 2,	//	Charge Current Limit - 2B	RW	DS. p. 57 - 7.5.1.3
	  REG05 = 3,	//	Input Voltage Limit - 1B	RW	DS. p. 58 - 7.5.1.4
	  REG06 = 4,	//	Input Current Limit - 2B	RW	DS. p. 59 - 7.5.1.5
	  REG08 = 5,	//	Precharge Control - 1B	RW	DS. p. 60 - 7.5.1.6
	  REG09 = 6,	//	Termination Control - 1B	RW	DS. p. 61 - 7.5.1.7
	  REG0A = 7,	//	Re-charge Control - 1B	RW	DS. p. 62 - 7.5.1.8
	  REG0B = 8,	//	VOTG regulation - 2B	RW	DS. p. 63 - 7.5.1.9
	  REG0D = 9,	//	IOTG regulation - 1B	RW	DS. p. 64 - 7.5.1.10
	  REG0E = 10,  //	Timer Control - 1B	RW	DS. p. 65 - 7.5.1.11
	  REG0F = 11,  //	Charger Control 0 - 1B	RW	DS. p. 66 - 7.5.1.12
	  REG10 = 12,  //	Charger Control 1 - 1B	RW	DS. p. 67 - 7.5.1.13
	  REG11 = 13,  //	Charger Control 2 - 1B	RW	DS. p. 68 - 7.5.1.14
	  REG12 = 14,  //	Charger Control 3 - 1B	RW	DS. p. 69 - 7.5.1.15
	  REG13 = 15,  //	Charger Control 4 - 1B	RW	DS. p. 70 - 7.5.1.16
	  REG14 = 16,  //	Charger Control 5 - 1B	RW	DS. p. 72 - 7.5.1.17
	  REG15 = 17,  //	MPPT Control - 1B	RW	DS. p. 73 - 7.5.1.18
	  REG16 = 18,  //	Temperature Control - 1B	RW	DS. p. 74 - 7.5.1.19
	  REG17 = 19,  //	NTC Control 0 - 1B	RW	DS. p. 75 - 7.5.1.20
	  REG18 = 20,  //	NTC Control 1 - 1B	RW	DS. p. 76 - 7.5.1.21
	  REG19 = 21,  //	ICO Current Limit - 2B	R	DS. p. 77 - 7.5.1.22
	  REG1B = 22,  //	Charger Status 0 - 1B	R	DS. p. 78 - 7.5.1.23
	  REG1C = 23,  //	Charger Status 1 - 1B	R	DS. p. 80 - 7.5.1.24
	  REG1D = 24,  //	Charger Status 2 - 1B	R	DS. p. 81 - 7.5.1.25
	  REG1E = 25,  //	Charger Status 3 - 1B	R	DS. p. 82 - 7.5.1.26
	  REG1F = 26,  //	Charger Status 4 - 1B	R	DS. p. 83 - 7.5.1.27
	  REG20 = 27,  //	FAULT Status 0 - 1B	R	DS. p. 84 - 7.5.1.28
	  REG21 = 28,  //	FAULT Status 1 - 1B	R	DS. p. 86 - 7.5.1.29
	  REG22 = 29,  //	Charger Flag 0 - 1B	R	DS. p. 87 - 7.5.1.30
	  REG23 = 30,  //	Charger Flag 1 - 1B	R	DS. p. 89 - 7.5.1.31
	  REG24 = 31,  //	Charger Flag 2 - 1B	R	DS. p. 90 - 7.5.1.32
	  REG25 = 32,  //	Charger Flag 3 - 1B	R	DS. p. 91 - 7.5.1.33
	  REG26 = 33,  //	FAULT Flag 0 - 1B	R	DS. p. 92 - 7.5.1.34
	  REG27 = 34,  //	FAULT Flag 1 - 1B	R	DS. p. 94 - 7.5.1.35
	  REG28 = 35,  //	Charger Mask 0 - 1B	RW	DS. p. 95 - 7.5.1.36
	  REG29 = 36,  //	Charger Mask 1 - 1B	RW	DS. p. 97 - 7.5.1.37
	  REG2A = 37,  //	Charger Mask 2 - 1B	RW	DS. p. 98 - 7.5.1.38
	  REG2B = 38,  //	Charger Mask 3 - 1B	RW	DS. p. 99 - 7.5.1.39
	  REG2C = 39,  //	FAULT Mask 0 - 1B	RW	DS. p. 100 - 7.5.1.40
	  REG2D = 40,  //	FAULT Mask 1 - 1B	RW	DS. p. 101 - 7.5.1.41
	  REG2E = 41,  //	ADC Control - 1B	RW	DS. p. 102 - 7.5.1.42
	  REG2F = 42,  //	ADC Function Disable 0 - 1B	RW	DS. p. 103 - 7.5.1.43
	  REG30 = 43,  //	ADC Function Disable 1 - 1B	RW	DS. p. 104 - 7.5.1.44
	  REG31 = 44,  //	IBUS ADC - 2B	R	DS. p. 105 - 7.5.1.45
	  REG33 = 45,  //	IBAT ADC - 2B	R	DS. p. 106 - 7.5.1.46
	  REG35 = 46,  //	VBUS ADC - 2B	R	DS. p. 107 - 7.5.1.47
	  REG37 = 47,  //	VAC1 ADC - 2B	R	DS. p. 108 - 7.5.1.48
	  REG39 = 48,  //	VAC2 ADC - 2B	R	DS. p. 109 - 7.5.1.49
	  REG3B = 49,  //	VBAT ADC - 2B	R	DS. p. 110 - 7.5.1.50
	  REG3D = 50,  //	VSYS ADC - 2B	R	DS. p. 111 - 7.5.1.51
	  REG3F = 51,  //	TS ADC - 2B	R	DS. p. 112 - 7.5.1.52
	  REG41 = 52,  //	TDIE_ADC - 2B	R	DS. p. 113 - 7.5.1.53
	  REG43 = 53,  //	D+ ADC - 2B	R	DS. p. 114 - 7.5.1.54
	  REG45 = 54,  //	D- ADC - 2B	R	DS. p. 115 - 7.5.1.55
	  REG47 = 55,  //	DPDM Driver - 1B	RW	DS. p. 116 - 7.5.1.56
	  REG48 = 56 	//	Part Information - 1B	R	 DS. p. 117 - 7.5.1.57
} RegIndex;

typedef struct	// struct describing devices on BQ25798
{
	uint8_t pg;  // LMR34206 power good, high - good, low - not good
	uint8_t stat;  // charge status, high - charge completed or disabled, low - charge in progress, blink at 1Hz - fault
	uint8_t irq;  // interrupt (reverse logic), high - asserted, low - not asserted

	void (*Write)(const uint8_t addr, const uint16_t val);	// I2C read method
	uint16_t (*Read)(const uint8_t addr);  // I2C write method
	void (*Dump)(void);  // sequential read of all device registers
} bq25798_t;

bq25798_t* bq25798_ctor(I2C_HandleTypeDef *const in_hi2c);

#endif /* INC_BQ25798_H_ */
