#ifndef MJ828_ADC_C_
#define MJ828_ADC_C_

#if defined(MJ828_)	// if this particular device is active

#include "adc\adc_actual.c"

static ADC_HandleTypeDef hadc;  // ADC object
extern TIM_HandleTypeDef htim2;  // Timer2 object - ADC conversion - 500ms

static __adc_t __ADC;  // forward declaration of object

static uint16_t __adc_buffer[ADC_CHANNELS] =
	{0};	// store for ADC readout

// reads channels as called from ADC ISR
static void _ConversionEnd(void)
{
	__ADC.__buffer[__ADC.__index] = HAL_ADC_GetValue(&hadc);	// store channel value at proper position in buffer

	(__ADC.__index == ADC_CHANNELS-1) ? __ADC.__index = 0 : ++__ADC.__index;  // count until ADC_CHANNELS - 1, then start over
}

// returns value stored at index i
static inline uint16_t _GetChannel(const uint8_t i)
{
	return __ADC.__buffer[i];
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
	hadc.Init.DMAContinuousRequests = DISABLE;
	hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;
	__HAL_RCC_ADC1_CLK_ENABLE();
	HAL_ADC_Init(&hadc);

	sConfig.Channel = ADC_CHANNEL_3;	//	Battery voltage - PA3
	sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_9;	//	Phototransistor / PB1
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;  //	built-in temperature sensor
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	sConfig.Channel = ADC_CHANNEL_VREFINT;	// Vrefint - see RM0091, 13.8, p. 260
	HAL_ADC_ConfigChannel(&hadc, &sConfig);

	HAL_ADCEx_Calibration_Start(&hadc);

	HAL_ADC_Start_IT(&hadc);
}

// starts the ADC peripheral
static inline void _Start(void)
{
	__HAL_RCC_ADC1_CLK_ENABLE();
	Device->StartTimer(&htim2);
}

// stops the ADC peripheral
static inline void _Stop(void)
{
	Device->StopTimer(&htim2);
	__HAL_RCC_ADC1_CLK_DISABLE();
}

adc_t* adc_ctor(void)
{
	_ADCInit();  // initialize ADC

	__ADC.__index = 0;	// initialise
	__ADC.__buffer = __adc_buffer;  // tie in ADC readout destination

	__ADC.public.GetChannel = &_GetChannel;  // set function pointer
	__ADC.public.ConversionEnd = &_ConversionEnd;  // ditto
	__ADC.public.Start = &_Start;  // ditto
	__ADC.public.Stop = &_Stop;  // ditto

	HAL_NVIC_SetPriority(ADC1_IRQn, 0, 0);	// ADC interrupt handling
	HAL_NVIC_EnableIRQ(ADC1_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);  // DMA interrupt handling
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	return &__ADC.public;  // return public parts
}

#endif // MJ828_

#endif /* MJ828_ADC_C_ */
