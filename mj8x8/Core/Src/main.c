#include "main.h"	// device core config

void SystemClock_Config(void);	// clock configuration prototype

#if defined(MJ808_)	// front light header include
#include "mj808/mj808.h"
#endif
#if defined(MJ818_)	// rear light header include
#include "mj818/mj818.h"
#endif
#if defined(MJ828_)	// user interface header include
#include "mj828/mj828.h"
#endif
#if defined(MJ838_)	// Čos header include
#include "mj838/mj838.h"
#endif
#if defined(MJ848_)	// logic unit header include
#include "mj848/mj848.h"
#endif

int main(void)
{
	event_handler_ctor();  // call event handler constructor; the Device constructor further down has the chance to override EventHandler.fpointer and implement its own handler

#if defined(MJ808_)	// MJ808 - call derived class constructor and tie in base class
	mj808_ctor();
#endif
#if defined(MJ818_)	// MJ818 - call derived class constructor and tie in base class
	mj818_ctor();
#endif
#if defined(MJ828_)	// MJ828 - call derived class constructor and tie in base class
	mj828_ctor();
#endif
#if defined(MJ838_)	// Čos - call derived class constructor and tie in base class
	cos_ctor();
#endif
#if defined(MJ848_)	// LU - call derived class constructor and tie in base class
	lu_ctor();
#endif

	message_handler_ctor(Device->mj8x8->can);  // call message handler constructor

	while(1)  // THE loop...
		{
			EventHandler->HandleEvent();	// execute the event handling function with argument taken from case table array
		}
}

#if ( defined(MJ808_) | defined(MJ828_) )	// ISR for timers 1 A compare match - button handling
ISR1( TIMER1_COMPA_vect)	// timer/counter 1 - button debounce - 25ms
{
	// code to be executed every 25ms
	sleep_disable();	// wakey wakey

	Device->button->deBounce();  // call the debouncer

	sleep_enable();  // back to sleep
}

#if defined(MJ828_)	// ISR for timer0 - 16.25ms - charlieplexing timer
ISR2( TIMER0_COMPA_vect)	// timer/counter0 - 16.25ms - charlieplexed blinking
{
	Device->led->Handler();	// handles LEDs according to CAN message (of type CMND_UTIL_LED)
}
#endif

#endif

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
