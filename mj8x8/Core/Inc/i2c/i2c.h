#ifndef CORE_INC_I2C_I2C_H_
#define CORE_INC_I2C_I2C_H_

#if USE_I2C

typedef struct
{
//	uint8_t TXDone:1; 	//
//	uint8_t RXDone:1; 	//

	void (*Read)(const uint16_t DevAddr, const uint16_t RegAddr, uint8_t *buffer, const uint8_t size);
	void (*Write)(const uint16_t DevAddr, const uint16_t RegAddr, uint8_t *buffer, const uint8_t size);
	void (*Transmit)(const uint16_t DevAddr, uint8_t *buffer, const uint8_t size);
	void (*Receive)(const uint16_t DevAddr, uint8_t *buffer, const uint8_t size);
} i2c_t;

i2c_t* i2c_ctor(const uint32_t SDA_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *I2C_Port);

#endif

#endif /* CORE_INC_I2C_I2C_H_ */
