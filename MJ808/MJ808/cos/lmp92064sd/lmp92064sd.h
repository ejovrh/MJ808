#ifndef LMP92064SD_H_
#define LMP92064SD_H_

#include <inttypes.h>




#define LMP92064SD_CONFIG_A		0x0000									// RW, default 0x18, datasheet p.13
#define LMP92064SD_CONFIG_A_RESET				0x07					//	RW
#define LMP92064SD_CONFIG_A_DDIR				6						//	RO
#define LMP92064SD_CONFIG_A_ADDRDIR				5						//	RO
#define LMP92064SD_CONFIG_A_SDDIR				4						//	RO
#define LMP92064SD_CONFIG_A_3					4						//
#define LMP92064SD_CONFIG_A_2					5						//
#define LMP92064SD_CONFIG_A_1					6						//
#define LMP92064SD_CONFIG_A_0					7						//

#define LMP92064SD_CONFIG_B		0x0001									// RW, default 0x00, datasheet p.14
#define LMP92064SD_CONFIG_B_STREAM				7						//	RO
#define LMP92064SD_CONFIG_B_BUFREG_RD			5						//	RW

#define LMP92064SD_CHIP_TYPE	0x0003									// RW, default 0x07, datasheet p.14, CHIP_TYPE[7:0]

#define LMP92064SD_CHIP_ID_LSB	0x0004									// RO, default 0x00, datasheet p.14
#define LMP92064SD_CHIP_ID_MSB	0x0005									// RO, default 0x04, datasheet p.14
#define LMP92064SD_CHIP_REV		0x0006									// RO, default 0x01, datasheet p.14
#define LMP92064SD_MFR_ID_LSB	0x000C									// RO, default 0x51, datasheet p.15
#define LMP92064SD_MFR_ID_MSB	0x000D									// RO, default 0x04, datasheet p.15

#define LMP92064SD_REG_UPDATE	0x000F									// RW, default 0x00, datasheet p.15
#define LMP92064SD_REG_UPDATE_BUFREG_UPDATE		0						//	RW

#define LMP92064SD_CONFIG_REG	0x0100									// RW, default 0x00, datasheet p.15

#define LMP92064SD_STATUS		0x0103									// RO, datasheet p.15
#define LMP92064SD_STATUS_STATUS				0						//	RO

#define LMP92064SD_DATA_VOUT_LSB	0x0200								// RO, datasheet p.16, VOUT_DATA_LSB[7:0]
#define LMP92064SD_DATA_VOUT_MSB	0x0201								// RO, datasheet p.16, VOUT_DATA_MSB[7:0]
#define LMP92064SD_DATA_COUT_LSB	0x0202								// RO, datasheet p.16, COUT_DATA_LSB[7:0]
#define LMP92064SD_DATA_COUT_MSB	0x0203								// RO, datasheet p.16, COUT_DATA_MSB[7:0]

typedef struct lmp92064sd_t												// lmp92064sd_t actual struct describing the Voltage/Current meter as a whole
{
	void (* const DownloadData)(uint8_t *data_array);					// returns 2byte voltage value
} lmp92064sd_t __attribute__((aligned(8)));

//lmp92064sd_t *lmp92064sd_ctor();										// lmp92064sd_t object constructor - does function pointer & hardware initialization

#endif /* LMP92064SD_H_ */