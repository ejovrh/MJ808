#ifndef CORE_INC_SPI_SPI_H_
#define CORE_INC_SPI_SPI_H_

#if USE_SPI

typedef struct
{
//	uint8_t TXDone:1; 	//
//	uint8_t RXDone:1; 	//

	void (*Transmit)(const uint16_t DevAddr, uint8_t *buffer, const uint8_t size); // SPI TX of up to n bytes of data
	void (*Receive)(const uint16_t DevAddr, uint8_t *buffer, const uint8_t size); // SPI RX of up to n bytes of data
} spi_t;

spi_t* spi_ctor(const uint32_t _MOSI_pin,  const uint32_t _MISO_pin, const uint32_t _SCL_Pin, GPIO_TypeDef *_I2C_Port);

#endif

#endif /* CORE_INC_SPI_SPI_H_ */
