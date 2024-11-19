#ifndef CORE_INC_I2C_I2C_H_
#define CORE_INC_I2C_I2C_H_

#if USE_I2C

typedef struct
{
	I2C_HandleTypeDef *I2C;  // I2C handle

	uint32_t (*Read)(const uint16_t DevAddr, const uint16_t RegAddr, const uint8_t size); // I2C read of up to 4 bytes of data from device
	void (*Write)(const uint16_t DevAddr, const uint16_t RegAddr, const uint32_t data, const uint8_t size); // I2C write of up to 4 bytes of data into device
} i2c_t;

i2c_t* i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port);

#endif

#endif /* CORE_INC_I2C_I2C_H_ */
