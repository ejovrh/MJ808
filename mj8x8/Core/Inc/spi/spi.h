#ifndef CORE_INC_SPI_SPI_H_
#define CORE_INC_SPI_SPI_H_

#include "main.h"

#if USE_SPI

typedef struct
{
//	SPI_HandleTypeDef hspi;  // SPI handle
	void (*tx)(void);  //
} spi_t;

void spi_ctor(void);

extern spi_t *const SPI;

#endif

#endif /* CORE_INC_SPI_SPI_H_ */
