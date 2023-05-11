#ifndef MJ838_ZEROCROSS_C_
#define MJ838_ZEROCROSS_C_

#if defined(MJ838_)	// if this particular device is active

#include "zerocross/zerocross_actual.c"

extern TIM_HandleTypeDef htim2;  // Timer2 object - input capture of zero-cross signal on rising edge
extern TIM_HandleTypeDef htim3;  // Timer3 object - measurement interval of timer2 data
static DMA_HandleTypeDef hdma_tim2_ch1;	// zero-cross frequency measurement

static __zerocross_t __ZeroCross;  // forward declaration of object

uint32_t _zerocross_buffer[ZC_BUFFER_LEN] =
	{0};	// stores timer2 counter readout

volatile float _freq_buffer = 0;	// container for average frequency calculation
volatile uint8_t _zcValues = 0;	// iterator for average frequency calculation
volatile uint8_t _sleep = 0;	// timer3-based count for sleep since last zero-cross detection

// returns measured wheel frequency
static inline uint16_t _GetWheelFrequency(const uint8_t i)
{
	return __ZeroCross._WheelFrequency;
}

// DMA init - device specific
static inline void __DMAInit(void)
{
	__HAL_RCC_DMA1_CLK_ENABLE();
	hdma_tim2_ch1.Instance = DMA1_Channel5;
	hdma_tim2_ch1.Init.Direction = DMA_PERIPH_TO_MEMORY;
	hdma_tim2_ch1.Init.PeriphInc = DMA_PINC_DISABLE;
	hdma_tim2_ch1.Init.MemInc = DMA_MINC_ENABLE;
	hdma_tim2_ch1.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	hdma_tim2_ch1.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	hdma_tim2_ch1.Init.Mode = DMA_CIRCULAR;
	hdma_tim2_ch1.Init.Priority = DMA_PRIORITY_LOW;
	HAL_DMA_Init(&hdma_tim2_ch1);

	__HAL_DMA_DISABLE_IT(&hdma_tim2_ch1, DMA_IT_HT | DMA_IT_TE);	// disable error & half-fransfer interrupts
	__HAL_LINKDMA(&htim2, hdma[TIM_DMA_ID_CC1], hdma_tim2_ch1);
}

// starts the timer & DMA peripherals
static inline void _Start(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__disable_irq();	// disable interrupts until end of initialisation
	Device->mj8x8->UpdateActivity(ZEROCROSS, ON);	// mark device as on
	HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);	// disable EXTI0 - we will use a different mode from now on...

	// configure to timer2 input-capture mode
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);

	Device->StartTimer(&htim2);	// start zero-cross input capture timer

	__DMAInit();	// initialise DMA

	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, _zerocross_buffer, ZC_BUFFER_LEN);	// start timer2 DMA
	Device->StartTimer(&htim3);	// start measurement interval timer

	__enable_irq();  // enable interrupts
}

// stops the timer & DMA peripherals
static inline void _Stop(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__disable_irq();	// disable interrupts until end of initialisation

	Device->StopTimer(&htim3);	// stop measurement interval timer
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);	// stop timer2 DMA
	__HAL_RCC_DMA1_CLK_DISABLE();	// turn off peripheral
	Device->StopTimer(&htim2);	// stop zero-cross input capture timer

	// configure to GPIO EXTI0 mode
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;  // catch zero cross activity (idle to first impulse and rolling)
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);

	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);	// enable EXTI0 for wakeup from stop mode

	Device->mj8x8->UpdateActivity(ZEROCROSS, OFF);	// mark device as off

	__enable_irq();  // enable interrupts
}

zerocross_t* zerocross_ctor(void)
{
	__ZeroCross.__buffer = _zerocross_buffer;  // tie in timer2 counter readout destination

	__ZeroCross.public.GetWheelFrequency = &_GetWheelFrequency;  // set function pointer
	__ZeroCross.public.Start = &_Start;  // ditto
	__ZeroCross.public.Stop = &_Stop;  // ditto

	HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);  // event handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 - zero-cross detection of first dynamo impulse after standstill
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);  // timer2 zero-cross signal frequency measurement via DMA
	HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

	return &__ZeroCross.public;  // return public parts
}

// EXTI0 ISR - standstill to first impulse: wakeup and activate zero-cross functionality
void EXTI0_1_IRQHandler(void)
{
	Device->mj8x8->StartCoreTimer();  // start core timer

	if(__HAL_GPIO_EXTI_GET_IT(ZeroCross_Pin))  // interrupt source detection
		Device->ZeroCross->Start();	// start zero-cross detection

	HAL_GPIO_EXTI_IRQHandler(ZeroCross_Pin);  // service the interrupt
}

// DMA ISR - zero-cross frequency measurement
void DMA1_Channel4_5_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_tim2_ch1);  // service the interrupt

	/* rollover: the counter goes up to 4294967295.
	 *  the prescaler is 10000, which means the rollover will be in 119 hours. for the purpose of this device - never.
	 */
	if ((_zerocross_buffer[1] > _zerocross_buffer[0]))	// very crude guard against rollover
		{
			_freq_buffer += (float) (10000.0 / (_zerocross_buffer[1] - _zerocross_buffer[0]));	// add up values for division later on
			++_zcValues;	// how many times did we add?
		}

	Device->AutoDrive->Do();  // run the AutoLight feature
}

// timer 3 ISR - 250ms interrupt - frequency measurement timer
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt

	// once timer3 ISR fires, calculate:
	__ZeroCross._DynamoFrequency = _freq_buffer / _zcValues;	// average dynamo AC frequency
	__ZeroCross._WheelFrequency = __ZeroCross._DynamoFrequency / SON_DYNAMO_CORRECTION;	// derive wheel rotations per second
	__ZeroCross._ms = __ZeroCross._WheelFrequency * WHEEL_CIRCUMFERENCE;	// derive speed in m/s
	__ZeroCross._kmh = __ZeroCross._ms * 3.6;	// derive speed in km/h

	if (_freq_buffer < 1)	// no rotation detected (timer3 clears the variable after one iteration)
			++_sleep;

	if (_sleep > SLEEPTIMEOUT_COUNTER) // sleep timeout expired - stop zero-cross
		{
			__ZeroCross._DynamoFrequency = 0;	// zero out values
			__ZeroCross._WheelFrequency = 0;
			__ZeroCross._ms = 0;
			__ZeroCross._kmh = 0;
			_sleep = 0;
			Device->ZeroCross->Stop();	// stop zero-cross detection; timer1 will put the device into stop mode soon
		}

	_freq_buffer = 0;	// zero & start over
	_zcValues = 0;
}

#endif // MJ838_

#endif /* MJ838_ZEROCROSS_C_ */
