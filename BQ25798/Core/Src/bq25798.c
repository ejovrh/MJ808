/*
 * bq25798.c
 *
 *  Created on: May 11, 2021
 *      Author: hrvoje
 */
#include "bq25798.h"


typedef struct																						// bq25798_t actual
{
		bq25798_t public;																			// publicly visible struct

		uint8_t *__buffer;																			// internal buffer for I2C device address and payload data (both r and w)
		I2C_HandleTypeDef	*__hi2c;																// I2C HAL handler
} __bq25798_t;

static __bq25798_t __Device __attribute__ ((section (".data")));									// preallocate __Device object in .data

uint16_t __Read(const uint8_t addr, const uint8_t len)
{
	__Device.__buffer[0] = addr;																	// store register address

    do
    {
      if(HAL_I2C_Master_Transmit_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, __Device.__buffer, (uint16_t) 1) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);

    do
    {
      if(HAL_I2C_Master_Receive_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, __Device.__buffer, (uint16_t) len) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);

    if (len == 2)																					// if we want to read 2 bytes
    	return ( __Device.__buffer[0] << 8 | __Device.__buffer[1] );								// do some shifting so that high and low byte of an uint16_t get into the appropriate array position
    else																							// else we read only 1 byte
    	return (__Device.__buffer[0]);
};

void __Write(const uint8_t addr, const uint16_t val)
{
	uint8_t __len;																					// holds the number of bytes to be transferred via I2C
	__Device.__buffer[0] = addr;																	// store register address

	if (val > 0x00FF)																				// if we want to write 2 bytes
	{
		__len = 3;																					// set number of bytes - reg. addr (one byte) plus 2 bytes of payload (effectively one uint16_t)
		__Device.__buffer[1] = (val & 0xFF00) >> 8;													// put uint16_t's high byte into position
		__Device.__buffer[2] = (uint8_t) val;														// put uint16_t's low byte into position
	}
	else																							// else we write only one byte
	{
		__len = 2;																					// set number of bytes - reg. addr. plus one byte of payload
		__Device.__buffer[1] = (uint8_t) val;														// put only lower uint16_t byte into position
	}

    do
    {
      if(HAL_I2C_Master_Transmit_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, __Device.__buffer, (uint16_t) __len) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);
};

bq25798_t *bq25798_ctor(I2C_HandleTypeDef * const in_hi2c)
{
	uint8_t _buf[2];																				// internal array for register address & payload

	__Device.__buffer = _buf;																		// point array pointer to internal array
	__Device.__hi2c = in_hi2c;																		// HAL's I2C handler

	__Device.public.Read = &__Read;																	// read method
	__Device.public.Write = &__Write;																// write method

	return (&__Device.public);
};


bq25798_t * const Device = &__Device.public ;														// set pointer to MsgHandler public part
