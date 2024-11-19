#ifndef MJ514_ADC_C_
#define MJ514_ADC_C_

#if defined(MJ514_)	// if this particular device is active

#include "adc/adc_actual.c"

#define ADC_MEASURE_ITERATIONS 10	// iterations for measurement data value average

static ADC_HandleTypeDef hadc;  // ADC object
extern TIM_HandleTypeDef htim17;  // ADC time base - 10ms

static DMA_HandleTypeDef hdma_adc;	// DMA object

static __adc_t __ADC;  // forward declaration of object

static volatile uint32_t __adc_dma_buffer[ADC_CHANNELS] = {0};	// store for ADC readout
volatile uint32_t __adc_results[ADC_CHANNELS] = {0}; // store ADC average data

static float _VddaConversionConstant;	// constant value pre-computed in constructor
static uint8_t i;  // iterator for average calculation
static uint32_t tempiprop;  // temporary variable for average calculation
static uint32_t tempvrefint;	// ditto
static uint32_t temptemp;  // ditto

// returns value stored at index i
static inline uint16_t _GetChannel(const uint8_t i)
{
	return __adc_results[i];
}

// DMA init - device specific
static inline void _DMAInit(void)
{
  __HAL_RCC_DMA1_CLK_ENABLE();

  hdma_adc.Instance = DMA1_Channel1;
  hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma_adc.Init.Mode = DMA_CIRCULAR;
  hdma_adc.Init.Priority = DMA_PRIORITY_LOW;
  HAL_DMA_Init(&hdma_adc) ;

  __HAL_LINKDMA(&hadc,DMA_Handle,hdma_adc);
}

// starts the ADC & DMA peripherals
static inline void _Start(void)
{
//	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();
	Device->StartTimer(&htim17);	// ADC time base - 10ms
}

// stops the ADC & DMA peripherals
static inline void _Stop(void)
{
	Device->StopTimer(&htim17);	// ADC time base - 10ms
	__HAL_RCC_ADC1_CLK_DISABLE();
//	__HAL_RCC_DMA1_CLK_DISABLE();
}
// ADC init - device specific
static void _ADCInit(void)
{
	ADC_ChannelConfTypeDef sConfig =
		{0};

	hadc.Instance = ADC1;
	hadc.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;  //ADC_CLOCK_SYNC_PCLK_DIV4
	hadc.Init.Resolution = ADC_RESOLUTION_12B;
	hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
	hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc.Init.LowPowerAutoWait = ENABLE;
	hadc.Init.LowPowerAutoPowerOff = ENABLE;
	hadc.Init.ContinuousConvMode = DISABLE;
	hadc.Init.DiscontinuousConvMode = DISABLE;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc.Init.DMAContinuousRequests = ENABLE;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	__HAL_RCC_ADC1_CLK_ENABLE();
	HAL_ADC_Init(&hadc);

	// channel order has to match mj828_adcchannels enum
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

	sConfig.Channel = ADC_CHANNEL_MOTOR_IPROP;	// channel 4 - motor proportional current - PA4
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;  // channel 16 -	built-in temperature sensor
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_VREFINT;	// channel 17 - Vrefint - see RM0091, 13.8, p. 260
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	HAL_ADCEx_Calibration_Start(&hadc);

	HAL_ADC_Start_DMA(&hadc, (uint32_t *) __adc_dma_buffer, ADC_CHANNELS);	// start DMA

	_Stop();	// init finished, power off peripheral
}

// DMA ISR executed function for ADC computation tasks
static void _Do(void)
{
	tempiprop += __adc_dma_buffer[Iprop];	// ditto
	temptemp += __adc_dma_buffer[Temperature];  // ...
	tempvrefint += __adc_dma_buffer[Vrefint];	// ...

	if(++i == ADC_MEASURE_ITERATIONS)
		{
			tempiprop /= ADC_MEASURE_ITERATIONS;  // ditto
			temptemp /= ADC_MEASURE_ITERATIONS;  // ...
			tempvrefint /= ADC_MEASURE_ITERATIONS;	// ...

			/* ADC channel voltage calculation - see RM 0091, chapter 13.8, p. 260
			 *
			 * using the Vrefint reference channel, the formula for calculating the ADC channel voltage is:
			 * 	Vchannel = (Vdda_charact. * Vrefint_cal * ADC_data) / (Vrefint_data * full scale)
			 *
			 * 	of these, only ADC_data and Vrefint_data are variable, the rest are constants which can be computed in advance
			 * 	thus, the formula becomes:
			 *
			 * 	Vchannel = (ADC_data/Vrefint_data) * ( (Vdda_charact. * Vrefint_cal) / full scale )
			 * 		the latter term is computed once in the constructor.
			 * 		in absolute numbers it is 4915.7509157509157
			 *
			 * 	Vchannel becomes (ADC_data/Vrefint_data) * VddaConversionConstant, true voltage in mV
			 * 	this is then typecast into uint16_t to have a nice round number
			 */
			__adc_results[Iprop] = (uint16_t) ((float) tempiprop / tempvrefint * _VddaConversionConstant); // store computed average in result buffer

			/* ADC temperature calculation - see RM0091, chapter 13.8, p. 259
			 *
			 *	https://techoverflow.net/2015/01/13/reading-stm32f0-internal-temperature-and-voltage-using-chibios/
			 */
			__adc_results[Temperature] = (((((float) (temptemp * VREFINT_CAL) / tempvrefint) - TS_CAL1) * 800) / (int16_t) (TS_CAL2 - TS_CAL1)) + 300;
			__adc_results[Vrefint] = tempvrefint; // store computed average in result buffer

			tempiprop = 0;
			temptemp = 0;
			tempvrefint = 0;
			i = 0;
		}
}

adc_t* adc_ctor(void)
{
	_DMAInit();	// initialize DMA
	_ADCInit();  // initialize ADC

	__ADC.__adc_results = __adc_results;  // tie in ADC readout destination
	__ADC._Do = &_Do;	// DMA ISR executed function for ADC computation tasks
	__ADC.public.GetChannel = &_GetChannel;  // set function pointer
	__ADC.public.Start = &_Start;  // ditto
	__ADC.public.Stop = &_Stop;  // ditto

	_VddaConversionConstant = (float) (3300 * VREFINT_CAL) / 4095;  // 3300 - 3.3V for mV, 4 for resistor divider

	return &__ADC.public;  // return public parts
}

// DMA ISR - due to mj828 timer2's MasterOutputTrigger = TIM_TRGO_UPDATE it is almost a timer2 250ms ISR
void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc);	// service the interrupt

  __ADC._Do();	// ADC computation tasks on every conversion
}

void TIM17_IRQHandler(void)
{
	HAL_ADC_Start_IT(&hadc);  // Trigger ADC conversion
	HAL_TIM_IRQHandler(&htim17);  // service the interrupt
}

#endif // MJ514_

#endif /* MJ514_ADC_C_ */
