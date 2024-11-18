#ifndef CORE_INC_I2C_I2C_H_
#define CORE_INC_I2C_I2C_H_

#if USE_I2C

typedef struct
{
	I2C_HandleTypeDef *I2C;  // I2C handle

	void (*Read)(const uint16_t DevAddr, uint16_t RegAddr, uint16_t *data, uint16_t size);
	void (*Write)(const uint16_t DevAddr, const uint16_t MemAddr, uint16_t const *data, uint16_t MemAddrSize);
} i2c_t;

i2c_t* i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port);

#endif

#endif /* CORE_INC_I2C_I2C_H_ */
