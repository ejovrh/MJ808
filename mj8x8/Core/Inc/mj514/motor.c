#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "motor.h"
#include "as5601.h"
#include "mj514_adc.c"	// concrete device-specific ADC functions

#include <stdlib.h>

extern TIM_HandleTypeDef htim2;  // motor control PWM signal generation
#define MOTOR_IN1_CCR htim2.Instance->CCR1	// Output Compare Register for motor PWM  channel 1
#define MOTOR_IN2_CCR htim2.Instance->CCR2	// Output Compare Register for motor PWM  channel 2
#define MOTOR_PWM_START (uint32_t) 0x0090 // CCR start value for each channel
#define MOTOR_PWM_END (uint32_t) 0x0100 // CCR end value for each channel
#define MOTOR_PWM_RAMP_TIME 125 // PWM signal ramp-up period in timer16 ISR ticks

extern TIM_HandleTypeDef htim3;  // timer in encoder mode for rotation detection
extern TIM_HandleTypeDef htim16;  // motor time base - 0.5ms

/* shifting  times (measured via time difference between first and last I2C transaction):
 * 0xB0 - approx. 340ms
 * 0xC0 - approx. 278ms
 * 0xD0 - approx. 246ms
 *
 */

static volatile uint16_t _CurrentAngle = 0;
static volatile uint16_t _StartAngle = 0;  // stores the start angle, before rotation
static volatile int16_t _DeltaAngle = 0;  // stores the angle delta
static volatile uint16_t _EndAngle = 0;  // stores rotation end angle
static volatile uint32_t _channel;  // set in _RotateE14MagneticCog(), used in HAL_TIM_PeriodElapsedCallback()
static volatile uint32_t *_ccr;  // general CCR, depending on which directional channel we use
static volatile uint16_t _ISRTicks = 0;  // ticks needed for a shifting operation
static uint16_t _RawDegreesPerms = 0;

typedef struct	// motor_t actual
{
	motor_t public;  // public struct

	adc_t *adc;  // ADC on device
	as5601_t *encoder;  // rotary encoder
} __motor_t;

static __motor_t __Motor __attribute__ ((section (".data")));  // preallocate __Motor object in .data

// starts the motor controller IC and ADC
static void _StartMotorController(void)
{
	Device->StartTimer(&htim16);	// start timebase and encoder timer
	__Motor.adc->Start();  // start ADC
	Device->StartTimer(&htim2);  // start PWM timer

	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_SET);	// put controller into wake state
}

// stops the motor controller IC and ADC
static void _StopMotorController(void)
{
	Device->StopTimer(&htim2);	// stop PWM timer
	__Motor.adc->Stop();  // stop ADC
	Device->StopTimer(&htim16);  // stop timebase and encoder timer

	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// put controller into sleep state
}

// shift up/down - rotate the "23-tooth magnetic gear" CCW/CW
static void _RotateE14MagneticCog(const uint32_t channel, volatile uint32_t *ccr)
{
	/* !!! definition of rotation direction !!!
	 *	(it is reversed to what is described in gear.h)
	 *
	 * e14 shifting unit backpanel disassembled, viewed from e14 unit RIGHT side:
	 * 	(viewing the e14 unit from the back - from the bike's RIGHT side)
	 *
	 * 	this definition applies to CW/CCW terms in this file only.
	 *
	 * 		"23-tooth magnetic gear" CCW is as5601 CCW
	 * 		"23-tooth magnetic gear" CW is as5601 CW
	 *
	 * 		AS5601 is mounted on e.g. board top, the sensing magnet is above the IC (mounted on the so-called "magnetic gear")
	 * 			- clockwise rotation will increase counters angle and timer3's CCR1/2
	 * 			- counterclockwise rotation will do the opposite
	 */

	_StartAngle = __Motor.encoder->Read(RAW_ANGLE);  // read out raw angle so that we know where we are starting
	_channel = channel;  // store the channel for the ISR
	_ccr = ccr;  // publish the incoming CCR for the ISR to set
	_DeltaAngle = 0;
	_ISRTicks = 0;

	_StartMotorController();
	HAL_TIM_PWM_Start(&htim2, channel);  // start the rotation
}

// rotate a gear to shift up/down
static void _Shift(const direction_t dir, const uint8_t n)
{
	_EndAngle = RAWANGLE_PER_GEAR * n;  // pre-compute the total rotation angle

	if(dir == ShiftUp)  // shift up: from gear 1 towards 14
		{
			_RotateE14MagneticCog(TIM_CHANNEL_2, &MOTOR_IN2_CCR);  // rotate "23-tooth magnetic gear" CW
			return;
		}

	if(dir == ShiftDown)  // shift down: from gear 14 towards 1
		{
			_RotateE14MagneticCog(TIM_CHANNEL_1, &MOTOR_IN1_CCR);  // rotate "23-tooth magnetic gear" CCW
			return;
		}
}

static __motor_t __Motor =  // instantiate motorc_t actual and set function pointers
	{  //
	.public.Shift = &_Shift,  // set function pointer
	};

motor_t* motor_ctor(void)  //
{
	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// start with sleep state

	__Motor.adc = adc_ctor();  // call ADC constructor
	__Motor.encoder = as5601_ctor();  // tie in rotary encoder object

	return &__Motor.public;  // set pointer to Motor public part
}

// Motor Fault ISR
void EXTI2_3_IRQHandler(void)
{
	if(__HAL_GPIO_EXTI_GET_IT(Motor_FLT_Pin))  // interrupt source detection
		{
// FIXME - motor fault is being triggered constantly
			HAL_GPIO_TogglePin(Debug_GPIO_Port, Debug_Pin);  // toggling of debug pin
		}
}

// timer in encoder mode for rotation detection
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt
}

// motor time base - 0.5ms
void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt
}

// motor time base - 0.5ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim16)
		{
			HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, 1);  // used to time the ISR

			//			HAL_ADC_Start_IT(&hadc);  // Trigger ADC conversion
			_CurrentAngle = __Motor.encoder->Read(RAW_ANGLE);  // read out the raw angle at the very first

			// PWM ramp-up
			if(_ISRTicks < MOTOR_PWM_RAMP_TIME)  // ramp up PWM signal within MOTOR_PWM_RAMP_TIME
				*_ccr = MOTOR_PWM_START + (_ISRTicks * (MOTOR_PWM_END - MOTOR_PWM_START ) / MOTOR_PWM_RAMP_TIME);

			// travel limit is identified by delta raw angle; see motor.h
			_DeltaAngle = (int16_t) (_CurrentAngle - _StartAngle);  // get the angle delta

			// handle rollover
			if(_DeltaAngle < -2048)
				_DeltaAngle += 4096;

			if(_DeltaAngle > 2048)
				_DeltaAngle -= 4096;

			volatile uint16_t abs_DeltaAngle = (uint16_t) abs(_DeltaAngle);  // compute absolute value for later
			if(abs_DeltaAngle >= _EndAngle)
				{
					_ISRTicks /= 2;  // convert to milliseconds
					_RawDegreesPerms = (uint16_t) ((abs_DeltaAngle * 1000) / _ISRTicks);  // compute rotational speed (divide by 1000 to get actual value)

					HAL_TIM_PWM_Stop(&htim2, _channel);  // stop PWM generation
					_StopMotorController();  // stop the motor controller
				}

			++_ISRTicks;  // increment the ISR counter

			HAL_GPIO_WritePin(Debug_GPIO_Port, Debug_Pin, 0);  // used to time the ISR
		}
}
#endif
