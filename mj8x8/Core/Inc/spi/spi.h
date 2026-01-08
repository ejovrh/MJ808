#ifndef CORE_INC_SPI_SPI_H_
#define CORE_INC_SPI_SPI_H_

#if USE_SPI

typedef struct
{
	void (*Transmit)(uint8_t *txData, const uint16_t size);  // SPI TX of up to n bytes of data
	void (*Receive)(uint8_t *rxData, const uint16_t size);  // SPI RX of up to n bytes of data
	void (*TransmitReceive)(uint8_t *txData, uint8_t *rxData, const uint16_t size); // SPI TX/RX full-duplex
} spi_t;

spi_t* spi_ctor(const uint32_t MOSI_Pin, const uint32_t MISO_Pin, const uint32_t SCL_Pin, GPIO_TypeDef *SPI_Port);

#endif

#endif /* CORE_INC_SPI_SPI_H_ */
