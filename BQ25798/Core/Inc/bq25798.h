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

#define	REG00_Minimal_System_Voltage 0x0	// 1B	RW	DS. p. 55 - 7.5.1.1
#define	REG01_Charge_Voltage_Limit 0x1	// 2B	RW	DS. p. 56 - 7.5.1.2
#define	REG03_Charge_Current_Limit 0x3	// 2B	RW	DS. p. 57 - 7.5.1.3
#define	REG05_Input_Voltage_Limit 0x5	// 1B	RW	DS. p. 58 - 7.5.1.4
#define	REG06_Input_Current_Limit 0x6	// 2B	RW	DS. p. 59 - 7.5.1.5
#define	REG08_Precharge_Control 0x7	// 1B	RW	DS. p. 60 - 7.5.1.6
#define	REG09_Termination_Control 0x8	// 1B	RW	DS. p. 61 - 7.5.1.7
#define 	REG_RST 6
#define	REG0A_Re_charge_Control 0xA	// 1B	RW	DS. p. 62 - 7.5.1.8
#define	REG0B_VOTG_regulation 0xB	// 2B	RW	DS. p. 63 - 7.5.1.9
#define	REG0D_IOTG_regulation 0xD	// 1B	RW	DS. p. 64 - 7.5.1.10
#define 	PRECHG_TMR 7
#define	REG0E_Timer_Control 0xE	// 1B	RW	DS. p. 65 - 7.5.1.11
#define 	EN_TRICHG_TMR 5
#define 	EN_PRECHG_TMR 4
#define 	EN_CHG_TMR 3
#define 	TMR2X_EN 0

#define	REG0F_Charger_Control_0 0xF	// 1B	RW	DS. p. 66 - 7.5.1.12
#define		EN_AUTO_IBATDIS 7	// enable auto battery discharge during OVP fault: 0 - don't discharge, 1 - discharge on BAT during OVP fault
#define		FORCE_IBATDIS 6	// force battery discharging current: 0 - idle (default), 1 - force discharge regardless of OVP status
#define		EN_CHG 5	// charger enable: 0 - disable (default), 1 - enable
#define		EN_ICO 4	// input current optimizer: 0 - disable (default), 1 - enable
#define		FORCE_ICO 3	// force start input current optimizer: 0 - not force, 1 - force
#define		EN_HIZ 2	// enable HIZ mode: 0 - disable (default), 1 - enable
#define		EN_TERM 1	// enable termination: 0 - disable (default), 1 - enable
#define 	EN_BACKUP 0	// enable backup mode where OTG engages when VBUS droops below VBUS_BACKUP_MASK: 0 - disable (default), 1 - enable

#define	REG10_Charger_Control_1 0x10 // 1B	RW	DS. p. 67 - 7.5.1.13
#define		VBUS_BACKUP_MASK 0xC0	// threshold to trigger backup mode
#define			VBUS_BACKUP_40 0x00	// 40% * VINDPM
#define			VBUS_BACKUP_60 0x40	// 60% * VINDPM
#define			VBUS_BACKUP_80 0x80	// 80% * VINDPM (default)
#define			VBUS_BACKUP_100 0xC0	// 100% * VINDPM
#define		VAC_OVP_MASK	0x30	// VAC OVP thresholds
#define			VAC_OVP_26V 0x00	// 26V
#define			VAC_OVP_22V 0x10	// 22V
#define			VAC_OVP_12V 0x20	// 12V
#define			VAC_OVP_7V 0x30	// 7V (default)
#define 	WD_RST 3	// I2C watchdog timer reset: 0 - normal (default), 1 - reset
#define		WATCHDOG_MASK 0x07	// watchdog timer settings
#define			WATCHDOG_TIMER_DISABLE 0x00	// timer set to disabled
#define			WATCHDOG_TIMER_05 0x01	// timer set to 0.5s
#define			WATCHDOG_TIMER_1 0x02	// timer set to 1s
#define			WATCHDOG_TIMER_2 0x03	// timer set to 2s
#define			WATCHDOG_TIMER_20 0x04	// timer set to 20s
#define			WATCHDOG_TIMER_40 0x05	// timer set to 40s  (default)
#define			WATCHDOG_TIMER_80 0x06	// timer set to 80s
#define			WATCHDOG_TIMER_160 0x07	// timer set to 160s

#define	REG11_Charger_Control_2 0x11 // 1B	RW	DS. p. 68 - 7.5.1.14
#define 	FORCE_INDET 7	// force D+/D- detection
#define 	AUTO_INDET_EN 6	// automatic D+/D- detection enable: 0 disabled, 1 - enabled (default)
#define 	EN_12V 5	// 12V HVDC: 0 - disable (default), 1 - enable
#define 	EN_9V 4	// 9V HVDC: 0 - disable (default), 1 - enable
#define 	HVDCP_EN 3 // high voltage DCP handshake enable: 0 - disable (default), 1 - enable
#define		SDRV_CTRL_MASK	0x06	// Ship FET control, forces the device into different modes
#define			SDRV_CTRL_IDLE	0x06	// Ship FET idle (default)
#define			SDRV_CTRL_SHUTDOWN 0x02 // shutdown mode
#define			SDRV_CTRL_SHIP 0x04 // ship mode
#define			SDRV_CTRL_POR 0x06	// system power reset
#define 	SDRV_DLY 0	// Ship FET control action delay: 1 - do not add delay, 1 - add 10 delay (default)

#define	REG12_Charger_Control_3 0x12 // 1B	RW	DS. p. 69 - 7.5.1.15
#define 	DIS_ACDRV 7	// force both EN_ACDRV1=0 and EN_ACDRV2=0 (default 0)
#define 	EN_OTG 6	// OTG mode control: 0 - disable (default), 1 - enable
#define 	PFM_OTG_DIS 5	// disable PFM in OTG mode: 0 - enable (default), 1 - disable
#define 	PFM_FWD_DIS 4	// disable PFM in forward mode: 0 enable  (default), 1 - disable
#define 	WKUP_DLY 3 // wake up delay from ship mode: 0 - 1s (default), 1 - 15s
#define 	DIS_LDO 2	// disable Bat FET LDO in pre-charge state: 0 - enable (default), 1 - disable
#define 	DIS_OTG_OOA 1	// disable OOA in OTG mode: 0 - enable (default), 1 - disable
#define 	DIS_FWD_OOA 0	// disable OOA in forward mode: 0 - enable (default), 1 - disable

#define	REG13_Charger_Control_4 0x13 // 1B	RW	DS. p. 70 - 7.5.1.16
#define		EN_ACDRV2 7	// ACFET2-RBFET2 gate driver control: 0 - turn off (default), 1 - turn on
#define		EN_ACDRV1 6	// ACFET1-RBFET1 gate driver control: 0 - turn off (default), 1 - turn on
#define		PWM_FREQ 5	// switching frequency: 0 - 1.5 MHz, 1 - 0.75 MHz
#define		DIS_STAT 4	// disable stat output: 0 - enable (default) - 1 disable
#define		DIS_VSYS_SHORT 3	// disable forward mode VSYS short hiccup detection: 0 - enable (default), 1 - disable
#define		DIS_VOTG_UVP 2	// disable OTG mode VOTG UVP hiccup protection: 0 - enable (default), 1 - disable
#define		FORCE_VINDPM_DET 1 // force VINDPM detection: 0 - don't force (default), 1 - do force
#define		EN_IBUS_OCP 0	// enable IBUS_OCP in forward mode: 0 - disable, 1 - enable (default)

#define	REG14_Charger_Control_5 0x14 // 1B	RW	DS. p. 72 - 7.5.1.17
#define		SFET_PRESENT 7	// Ship FET present: 0 - not populated, 1 - FET present
#define		EN_IBAT 5	// IBAT discharge current sensing enable: 0 - disable sensing (defauklt), 1 - enable sensing
#define		IBAT_REG_MASK 0x18	// battery discharging current regulation in OTG mode
#define			IBAT_REG_3A	0x00	//
#define			IBAT_REG_4A	0x08	//
#define			IBAT_REG_5A	0x10	//
#define			IBAT_REG_DISABLE 0x18 // (default)
#define		EN_IINDPM 2	//	Enable the internal IINDPM register input current regulation: 0 - disable, 1 - enable (default)
#define		EN_EXTILIM 1	// Enable the external ILIM_HIZ pin input current regulation: 0 - disable, 1 - enable (default)
#define		EN_BATOC 0	//	Enable the battery discharging current OCP: 0 - disable, 1 - enable (default)

#define	REG15_MPPT_Control 0x15	// 1B	RW	DS. p. 73 - 7.5.1.18
#define		VOC_PCT_MASK 0xE0	// VINDPM percentage of VBUS open
#define			VOC_PCT_05625 0x00	// 0.5625
#define			VOC_PCT_0625 0x20	// 0.625
#define			VOC_PCT_06875	0x40	// 0.6875
#define			VOC_PCT_075	0x60	// 0.75
#define			VOC_PCT_08125 0x80	// 0.8125
#define			VOC_PCT_0875 0xA0	//	0.875 (default)
#define			VOC_PCT_09375	0xC0	//	0.9375
#define			VOC_PCT_1	0xE0	//	1
#define		VOC_DLY_MASK 0x18	// time delay for VOC measurement
#define			VOC_DLY_50MS	0x00	//	50ms
#define			VOC_DLY_300MS	0x08	// 300ms (default)
#define			VOC_DLY_2S	0x10	// 1s
#define			VOC_DLY_5S	0x18	// 5s
#define		VOC_RATE_MASK	0x06	// time interval between two VBUS open circuit voltage measurement
#define			VOC_RATE_30S 0x00	// 30 s
#define			VOC_RATE_2MIN 0x02	//	2 min
#define			VOC_RATE_10MIN 0x04	//	10 min
#define			VOC_RATE_30MIN 0x06	//	30 min
#define	EN_MPPT 0	// enable the MPPT to measure the VBUS open circuit voltage

#define	REG16_Temperature_Control 0x16 // 1B	RW	DS. p. 74 - 7.5.1.19
#define		VBUS_PD_EN 3
#define		VAC1_PD_EN 2
#define		VAC2_PD_EN 1
#define	REG17_NTC_Control_0 0x17 // 1B	RW	DS. p. 75 - 7.5.1.20

#define	REG18_NTC_Control_1 0x18 // 1B	RW	DS. p. 76 - 7.5.1.21
#define		TS_COOL_MASK 0xc0	// JEITA VT2 comparator voltage rising thresholds as a percentage of REGN. The corresponding temperature in the brackets is achieved when a 103AT NTC thermistor is used, RT1=5.24kΩ and RT2=30.31kΩ.
#define			TS_COOL_5DEG 0x00	// 71.1% (5°C)
#define			TS_COOL_10DEG 0x40	// 68.4% (default) (10°C)
#define			TS_COOL_15DEG 0x80	// 65.5% (15°C)
#define			TS_COOL_20DEG 0xc0	// 62.4% (20°C)
#define		TS_WARM_MASK 0x30	// JEITA VT3 comparator voltage falling thresholds as a percentage of REGN. The corresponding temperature in the brackets is achieved when a 103AT NTC thermistor is used, RT1=5.24kΩ and RT2=30.31kΩ
#define			TS_WARM_40DEG 0x00	// 48.4% (40°C)
#define			TS_WARM_45DEG 0x10	// 44.8% (default) (45°C)
#define			TS_WARM_50DEG 0x20	// 41.2% (50°C)
#define			TS_WARM_55DEG 0x30	// 37.7% (55°C)
#define		BHOT_MASK 0x0c	// OTG mode TS HOT temperature threshold
#define			BHOT_55DEG 0x00	// 55°C
#define			BHOT_60DEG 0x04	// 60°C (default)
#define			BHOT_65DEG 0x08	// 65°C
#define			BHOT_DISABLE 0x0c	// Disable
#define		BCOLD 1
#define		TS_IGNORE 0

#define	REG19_ICO_Current_Limit 0x19 // 2B	R	DS. p. 77 - 7.5.1.22
#define	REG1B_Charger_Status_0 0x1B	// 1B	R	DS. p. 78 - 7.5.1.23
#define		IINDPM_STAT 7
#define		VINDPM_STAT 6
#define		WD_STAT 5
#define		POORSRC_STAT 4
#define		PG_STAT 3
#define		AC2_PRESENT_STAT 2
#define		AC1_PRESENT_STAT 1
#define		VBUS_PRESENT_STAT 0
#define	REG1C_Charger_Status_1 0x1C	// 1B	R	DS. p. 80 - 7.5.1.24
#define		BC12_DONE_STAT 0
#define	REG1D_Charger_Status_2 0x1D	// 1B	R	DS. p. 81 - 7.5.1.25
#define		TREG_STAT 2
#define		DPDM_STAT 1
#define		VBAT_PRESENT_STAT 0
#define	REG1E_Charger_Status_3 0x1E	// 1B	R	DS. p. 82 - 7.5.1.26
#define		ACRB2_STAT 7
#define		ACRB1_STAT 6
#define		ADC_DONE_STAT 5
#define		VSYS_STAT 4
#define		CHG_TMR_STAT 3
#define		TRICHG_TMR_STAT 2
#define		PRECHG_TMR_STAT 1
#define	REG1F_Charger_Status_4 0x1F	// 1B	R	DS. p. 83 - 7.5.1.27
#define		VBATOTG_LOW_STAT 4
#define		TS_COLD_STAT 3
#define		TS_COOL_STAT 2
#define		TS_WARM_STAT 1
#define		TS_HOT_STAT 0
#define	REG20_FAULT_Status_0 0x20	// 1B	R	DS. p. 84 - 7.5.1.28
#define		IBAT_REG_STAT 7
#define		VBUS_OVP_STAT 6
#define		VBAT_OVP_STAT 5
#define		IBUS_OCP_STAT 4
#define		IBAT_OCP_STAT 3
#define		CONV_OCP_STAT 2
#define		VAC2_OVP_STAT 1
#define		VAC1_OVP_STAT 0
#define	REG21_FAULT_Status_1 0x21	// 1B	R	DS. p. 86 - 7.5.1.29
#define		VSYS_SHORT_STAT 7
#define		VSYS_OVP_STAT 6
#define		OTG_OVP_STAT 5
#define		OTG_UVP_STAT 4
#define		TSHUT_STAT 2
#define	REG22_Charger_Flag_0 0x22	// 1B	R	DS. p. 87 - 7.5.1.30
#define		IINDPM_FLAG 7
#define		VINDPM_FLAG 6
#define		WD_FLAG 5
#define		POORSRC_FLAG 4
#define		PG_FLAG 3
#define		AC2_PRESENT_FLAG 2
#define		AC1_PRESENT_FLAG 1
#define		VBUS_PRESENT_FLAG 0
#define	REG23_Charger_Flag_1 0x23	// 1B	R	DS. p. 89 - 7.5.1.31
#define		CHG_FLAG 7
#define		ICO_FLAG 6
#define		VBUS_FLAG 4
#define		TREG_FLAG 2
#define		VBAT_PRESENT_FLAG 1
#define		BC12_DONE_FLAG 0
#define	REG24_Charger_Flag_2 0x24	// 1B	R	DS. p. 90 - 7.5.1.32
#define		DPDM_DONE_FLAG 6
#define		ADC_DONE_FLAG 5
#define		VSYS_FLAG 4
#define		CHG_TMR_FLAG 3
#define		TRICHG_TMR_FLAG 2
#define		PRECHG_TMR_FLAG 1
#define		TOPOFF_TMR_FLAG 0
#define	REG25_Charger_Flag_3 0x25	// 1B	R	DS. p. 91 - 7.5.1.33
#define		VBATOTG_LOW_FLAG 4
#define		TS_COLD_FLAG 3
#define		TS_COOL_FLAG 2
#define		TS_WARM_FLAG 1
#define		TS_HOT_FLAG 0
#define	REG26_FAULT_Flag_0 0x26	// 1B	R	DS. p. 92 - 7.5.1.34
#define		IBAT_REG_FLAG 7
#define		VBUS_OVP_FLAG 6
#define		VBAT_OVP_FLAG 5
#define		IBUS_OCP_FLAG 4
#define		IBAT_OCP_FLAG 3
#define		CONV_OCP_FLAG 2
#define		VAC2_OVP_FLAG 1
#define		VAC1_OVP_FLAG 0
#define	REG27_FAULT_Flag_1 0x27	// 1B	R	DS. p. 94 - 7.5.1.35
#define		VSYS_SHORT_FLAG 7
#define		VSYS_OVP_FLAG 6
#define		OTG_OVP_FLAG 5
#define		OTG_UVP_FLAG 4
#define		TSHUT_FLAG 2
#define	REG28_Charger_Mask_0 0x28	// 1B	RW	DS. p. 95 - 7.5.1.36
#define		IINDPM_MASK 7
#define		VINDPM_MASK 6
#define		WD_MASK 5
#define		POORSRC_MASK 4
#define		PG_MASK 3
#define		AC2_PRESENT_MASK 2
#define		AC1_PRESENT_MASK 1
#define		VBUS_PRESENT_MASK 0
#define	REG29_Charger_Mask_1 0x29	// 1B	RW	DS. p. 97 - 7.5.1.37
#define		CHG_MASK 7
#define		ICO_MASK 6
#define		VBUS_MASK 4
#define		TREG_MASK 2
#define		VBAT_PRESENT_MASK 1
#define		BC12_DONE_MASK 0
#define	REG2A_Charger_Mask_2 0x2A	// 1B	RW	DS. p. 98 - 7.5.1.38
#define		DPDM_DONE_MASK 6
#define		ADC_DONE_MASK 5
#define		VSYS_MASK 4
#define		CHG_TMR_MASK 3
#define		TRICHG_TMR_MASK 2
#define		PRECHG_TMR_MASK 1
#define		TOPOFF_TMR_MASK 0
#define	REG2B_Charger_Mask_3 0x2B	// 1B	RW	DS. p. 99 - 7.5.1.39
#define		VBATOTG_LOW_MASK 4
#define		TS_COLD_MASK 3
#define		TS_COOL_MASK 2
#define		TS_WARM_MASK 1
#define		TS_HOT_MASK 0
#define	REG2C_FAULT_Mask_0 0x2C	// 1B	RW	DS. p. 100 - 7.5.1.40
//#define		IBAT_REG_MASK 7
#define		VBUS_OVP_MASK 6
#define		VBAT_OVP_MASK 5
#define		IBUS_OCP_MASK 4
#define		IBAT_OCP_MASK 3
#define		CONV_OCP_MASK 2
#define		VAC2_OVP_MASK 1
#define		VAC1_OVP_MASK 0
#define	REG2D_FAULT_Mask_1 0x2D	// 1B	RW	DS. p. 101 - 7.5.1.41
#define		VSYS_SHORT_MASK 7
#define		VSYS_OVP_MASK 6
#define		OTG_OVP_MASK 5
#define		OTG_UVP_MASK 4
#define		TSHUT_MASK 2
#define	REG2E_ADC_Control 0x2E	// 1B	RW	DS. p. 102 - 7.5.1.42
#define		ADC_EN 7
#define		ADC_RATE 6
#define		ADC_AVG 3
#define		ADC_AVG_INIT 2
#define	REG2F_ADC_Function_Disable_0 0x2F	// 1B	RW	DS. p. 103 - 7.5.1.43
#define		IBUS_ADC_DIS 7
#define		IBAT_ADC_DIS 6
#define		VBUS_ADC_DIS 5
#define		VBAT_ADC_DIS 4
#define		VSYS_ADC_DIS 3
#define		TS_ADC_DIS 2
#define		TDIE_ADC_DIS 1
#define	REG30_ADC_Function_Disable_1 0x30	// 1B	RW	DS. p. 104 - 7.5.1.44
#define		DP_ADC_DIS 7
#define		DM_ADC_DIS 6
#define		VAC2_ADC_DIS 5
#define		VAC1_ADC_DIS 4
#define	REG31_IBUS_ADC 0x31	// 2B	R	DS. p. 105 - 7.5.1.45
#define	REG33_IBAT_ADC 0x33	// 2B	R	DS. p. 106 - 7.5.1.46
#define	REG35_VBUS_ADC 0x35	// 2B	R	DS. p. 107 - 7.5.1.47
#define	REG37_VAC1_ADC 0x37	// 2B	R	DS. p. 108 - 7.5.1.48
#define	REG39_VAC2_ADC 0x39	// 2B	R	DS. p. 109 - 7.5.1.49
#define	REG3B_VBAT_ADC 0x3B	// 2B	R	DS. p. 110 - 7.5.1.50
#define	REG3D_VSYS_ADC 0x3D	// 2B	R	DS. p. 111 - 7.5.1.51
#define	REG3F_TS_ADC 0x3F	// 2B	R	DS. p. 112 - 7.5.1.52
#define	REG41_TDIE_ADC 0x41	// 2B	R	DS. p. 113 - 7.5.1.53
#define	REG43_DP_ADC 0x43	// 2B	R	DS. p. 114 - 7.5.1.54
#define	REG45_DN_ADC 0x45	// 2B	R	DS. p. 115 - 7.5.1.55
#define	REG47_DPDM_Driver 0x47	// 1B	RW	DS. p. 116 - 7.5.1.56
#define	REG48_Part_Information 0x48	// 1B	R	 DS. p. 117 - 7.5.1.57

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
