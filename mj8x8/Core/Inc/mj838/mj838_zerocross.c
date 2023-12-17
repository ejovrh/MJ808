#ifndef MJ838_ZEROCROSS_C_
#define MJ838_ZEROCROSS_C_

#if defined(MJ838_)	// if this particular device is active

#include "zerocross/zerocross_actual.c"

extern TIM_HandleTypeDef htim2;  // Timer2 object - input capture of zero-cross signal on rising edge
extern TIM_HandleTypeDef htim3;  // Timer3 object - measurement/calculation interval of timer2 data - default 250ms
static DMA_HandleTypeDef hdma_tim2_ch1;	// zero-cross frequency measurement

static __zerocross_t __ZeroCross;  // forward declaration of object

uint32_t _zc_counter_buffer[2] =
	{0};	// stores timer2 counter readout

volatile uint32_t _zc_counter_delta = 0;	// container for average frequency calculation
volatile uint16_t _zcValues = 0;	// iterator for average frequency calculation
volatile uint8_t _sleep = 0;	// timer3-based count for sleep since last zero-cross detection

// computes Zero-Cross signal frequency
static void  _CalculateZCFrequency(void)
{
  if (_zc_counter_delta)	// if there is data...
  {
#if SIGNAL_GENERATOR_INPUT
  	__ZeroCross._ZeroCrossFrequency = (800000000 / (_zc_counter_delta / _zcValues) ) / 100.0;	// "round" and average dynamo AC frequency
#else
  	__ZeroCross._ZeroCrossFrequency = 8000000.0 / (_zc_counter_delta / _zcValues);	// average dynamo AC frequency
#endif

  	if (__ZeroCross._ZeroCrossFrequency >= 5)
  		__HAL_TIM_SET_AUTORELOAD(&htim3, 2499);	// 250ms

  	if (__ZeroCross._ZeroCrossFrequency < 5)
    		__HAL_TIM_SET_AUTORELOAD(&htim3, 4999);	// 500ms

  	if (__ZeroCross._ZeroCrossFrequency < 2)
  		__HAL_TIM_SET_AUTORELOAD(&htim3, 9999);	// 1s

  	if (__ZeroCross._ZeroCrossFrequency < 1)
  		__HAL_TIM_SET_AUTORELOAD(&htim3, 19999);	// 2s
// FIXME - _CalculateZCFrequency() - correct transitions to _ZeroCrossFrequency == 0 and from 0 (i.e. motion to standstill and back to motion)
  	// at above 3.8 kps (a convenient 1 mps) the dynamo waveform under load is too unstable to provide accurate frequency measurements so any substantial load has to be switched off
  	if (__ZeroCross._ZeroCrossFrequency == 0)
  		__HAL_TIM_SET_AUTORELOAD(&htim3, 2499);	// 250ms

  }
  else	// no data
  {
      __ZeroCross._ZeroCrossFrequency = 0;
      ++_sleep;
  }

  if (_sleep > SLEEPTIMEOUT_COUNTER)	// n iterations of sleep
  {
      __ZeroCross._ZeroCrossFrequency = 0;	// zero & start over
      _sleep = 0;
      Device->ZeroCross->Stop();	// stop zero-cross detection; timer1 will put the device into stop mode
  }

  _zc_counter_delta = 0;	// zero & start over
  _zcValues = 0;
}

// returns computed Zero-Cross signal frequency
static inline float _GetZCFrequency(void)
{
	return __ZeroCross._ZeroCrossFrequency;
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
static inline void _StartZeroCross(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__disable_irq();	// disable interrupts until end of initialisation

	Device->mj8x8->UpdateActivity(ZEROCROSS, ON);	// mark device as on
	HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);	// disable EXTI0 - we will use a timer2 IC  mode from now on...

	// configure from EXTI0 to timer2 input-capture mode (so that the device can measure ZC frequency)
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF2_TIM2;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);

	Device->StartTimer(&htim2);	// start zero-cross input capture timer

	__HAL_RCC_DMA1_CLK_ENABLE();	// start the DMA clock

	HAL_TIM_IC_Start_DMA(&htim2, TIM_CHANNEL_1, _zc_counter_buffer, 2);	// start timer2 DMA
	Device->StartTimer(&htim3);	// start measurement interval timer

	__enable_irq();  // enable interrupts
}

// stops the timer & DMA peripherals
static inline void _StopZeroCross(void)
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	__disable_irq();	// disable interrupts until end of initialisation

	Device->StopTimer(&htim3);	// stop measurement interval timer
	HAL_TIM_IC_Stop_DMA(&htim2, TIM_CHANNEL_1);	// stop timer2 DMA
	__HAL_RCC_DMA1_CLK_DISABLE();	// turn off peripheral
	Device->StopTimer(&htim2);	// stop zero-cross input capture timer

	// configure from timer2 IC to GPIO EXTI0 mode (so that the device can wake up on ZC detection)
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
	__ZeroCross.public.GetZCFrequency = &_GetZCFrequency;  // set function pointer
	__ZeroCross.public.Start = &_StartZeroCross;  // ditto
	__ZeroCross.public.Stop = &_StopZeroCross;  // ditto

	__DMAInit();	// initialise DMA

	HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);  // event handler timer (on demand)
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 - zero-cross detection of first dynamo impulse after standstill
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 0, 0);  // timer2 zero-cross signal frequency measurement via DMA
	HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

	return &__ZeroCross.public;  // return public parts
}

// DMA ISR - zero-cross frequency measurement - fires once every rising edge zero-cross
void DMA1_Channel4_5_IRQHandler(void)
{
	// TODO - GnZC - validate actual ZC pulse width over speed
	HAL_DMA_IRQHandler(&hdma_tim2_ch1);  // service the interrupt

  uint32_t zc_counter_delta_abs = (_zc_counter_buffer[1] > _zc_counter_buffer[0]) ?	// expression
                                     (_zc_counter_buffer[1] - _zc_counter_buffer[0]) :	// if expression true
                                     (_zc_counter_buffer[0] - _zc_counter_buffer[1]);	//	if expression false

  _zc_counter_delta += zc_counter_delta_abs;	// accumulate the absolute difference

	++_zcValues;	// how many times did we add?
}

// timer 3 ISR - 250ms interrupt - frequency measurement timer
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt

	_CalculateZCFrequency();  // calculate ZC signal frequency

	Device->AutoDrive->Do();	// let AutoDrive do its thing
}

#endif // MJ838_

#endif /* MJ838_ZEROCROSS_C_ */
