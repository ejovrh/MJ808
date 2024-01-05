#ifndef CORE_INC_I2C_I2C_H_
#define CORE_INC_I2C_I2C_H_

#include "main.h"

#if USE_I2C

typedef struct
{
	void (*Read)(const uint16_t DevAddr, uint16_t RegAddr, uint16_t *data, uint16_t size);
	void (*Write)(const uint16_t DevAddr, const uint16_t MemAddr, uint16_t const *data, uint16_t MemAddrSize);
} i2c_t;

void i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port);

extern i2c_t *const I2C;

#endif

#endif /* CORE_INC_I2C_I2C_H_ */
