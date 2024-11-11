#ifndef MJ838_ZEROCROSS_C_
#define MJ838_ZEROCROSS_C_

#if defined(MJ838_)	// if this particular device is active

#include "zerocross/zerocross_actual.c"

extern TIM_HandleTypeDef htim2;  // Timer2 object - input capture of zero-cross signal on rising edge
extern TIM_HandleTypeDef htim3;  // Timer3 object - periodic frequency measurement of timer2 data - default 250ms
static DMA_HandleTypeDef hdma_tim2_ch1;	// zero-cross frequency measurement

static __zerocross_t __ZeroCross;  // forward declaration of object

uint32_t _zc_counter_buffer[2] =
	{0};	// stores timer2 counter readout

volatile uint32_t _zc_counter_delta = 0;	// container for average frequency calculation
volatile uint16_t _zcValues = 0;	// iterator for average frequency calculation
volatile uint16_t _sleep = 0;	// timer3-based count for sleep since last zero-cross detection
volatile uint32_t _tim2ICcounterOVF = 0;	// timer2 IC counter overflow counter
volatile float _previousFrequency = 0;	//

// timer3-triggered - computes Zero-Cross signal frequency, normally at 250ms intervals
static void _Do(void)
{
  if (_zc_counter_delta)	// if there is data...
  {
  		_sleep = 0;	// reset the sleep counter

  		// below is the 250ms-speed-average calculation for normal speeds (i.e. ZC period << 250ms)
#if SIGNAL_GENERATOR_INPUT
  	__ZeroCross._ZeroCrossFrequency = (800000000 / (_zc_counter_delta / _zcValues) ) / 100.0;	// "round" and average dynamo AC frequency
#else
  	__ZeroCross._ZeroCrossFrequency = 8000000.0 / (_zc_counter_delta / _zcValues);	// average dynamo AC frequency
#endif



  	// special handling for lower speeds (below normal walking speed)
  	if (__ZeroCross._ZeroCrossFrequency >= 5)	// 5Hz - speeds >= 0.75 mps / 2.69 kph
  	{
  			// adapt based on detected speed
    		__HAL_TIM_SET_AUTORELOAD(&htim3, 2499);	// 250ms
  	}
  	else
 		{
  			__HAL_TIM_SET_AUTORELOAD(&htim3, 4999);	// 500ms

  			// adapt based on detected speed
  	  	if (__ZeroCross._ZeroCrossFrequency < 2)	// 2 Hz - speeds < 0.3 mps / 1.07 kph
  	  		__HAL_TIM_SET_AUTORELOAD(&htim3, 9999);	// 1s

  			// adapt based on detected speed
  	  	if (__ZeroCross._ZeroCrossFrequency < 1) // 1 Hz - speeds < 0.15 mps / 0.53 kph
  	  		__HAL_TIM_SET_AUTORELOAD(&htim3, 19999);	// 2s



  	  	// adapt based on calculated acceleration
  	  	if (__ZeroCross._ZeroCrossFrequencyRate > 1 ) // 1Hz/s²
  	  		__HAL_TIM_SET_AUTORELOAD(&htim3, 9999);	// 1s

  	  	// adapt based on calculated acceleration
  	  	if (__ZeroCross._ZeroCrossFrequencyRate > 2 ) // 2Hz/s²
  	  		__HAL_TIM_SET_AUTORELOAD(&htim3, 4999);	// 500ms

  	  	// adapt based on calculated acceleration
  	  	if (__ZeroCross._ZeroCrossFrequencyRate > 4 )	// 4Hz/s²
  	  		__HAL_TIM_SET_AUTORELOAD(&htim3, 2499);	// 250ms
 		}
  }
  else	// no data - standstill
  {
  		__HAL_TIM_SET_AUTORELOAD(&htim3, 2499);	// 250ms -- set for normal operation

      __ZeroCross._ZeroCrossFrequency = 0;
      ++_sleep;
  }

  __ZeroCross._ZeroCrossFrequencyRate = (__ZeroCross._ZeroCrossFrequency - _previousFrequency) / ((__HAL_TIM_GET_AUTORELOAD(&htim3) / 10000) + 1 );

  _previousFrequency = __ZeroCross._ZeroCrossFrequency;	// save current frequency for next iteration

  if (_sleep > SLEEPTIMEOUT_COUNTER)	// n iterations of sleep
  {
      __ZeroCross._ZeroCrossFrequency = 0;	// zero & start over
      __ZeroCross._ZeroCrossFrequencyRate = 0;	// zero & start over
      _sleep = 0;
      Device->ZeroCross->Stop();	// stop zero-cross detection; timer1 will put the device into stop mode
      Device->AutoDrive->LightOff(); // tell AutoDrive that we are stopped
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

	Device->mj8x8->UpdateActivity(ZEROCROSS, ON);	// update the bus
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

//FIXME - something is not stopping the timer: upon entering this function on standstill, the timers 2 and 3 gets stopped but turned on again somehow
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

	Device->mj8x8->UpdateActivity(ZEROCROSS, OFF);	// update the bus

	__enable_irq();  // enable interrupts
}

zerocross_t* zerocross_ctor(void)
{
	__ZeroCross.public.GetZCFrequency = &_GetZCFrequency;  // set function pointer
	__ZeroCross.public.Do = &_Do;  // ditto
	__ZeroCross.public.Start = &_StartZeroCross;  // ditto
	__ZeroCross.public.Stop = &_StopZeroCross;  // ditto

	__DMAInit();	// initialise DMA

	HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);  // Set the timer interrupt priority
	HAL_NVIC_EnableIRQ(TIM2_IRQn);           // Enable the timer interrupt

	HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);  // frequency measurement timer for timer2 data (on demand)
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 0, 0);  // EXTI0 - zero-cross detection of first dynamo impulse after standstill
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel4_5_IRQn, 1, 0);  // timer2 zero-cross signal frequency measurement via DMA
	HAL_NVIC_EnableIRQ(DMA1_Channel4_5_IRQn);

	return &__ZeroCross.public;  // return public parts
}

// DMA ISR - zero-cross frequency measurement - fires once every rising edge zero-cross
void DMA1_Channel4_5_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_tim2_ch1);  // service the interrupt

  uint32_t zc_counter_delta_abs = (_zc_counter_buffer[1] > _zc_counter_buffer[0]) ?	// expression
                                     (_zc_counter_buffer[1] - _zc_counter_buffer[0]) :	// if expression true
                                     (_zc_counter_buffer[0] - _zc_counter_buffer[1]);	//	if expression false

  zc_counter_delta_abs += _tim2ICcounterOVF * TIMER2_PERIOD; // account for overflow condition
  _tim2ICcounterOVF = 0;

  _zc_counter_delta += zc_counter_delta_abs;	// accumulate the absolute difference
	++_zcValues;	// how many times did we add?
}

// counter overflow
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
  		++_tim2ICcounterOVF;	// Timer overflow occurred
  }
}

// timer 2 ISR - update general interrupt (interrupt itself not needed, but must be serviced anyway)
void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim2);  // service the interrupt
}

// timer 3 ISR - 250ms interrupt - frequency measurement timer
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt

	Device->ZeroCross->Do();	// calculate ZC signal frequency
	Device->AutoDrive->Do();	// let AutoDrive do its thing
	Device->AutoCharge->Do();	// let AutoCharge do its thing
}

#endif // MJ838_

#endif /* MJ838_ZEROCROSS_C_ */
