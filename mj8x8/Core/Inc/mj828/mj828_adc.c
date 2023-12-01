#ifndef MJ828_ADC_C_
#define MJ828_ADC_C_

#if defined(MJ828_)	// if this particular device is active

#include "adc\adc_actual.c"

static ADC_HandleTypeDef hadc;  // ADC object
extern TIM_HandleTypeDef htim2;  // Timer2 object - ADC conversion - 250ms
static DMA_HandleTypeDef hdma_adc;	// DMA object

static __adc_t __ADC;  // forward declaration of object

volatile uint32_t __adc_buffer[ADC_CHANNELS] =
	{0};	// store for ADC readout

// returns value stored at index i
static inline uint16_t _GetChannel(const uint8_t i)
{
	return __adc_buffer[i];
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
	hadc.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
	hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
	hadc.Init.DMAContinuousRequests = ENABLE;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	__HAL_RCC_ADC1_CLK_ENABLE();
	HAL_ADC_Init(&hadc);

	// channel order has to match mj828_adcchannels enum
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;

	sConfig.Channel = ADC_CHANNEL_PHOTOTRANSISTOR;	// channel 1 - Phototransistor - PA1
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_VBATT;	// channel 8 -Battery voltage - PB0
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;  // channel 16 -	built-in temperature sensor
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_VREFINT;	// channel 17 - Vrefint - see RM0091, 13.8, p. 260
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	HAL_ADCEx_Calibration_Start(&hadc);
}

// starts the ADC & DMA peripherals
static inline void _Start(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();
	Device->StartTimer(&htim2);	// Timer2 object - ADC conversion - 250ms
}

// stops the ADC & DMA peripherals
static inline void _Stop(void)
{
	Device->StopTimer(&htim2);	// Timer2 object - ADC conversion - 250ms
	__HAL_RCC_ADC1_CLK_DISABLE();
	__HAL_RCC_DMA1_CLK_DISABLE();
}

adc_t* adc_ctor(void)
{
	_DMAInit();	// initialize DMA
	_ADCInit();  // initialize ADC

	__ADC.__buffer = __adc_buffer;  // tie in ADC readout destination

	__ADC.public.GetChannel = &_GetChannel;  // set function pointer
	__ADC.public.Start = &_Start;  // ditto
	__ADC.public.Stop = &_Stop;  // ditto

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);  // DMA interrupt handling
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	HAL_ADC_Start_DMA(&hadc, (uint32_t *)__ADC.__buffer, ADC_CHANNELS);	// start DMA

	return &__ADC.public;  // return public parts
}

// DMA ISR - due to mj828 timer2's MasterOutputTrigger = TIM_TRGO_UPDATE it is almost a timer2 250ms ISR
void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_adc);	// service the interrupt

	Device->autolight->Do();  // run the AutoLight feature
	Device->autobatt->Do();  // run the AutoBatt feature
}

#endif // MJ828_

#endif /* MJ828_ADC_C_ */
