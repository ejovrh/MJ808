#include "main.h"	// device core config

void SystemClock_Config(void);	// clock configuration prototype

int main(void)
{
	HAL_Init();  //Reset of all peripherals, Initializes the Flash interface and the Systick

	SystemClock_Config();  //Configure the system clock

	while(1)  // THE loop...
		{
			;
		}
}

void SystemClock_Config(void)  //System Clock Configuration
{
	RCC_OscInitTypeDef RCC_OscInitStruct =
		{0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct =
		{0};

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
		{
			Error_Handler();
		}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

	if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
		{
			Error_Handler();
		}
}

void Error_Handler(void)  //This function is executed in case of error occurrence
{
	__disable_irq();
	while(1)
		{
			;
		}
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)	//Reports the name of the source file and the source line number where the assert_param error has occurred
{
	;
}
#endif
