/*
 * bq25798.h
 *
 *  Created on: May 11, 2021
 *      Author: hrvoje
 */
//#include "stm32f0xx_hal_i2c.h"

#include "stm32f0xx_hal.h"

#ifndef INC_BQ25798_H_
#define INC_BQ25798_H_

#define	BQ25798_I2C_ADDRESS_LSHIFTED	0xD6														// DS. pp. 49; raw device address is 0x6B

#define	REG00_Minimal_System_Voltage	0x0															// DS. p. 55 - 7.5.1.1
#define	REG01_Charge_Voltage_Limit		0x1															// DS. p. 56 - 7.5.1.2
#define	REG03_Charge_Current_Limit		0x3															// DS. p. 57 - 7.5.1.3
#define	REG05_Input_Voltage_Limit		0x5															// DS. p. 58 - 7.5.1.4
#define	REG06_Input_Current_Limit		0x6															// DS. p. 59 - 7.5.1.5
#define	REG08_Precharge_Control			0x7															// DS. p. 60 - 7.5.1.6
#define	REG09_Termination_Control		0x8															// DS. p. 61 - 7.5.1.7
#define 	REG_RST							6
#define	REG0A_Re_charge_Control			0xA															// DS. p. 62 - 7.5.1.8
#define	REG0B_VOTG_regulation			0xB															// DS. p. 63 - 7.5.1.9
#define	REG0D_IOTG_regulation			0xD															// DS. p. 64 - 7.5.1.10
#define PRECHG_TMR						7
#define	REG0E_Timer_Control				0xE															// DS. p. 65 - 7.5.1.11
#define 	EN_TRICHG_TMR					5
#define 	EN_PRECHG_TMR					4
#define 	EN_CHG_TMR						3
#define 	TMR2X_EN						0
#define	REG0F_Charger_Control_0			0xF															// DS. p. 66 - 7.5.1.12
#define		EN_AUTO_IBATDIS					7
#define		FORCE_IBATDIS					6
#define		EN_CHG							5
#define		EN_ICO							4
#define		FORCE_ICO						3
#define		EN_HIZ							2
#define		EN_TERM							1
#define	REG10_Charger_Control_1			0x10														// DS. p. 67 - 7.5.1.13
#define 	WD_RST							3
#define	REG11_Charger_Control_2			0x11														// DS. p. 68 - 7.5.1.14
#define 	FORCE_INDET						7
#define 	AUTO_INDET_EN					6
#define 	EN_12V							5
#define 	EN_9V							4
#define 	HVDCP_EN						3
#define 	SDRV_DLY						0
#define	REG12_Charger_Control_3			0x12														// DS. p. 69 - 7.5.1.15
#define 	DIS_ACDRV						7
#define 	EN_OTG							6
#define 	PFM_OTG_DIS						5
#define 	PFM_FWD_DIS						4
#define 	WKUP_DLY						3
#define 	DIS_LDO							2
#define 	DIS_OTG_OOA						1
#define 	DIS_FWD_OOA						0
#define	REG13_Charger_Control_4			0x13														// DS. p. 70 - 7.5.1.16
#define		EN_ACDRV2						7
#define		EN_ACDRV1						6
#define		PWM_FREQ						5
#define		DIS_STAT						4
#define		DIS_VSYS_SHORT					3
#define		DIS_VOTG_UVP					2
#define		FORCE_VINDPM_DET				1
#define		EN_IBUS_OCP						0
#define	REG14_Charger_Control_5			0x14														// DS. p. 72 - 7.5.1.17
#define		SFET_PRESENT					7
#define		EN_IBAT							5
#define		EN_IINDPM						2
#define		EN_EXTILIM						1
#define		EN_BATOC						0
#define	REG15_MPPT_Control				0x15														// DS. p. 73 - 7.5.1.18
#define		EN_MPPT							0
#define	REG16_Temperature_Control		0x16														// DS. p. 74 - 7.5.1.19
#define		VBUS_PD_EN						3
#define		VAC1_PD_EN						2
#define		VAC2_PD_EN						1
#define	REG17_NTC_Control_0				0x17														// DS. p. 75 - 7.5.1.20
#define	REG18_NTC_Control_1				0x18														// DS. p. 76 - 7.5.1.21
#define		BCOLD							1
#define		TS_IGNORE						0
#define	REG19_ICO_Current_Limit			0x19														// DS. p. 77 - 7.5.1.22
#define	REG1B_Charger_Status_0			0x1B														// DS. p. 78 - 7.5.1.23
#define		IINDPM_STAT						7
#define		VINDPM_STAT						6
#define		WD_STAT							5
#define		POORSRC_STAT					4
#define		PG_STAT							3
#define		AC2_PRESENT_STAT				2
#define		AC1_PRESENT_STAT				1
#define		VBUS_PRESENT_STAT				0
#define	REG1C_Charger_Status_1			0x1C														// DS. p. 80 - 7.5.1.24
#define		BC12_DONE_STAT					0
#define	REG1D_Charger_Status_2			0x1D														// DS. p. 81 - 7.5.1.25
#define		TREG_STAT						2
#define		DPDM_STAT						1
#define		VBAT_PRESENT_STAT				0
#define	REG1E_Charger_Status_3			0x1E														// DS. p. 82 - 7.5.1.26
#define		ACRB2_STAT						7
#define		ACRB1_STAT						6
#define		ADC_DONE_STAT					5
#define		VSYS_STAT						4
#define		CHG_TMR_STAT					3
#define		TRICHG_TMR_STAT					2
#define		PRECHG_TMR_STAT					1
#define	REG1F_Charger_Status_4			0x1F														// DS. p. 83 - 7.5.1.27
#define		VBATOTG_LOW_STAT				4
#define		TS_COLD_STAT					3
#define		TS_COOL_STAT					2
#define		TS_WARM_STAT					1
#define		TS_HOT_STAT						0
#define	REG20_FAULT_Status_0			0x20														// DS. p. 84 - 7.5.1.28
#define		IBAT_REG_STAT					7
#define		VBUS_OVP_STAT					6
#define		VBAT_OVP_STAT					5
#define		IBUS_OCP_STAT					4
#define		IBAT_OCP_STAT					3
#define		CONV_OCP_STAT					2
#define		VAC2_OVP_STAT					1
#define		VAC1_OVP_STAT					0
#define	REG21_FAULT_Status_1			0x21														// DS. p. 86 - 7.5.1.29
#define		VSYS_SHORT_STAT					7
#define		VSYS_OVP_STAT					6
#define		OTG_OVP_STAT					5
#define		OTG_UVP_STAT					4
#define		TSHUT_STAT						2
#define	REG22_Charger_Flag_0			0x22														// DS. p. 87 - 7.5.1.30
#define		IINDPM_FLAG						7
#define		VINDPM_FLAG						6
#define		WD_FLAG							5
#define		POORSRC_FLAG					4
#define		PG_FLAG							3
#define		AC2_PRESENT_FLAG				2
#define		AC1_PRESENT_FLAG				1
#define		VBUS_PRESENT_FLAG				0
#define	REG23_Charger_Flag_1			0x23														// DS. p. 89 - 7.5.1.31
#define		CHG_FLAG						7
#define		ICO_FLAG						6
#define		VBUS_FLAG						4
#define		TREG_FLAG						2
#define		VBAT_PRESENT_FLAG				1
#define		BC12_DONE_FLAG					0
#define	REG24_Charger_Flag_2			0x24														// DS. p. 90 - 7.5.1.32
#define		DPDM_DONE_FLAG					6
#define		ADC_DONE_FLAG					5
#define		VSYS_FLAG						4
#define		CHG_TMR_FLAG					3
#define		TRICHG_TMR_FLAG					2
#define		PRECHG_TMR_FLAG					1
#define		TOPOFF_TMR_FLAG					0
#define	REG25_Charger_Flag_3			0x25														// DS. p. 91 - 7.5.1.33
#define		VBATOTG_LOW_FLAG				4
#define		TS_COLD_FLAG					3
#define		TS_COOL_FLAG					2
#define		TS_WARM_FLAG					1
#define		TS_HOT_FLAG						0
#define	REG26_FAULT_Flag_0				0x26														// DS. p. 92 - 7.5.1.34
#define		IBAT_REG_FLAG					7
#define		VBUS_OVP_FLAG					6
#define		VBAT_OVP_FLAG					5
#define		IBUS_OCP_FLAG					4
#define		IBAT_OCP_FLAG					3
#define		CONV_OCP_FLAG					2
#define		VAC2_OVP_FLAG					1
#define		VAC1_OVP_FLAG					0
#define	REG27_FAULT_Flag_1				0x27														// DS. p. 94 - 7.5.1.35
#define		VSYS_SHORT_FLAG					7
#define		VSYS_OVP_FLAG					6
#define		OTG_OVP_FLAG					5
#define		OTG_UVP_FLAG					4
#define		TSHUT_FLAG						2
#define	REG28_Charger_Mask_0			0x28														// DS. p. 95 - 7.5.1.36
#define		IINDPM_MASK						7
#define		VINDPM_MASK						6
#define		WD_MASK							5
#define		POORSRC_MASK					4
#define		PG_MASK							3
#define		AC2_PRESENT_MASK				2
#define		AC1_PRESENT_MASK				1
#define		VBUS_PRESENT_MASK				0
#define	REG29_Charger_Mask_1			0x29														// DS. p. 97 - 7.5.1.37
#define		CHG_MASK						7
#define		ICO_MASK						6
#define		VBUS_MASK						4
#define		TREG_MASK						2
#define		VBAT_PRESENT_MASK				1
#define		BC12_DONE_MASK					0
#define	REG2A_Charger_Mask_2			0x2A														// DS. p. 98 - 7.5.1.38
#define		DPDM_DONE_MASK					6
#define		ADC_DONE_MASK					5
#define		VSYS_MASK						4
#define		CHG_TMR_MASK					3
#define		TRICHG_TMR_MASK					2
#define		PRECHG_TMR_MASK					1
#define		TOPOFF_TMR_MASK					0
#define	REG2B_Charger_Mask_3			0x2B														// DS. p. 99 - 7.5.1.39
#define		VBATOTG_LOW_MASK				4
#define		TS_COLD_MASK					3
#define		TS_COOL_MASK					2
#define		TS_WARM_MASK					1
#define		TS_HOT_MASK						0
#define	REG2C_FAULT_Mask_0				0x2C														// DS. p. 100 - 7.5.1.40
#define		IBAT_REG_MASK					7
#define		VBUS_OVP_MASK					6
#define		VBAT_OVP_MASK					5
#define		IBUS_OCP_MASK					4
#define		IBAT_OCP_MASK					3
#define		CONV_OCP_MASK					2
#define		VAC2_OVP_MASK					1
#define		VAC1_OVP_MASK					0
#define	REG2D_FAULT_Mask_1				0x2D														// DS. p. 101 - 7.5.1.41
#define		VSYS_SHORT_MASK					7
#define		VSYS_OVP_MASK					6
#define		OTG_OVP_MASK					5
#define		OTG_UVP_MASK					4
#define		TSHUT_MASK						2
#define	REG2E_ADC_Control				0x2E														// DS. p. 102 - 7.5.1.42
#define		ADC_EN							7
#define		ADC_RATE						6
#define		ADC_AVG							3
#define		ADC_AVG_INIT					2
#define	REG2F_ADC_Function_Disable_0	0x2F														// DS. p. 103 - 7.5.1.43
#define		IBUS_ADC_DIS					7
#define		IBAT_ADC_DIS					6
#define		VBUS_ADC_DIS					5
#define		VBAT_ADC_DIS					4
#define		VSYS_ADC_DIS					3
#define		TS_ADC_DIS						2
#define		TDIE_ADC_DIS					1
#define	REG30_ADC_Function_Disable_1	0x30														// DS. p. 104 - 7.5.1.44
#define		DP_ADC_DIS						7
#define		DM_ADC_DIS						6
#define		VAC2_ADC_DIS					5
#define		VAC1_ADC_DIS					4
#define	REG31_IBUS_ADC					0x31														// DS. p. 105 - 7.5.1.45
#define	REG33_IBAT_ADC					0x33														// DS. p. 106 - 7.5.1.46
#define	REG35_VBUS_ADC					0x35														// DS. p. 107 - 7.5.1.47
#define	REG37_VAC1_ADC					0x37														// DS. p. 108 - 7.5.1.48
#define	REG39_VAC2_ADC					0x39														// DS. p. 109 - 7.5.1.49
#define	REG3B_VBAT_ADC					0x3B														// DS. p. 110 - 7.5.1.50
#define	REG3D_VSYS_ADC					0x3D														// DS. p. 111 - 7.5.1.51
#define	REG3F_TS_ADC					0x3F														// DS. p. 112 - 7.5.1.52
#define	REG41_TDIE_ADC					0x41														// DS. p. 113 - 7.5.1.53
#define	REG43_DP_ADC					0x43														// DS. p. 114 - 7.5.1.54
#define	REG45_DN_ADC					0x45														// DS. p. 115 - 7.5.1.55
#define	REG47_DPDM_Driver				0x47														// DS. p. 116 - 7.5.1.56
#define	REG48_Part_Information			0x48														// DS. p. 117 - 7.5.1.57



typedef struct 																						// struct describing devices on BQ25798
{
	void (* Write)(const uint8_t addr, const uint16_t val);											// function pointer for Write method
	uint16_t (* Read)(const uint8_t addr, const uint8_t len);											// function pointer for Read method
} bq25798_t;

bq25798_t * bq25798_ctor(I2C_HandleTypeDef * const in_hi2c);

#endif /* INC_BQ25798_H_ */
