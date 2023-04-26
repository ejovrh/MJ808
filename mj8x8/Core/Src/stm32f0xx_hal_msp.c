#include "main.h"

void HAL_MspInit(void)	// Initializes the Global MSP
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_REMAP_PIN_ENABLE(HAL_REMAP_PA11_PA12);
}
