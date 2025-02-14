#ifndef MJ838_ZEROCROSS_C_
#define MJ838_ZEROCROSS_C_

#if defined(MJ838_)	// if this particular device is active

#include "zerocross/zerocross_actual.c"

extern TIM_HandleTypeDef htim2;  // Timer2 object - periodic frequency measurement of timer2 data - default 250ms
extern TIM_HandleTypeDef htim3;  // Timer3 object - input capture of zero-cross signal on rising edge
extern TIM_HandleTypeDef htim16;  // Timer16 object - odometer & co. 1s

static DMA_HandleTypeDef hdma_tim3_ch3;  // zero-cross frequency measurement

static __zerocross_t  __ZeroCross;  // forward declaration of object
GPIO_InitTypeDef GPIO_InitStruct =
	{0};

uint32_t _zc_counter_buffer[2] =
	{0};  // stores timer3 counter readout
uint32_t _zc_counter_delta = 0;  // container for average frequency calculation
uint16_t _zcValues = 0;  // iterator for average frequency calculation
uint8_t _sleep = 0;  // timer2-based count for sleep since last zero-cross detection
float _previousFrequency = 0;  // previous frequency value for change rate calculation

const uint16_t _ScaledCPUTick = SCALED_CPU_TICK;

#if USE_PAC1952
uint8_t Vbus[8];
uint8_t Vsense[8];
uint8_t Vpower[8];
#endif

// timer2-triggered - computes Zero-Cross signal frequency, normally at 250ms intervals
static void _Do(void)
{
	const float LOW_SPEED_THRESHOLD = 5.0f;
	const float VERY_LOW_SPEED_THRESHOLD = 2.0f;
	const float EXTREMELY_LOW_SPEED_THRESHOLD = 1.0f;
	const float ACCELERATION_THRESHOLD_1 = 1.0f;
	const float ACCELERATION_THRESHOLD_2 = 2.0f;
	const float ACCELERATION_THRESHOLD_3 = 4.0f;

	uint16_t _arr = 2499;  // default value for ARR - 250ms - good for all speeds faster than crawling (including transition to standstill)

	if(_zc_counter_delta)  // if there is data
		{
			_sleep = 0;  // reset the sleep counter

			// 250ms-speed-average calculation for normal speeds (i.e. ZC period << 250ms)
			__ZeroCross._ZeroCrossFrequency = ((float) (_ScaledCPUTick * _zcValues) / (float) _zc_counter_delta);  // average dynamo AC frequency

			// special handling for lower speeds (below normal walking speed)
			if(__ZeroCross._ZeroCrossFrequency < LOW_SPEED_THRESHOLD)  // 5Hz - speeds < 0.75 mps / 2.69 kph
				{
					_arr = 4999;  // 500ms

					// adapt based on detected speed
					if(__ZeroCross._ZeroCrossFrequency < VERY_LOW_SPEED_THRESHOLD)  // 2 Hz - speeds < 0.3 mps / 1.07 kph
						_arr = 9999;  // 1s

					// adapt based on detected speed
					if(__ZeroCross._ZeroCrossFrequency < EXTREMELY_LOW_SPEED_THRESHOLD)  // 1 Hz - speeds < 0.15 mps / 0.53 kph
						_arr = 19999;  // 2s

					// adapt based on calculated acceleration
					if(__ZeroCross._ZeroCrossFrequencyRate > ACCELERATION_THRESHOLD_1)  // 1Hz/s²
						_arr = 9999;  // 1s

					// adapt based on calculated acceleration
					if(__ZeroCross._ZeroCrossFrequencyRate > ACCELERATION_THRESHOLD_2)  // 2Hz/s²
						_arr = 4999;  // 500ms

					// adapt based on calculated acceleration
					if(__ZeroCross._ZeroCrossFrequencyRate > ACCELERATION_THRESHOLD_3)  // 4Hz/s²
						_arr = 2499;  // 250ms
				}
		}
	else  // no data - standstill
		{
			__ZeroCross._ZeroCrossFrequency = 0;
			++_sleep;
		}

	__HAL_TIM_SET_AUTORELOAD(&htim2, _arr);  // set determined ARR value

	// FIXME - check in what unit of time the change rate is computed
	__ZeroCross._ZeroCrossFrequencyRate = ((float) (__ZeroCross._ZeroCrossFrequency - _previousFrequency) / (float) (((__HAL_TIM_GET_AUTORELOAD(&htim2)) / 10000) + 1));

	_previousFrequency = __ZeroCross._ZeroCrossFrequency;  // save current frequency for next iteration

	if(_sleep > SLEEPTIMEOUT_COUNTER)  // n iterations of sleep
		{
			__ZeroCross._ZeroCrossFrequency = 0;  // zero & start over
			__ZeroCross._ZeroCrossFrequencyRate = 0;  // zero & start over
			_sleep = 0;
			Device->ZeroCross->Stop();  // stop zero-cross detection; timer1 will put the device into stop mode
			Device->AutoDrive->LightOff();  // tell AutoDrive that we are stopped
		}

	_zc_counter_delta = 0;  // zero & start over
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
	hdma_tim3_ch3.Instance = DMA1_Channel2;  // DMA1 channel 2 - TIM3_CH3
	hdma_tim3_ch3.Init.Direction = DMA_PERIPH_TO_MEMORY;  // from peripheral to memory
	hdma_tim3_ch3.Init.PeriphInc = DMA_PINC_DISABLE;  // do not increment peripheral address
	hdma_tim3_ch3.Init.MemInc = DMA_MINC_ENABLE;  // increment memory address
	hdma_tim3_ch3.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;  // 32-bit data
	hdma_tim3_ch3.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;  // 32-bit data
	hdma_tim3_ch3.Init.Mode = DMA_CIRCULAR;  // circular mode
	hdma_tim3_ch3.Init.Priority = DMA_PRIORITY_LOW;  //	normal priority
	HAL_DMA_Init(&hdma_tim3_ch3);

	__HAL_DMA_DISABLE_IT(&hdma_tim3_ch3, DMA_IT_HT | DMA_IT_TE);	// disable error & half-fransfer interrupts
	__HAL_LINKDMA(&htim3, hdma[TIM_DMA_ID_CC3], hdma_tim3_ch3);

	__HAL_RCC_DMA1_CLK_DISABLE();
}

// configure GPIO pin for zero-cross detection - timer3 input capture mode
static inline void _ConfigureZeroCrossPinforZC(void)
{
	HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);	// disable EXTI0 - we will use a timer2 IC  mode from now on...
	NVIC_ClearPendingIRQ(EXTI0_1_IRQn);  // clear pending interrupt

	// configure from EXTI0 to timer2 input-capture mode (so that the device can measure ZC frequency)
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);
}

// configure GPIO pin for EXTI0-based wakeup on first pulse
static inline void _ConfigureZeroCrossPinforEXTI(void)
{
	// configure from timer3 IC to GPIO EXTI0 mode (so that the device can wake up on 1st ZC pulse just after start of movement)
	GPIO_InitStruct.Pin = ZeroCross_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // catch zero cross activity (transition from idle to first pulse)
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(ZeroCross_GPIO_Port, &GPIO_InitStruct);

	NVIC_ClearPendingIRQ(EXTI0_1_IRQn);  // clear pending interrupt
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);  // enable EXTI0 for wakeup from stop mode
}

// starts the timer & DMA peripherals
static inline void _StartZeroCross(void)
{
	Device->AutoDrive->UpdateOdometer();  // update odometer

#if USE_PAC1952
	Device->PowerMonitor->PowerOn();  // power on the power monitor
#endif

	__disable_irq();	// disable interrupts until end of initialisation

	_ConfigureZeroCrossPinforZC();  // configure GPIO pin for zero-cross detection

	Device->StartTimer(&htim3);  // start zero-cross input capture timer

	__HAL_RCC_DMA1_CLK_ENABLE();	// start the DMA clock

	HAL_TIM_IC_Start_DMA(&htim3, TIM_CHANNEL_3, _zc_counter_buffer, 2);  // start timer2 DMA
	Device->StartTimer(&htim2);  // start measurement interval timer & odometer & co. timer
	Device->StartTimer(&htim16);  // start odometer & co. timer

	__enable_irq();  // enable interrupts

	Device->mj8x8->UpdateActivity(ZEROCROSS, ON);  // update the bus
}

// stops the timer & DMA peripherals
static inline void _StopZeroCross(void)
{
#if USE_PAC1952
	Device->PowerMonitor->PowerOff();  // power off the power monitor
#endif

	Device->AutoDrive->UpdateOdometer();  // update odometer

	__disable_irq();	// disable interrupts until end of initialisation

	Device->StopTimer(&htim2);	// stop measurement interval timer & odometer & co. timer
	Device->StopTimer(&htim16);  // stop odometer & co. timer

	HAL_TIM_IC_Stop_DMA(&htim3, TIM_CHANNEL_3);  // stop timer2 DMA
	__HAL_RCC_DMA1_CLK_DISABLE();  // turn off peripheral
	Device->StopTimer(&htim3);	// stop zero-cross input capture timer

	_ConfigureZeroCrossPinforEXTI();  // configure GPIO pin for ZeroCross wakeup on first impulse

	// turn off LEDs - in case they were on
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);

	// FIXME - with a signal generator input, on signal stop (emulated wheel rotation stop), zerocross stop does get executed correctly, however for some reason EXTI0 gets triggered somehow.
	__enable_irq();  // enable interrupts

	Device->mj8x8->UpdateActivity(ZEROCROSS, OFF);	// update the bus
}

zerocross_t* zerocross_ctor(void)
{
	__ZeroCross.public.GetZCFrequency = &_GetZCFrequency;  // set function pointer
	__ZeroCross.public.Do = &_Do;  // ditto
	__ZeroCross.public.Start = &_StartZeroCross;  // ditto
	__ZeroCross.public.Stop = &_StopZeroCross;  // ditto

	__DMAInit();	// initialise DMA

	HAL_NVIC_SetPriority(TIM2_IRQn, 3, 0);  // frequency measurement timer for timer2 data (on demand)
	HAL_NVIC_EnableIRQ(TIM2_IRQn);

	HAL_NVIC_SetPriority(EXTI0_1_IRQn, 1, 0);  // EXTI0 - zero-cross detection of first dynamo impulse after standstill
	HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

	HAL_NVIC_SetPriority(DMA1_Channel2_3_IRQn, 1, 0);  // timer3 zero-cross signal frequency measurement via DMA
	HAL_NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);

	return &__ZeroCross.public;  // return public parts
}

// DMA ISR - zero-cross frequency measurement - fires once every falling edge zero-cross
void DMA1_Channel2_3_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&hdma_tim3_ch3);  // service the interrupt

	uint16_t delta = (uint16_t) abs((int16_t) (_zc_counter_buffer[1] - _zc_counter_buffer[0]));  // calculate the difference

	if(delta)  // it can be zero...
		{
			_zc_counter_delta += delta;  // accumulate the difference
			++_zcValues;	// count additions for average calculation
		}
}

// IC callback - will execute at 1/8th of the ZeroCross frequency
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM3)
		{
			if(Device->AutoCharge->IsLoadConnected())
				{  // blink green
					HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
					HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
				}
			else
				{  // blink red
					HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_RESET);
					HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
				}
		}
}

// timer 2 ISR - periodic frequency measurement of timer2 data - default 250ms
void TIM2_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim2);  // service the interrupt

	Device->ZeroCross->Do();	// calculate ZC signal frequency
	Device->AutoDrive->Do();	// let AutoDrive do its thing
	Device->AutoCharge->Do();  // let AutoCharge do its thing

#if USE_PAC1952
	Device->PowerMonitor->RefreshV(); // send refresh_v command
	Device->PowerMonitor->BlockRead(0x07, Vbus, 8);	// read Vbus
	Device->PowerMonitor->BlockRead(0x0B, Vsense, 8);	// read Vsense
	Device->PowerMonitor->BlockRead(0x17, Vpower, 8);	// read Vpower
#endif
}

#endif // MJ838_

#endif /* MJ838_ZEROCROSS_C_ */
