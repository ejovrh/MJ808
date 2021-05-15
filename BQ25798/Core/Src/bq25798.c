/*
 * bq25798.c
 *
 *  Created on: May 11, 2021
 *      Author: hrvoje
 */
#include "bq25798.h"


typedef struct																						// bq25798_t actual
{
		bq25798_t public;																			// public struct

		I2C_HandleTypeDef	*__hi2c;																	//
} __bq25798_t;

static __bq25798_t __Device __attribute__ ((section (".data")));									// preallocate __Device object in .data

/*
void __TXn(uint8_t * const buf, const uint8_t n)
{
    do
    {
      if(HAL_I2C_Master_Transmit_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, buf, n) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);
};

void __RXn(uint8_t * const buf, const uint8_t n)
{
    do
    {
      if(HAL_I2C_Master_Receive_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, buf, n) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);
};
*/

void __Read(uint8_t * const buf, const uint8_t n) // @suppress("Name convention for function")
{
    do
    {
      if(HAL_I2C_Master_Transmit_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, buf, (uint16_t) 1) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);

    do
    {
      if(HAL_I2C_Master_Receive_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, buf, (uint16_t) n) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);
};

void __Write(uint8_t * const buf, const uint8_t n)
{
    do
    {
      if(HAL_I2C_Master_Transmit_IT(__Device.__hi2c, (uint16_t) BQ25798_I2C_ADDRESS_LSHIFTED, buf, (uint16_t) n) != HAL_OK)
        Error_Handler();

      while (HAL_I2C_GetState(__Device.__hi2c) != HAL_I2C_STATE_READY)
    	  ;

    } while(HAL_I2C_GetError(__Device.__hi2c) == HAL_I2C_ERROR_AF);
};

bq25798_t *bq25798_ctor(I2C_HandleTypeDef * const in_hi2c)
{
	__Device.__hi2c = in_hi2c;
	__Device.public.Read = &__Read;
	__Device.public.Write = &__Write;

	return (&__Device.public);
};


bq25798_t * const Device = &__Device.public ;														// set pointer to MsgHandler public part
