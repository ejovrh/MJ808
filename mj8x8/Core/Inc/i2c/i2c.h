#ifndef CORE_INC_I2C_I2C_H_
#define CORE_INC_I2C_I2C_H_

#if USE_I2C

typedef struct
{
	I2C_HandleTypeDef *I2C;  // I2C handle

	uint32_t (*Read)(const uint16_t DevAddr, const uint16_t RegAddr, const uint8_t size);
	void (*Write)(const uint16_t DevAddr, const uint16_t RegAddr, const uint32_t data, const uint8_t size);
} i2c_t;

i2c_t* i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port);

#endif

#endif /* CORE_INC_I2C_I2C_H_ */
