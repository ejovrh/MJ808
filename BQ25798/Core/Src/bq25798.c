#include "bq25798.h"

typedef struct	// bq25798_t actual
{
	bq25798_t public;  // publicly visible struct

	uint8_t *__buffer;	// internal buffer for I2C device address and payload data (both r and w)
	I2C_HandleTypeDef *__hi2c;	// I2C HAL handler
} __bq25798_t;

static __bq25798_t   __Device   __attribute__ ((section (".data")));	// preallocate __Device object in .data

extern void Error_Handler(void);
extern I2C_HandleTypeDef hi2c1;
uint8_t _buf[49];  // internal array for register address & payload
//
void __Read(const uint8_t RegAddr, const uint8_t len)
{
	do	// read from device
		{
			if(HAL_I2C_Mem_Read_DMA(&hi2c1, (uint16_t) BQ25798_I2C_ADDR, RegAddr, (uint16_t) len, _buf, (uint16_t) len) != HAL_OK)
				Error_Handler();

			while(HAL_I2C_IsDeviceReady(&hi2c1, (uint16_t) BQ25798_I2C_ADDR, 5, 50) == HAL_TIMEOUT)
				;

			while(HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
				;		// wait for end of transfer

		}
	while(HAL_I2C_GetError(&hi2c1) == HAL_I2C_ERROR_AF);  // retry on acknowledge failure
}

//
void __Write(const uint8_t RegAddr, const uint16_t val)
{
	uint8_t _len;  // holds the number of bytes to be transferred via I2C

	if(val > 0xFF)
		_len = 2;
	else
		_len = 1;

	do  // write to device
		{
			if(HAL_I2C_Mem_Write_DMA(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDR, RegAddr, _len, (uint8_t*) &val, _len) != HAL_OK)  // transmit
				Error_Handler();

			while(HAL_I2C_IsDeviceReady(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDR, 5, 50) == HAL_TIMEOUT)
				;

			while(HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
				;		// wait for end of transfer

		}
	while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);  // retry on acknowledge failure
}
;

bq25798_t* bq25798_ctor(I2C_HandleTypeDef *const in_hi2c)
{
//	uint8_t _buf[49];  // internal array for register address & payload

	__Device.__buffer = _buf;  // point array pointer to internal array
	__Device.__hi2c = in_hi2c;	// HAL's I2C handler

	__Device.public.Read = &__Read;  // read method
	__Device.public.Write = &__Write;  // write method

	__Device.public.Write(REG10_Charger_Control_1, 0x07);
	__Device.public.Read(REG48_Part_Information, 1);
	__Device.public.Read(REG10_Charger_Control_1, 1);

//	__Device.public.Write(REG10_Charger_Control_1, 0x01);  // reset watchdog, set WD timer to 0.5s
//	__Device.public.Write(REG14_Charger_Control_5, 0x80);  // set ship FET to enabled
//
//	__Device.public.Write(REG0F_Charger_Control_0, 0xB2);  //
//	__Device.public.Write(REG10_Charger_Control_1, 0x1B);  //
//	__Device.public.Write(REG11_Charger_Control_2, 0x79);  //
//	__Device.public.Write(REG12_Charger_Control_3, 0x00);  //
//	__Device.public.Write(REG13_Charger_Control_4, 0xD1);  //
//	__Device.public.Write(REG14_Charger_Control_5, 0xA7);  //
//
//	__Device.public.Write(REG00_Minimal_System_Voltage, 0x18);	// 24*0.25=6, 6+2.5 (fixed offset) = 8.5V
//	__Device.public.Write(REG01_Charge_Voltage_Limit, 0x019A);	// 4.1V
//	__Device.public.Write(REG03_Charge_Current_Limit, 0x0096);	// 1.5A
//	__Device.public.Write(REG05_Input_Voltage_Limit, 0xB4);  // 18.0V
//	__Device.public.Write(REG06_Input_Current_Limit, 0x012C);  // 3A
//	__Device.public.Write(REG08_Precharge_Control, 0xC3);  //
//	__Device.public.Write(REG09_Termination_Control, 0x05);  //
//	__Device.public.Write(REG0A_Re_charge_Control, 0x23);  //
//	__Device.public.Write(REG0B_VOTG_regulation, 0x023A);  // 8.5-2.8=5.7, 5.7/0.01=570
//	__Device.public.Write(REG0D_IOTG_regulation, 0x4B);  // 3A
//	__Device.public.Write(REG0E_Timer_Control, 0x79);  //
////	__Device.public.Write(REG15_MPPT_Control, 0x00);	//
////	__Device.public.Write(REG16_Temperature_Control, 0x00);	//
////	__Device.public.Write(REG17_NTC_Control_0, 0x00);	//
////	__Device.public.Write(REG18_NTC_Control_1, 0x00);	//
////	__Device.public.Write(REG28_Charger_Mask_0, 0x00);	//
////	__Device.public.Write(REG29_Charger_Mask_1, 0x00);	//
////	__Device.public.Write(REG2A_Charger_Mask_2, 0x00);	//
////	__Device.public.Write(REG2B_Charger_Mask_3, 0x00);	//
////	__Device.public.Write(REG2C_FAULT_Mask_0, 0x00);	//
////	__Device.public.Write(REG2D_FAULT_Mask_1, 0x00);	//
//	__Device.public.Write(REG2E_ADC_Control, 0xB4);  //
////	__Device.public.Write(REG2F_ADC_Function_Disable_0, 0x00);	//
////	__Device.public.Write(REG30_ADC_Function_Disable_1, 0x00);	//
////	__Device.public.Write(REG47_DPDM_Driver, 0x00);	//

	return (&__Device.public);
}
;

bq25798_t *const Device = &__Device.public;  // set pointer to MsgHandler public part
