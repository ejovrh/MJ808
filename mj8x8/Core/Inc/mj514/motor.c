#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "motor.h"
#include "as5601.h"
#include "mj514_adc.c"	// concrete device-specific ADC functions

#include <stdlib.h>

extern ADC_HandleTypeDef hadc;  // ADC object

extern TIM_HandleTypeDef htim2;  // motor control PWM signal generation
#define MOTOR_IN1_CCR htim2.Instance->CCR1	// Output Compare Register for motor PWM  channel 1
#define MOTOR_IN2_CCR htim2.Instance->CCR2	// Output Compare Register for motor PWM  channel 2

#if USE_RAMPUP
#define MOTOR_PWM_END (uint32_t) 0x0190 // CCR end value for each channel
#define MOTOR_PWM_START (uint32_t) 0x090 // CCR start value for each channel, below 0x0090 there is hardly enough oomph to rotate anything
#define MOTOR_PWM_RAMP_TIME 175 // PWM signal ramp-up period in timer16 ISR ticks

#if USE_STALL_DETECTION_RAMPUP
#define STALL_DETECTION_TIME 100	// x time in timer16 ISR ticks
#define STALL_ANGLE_THRESHOLD 20	// raw angle delta
#endif
#else
#define MOTOR_PWM_START (uint32_t) 0x0BF // CCR start value for each channel, below 0x0090 there is hardly enough oomph to rotate anything
#endif
#if USE_AS5601_PULSE
extern TIM_HandleTypeDef htim3;  // timer in encoder mode for rotation detection
#endif

#if CURRENT_STATS || ANGLE_STATS
#define STAT_BUFFER_LEN 512
#endif

extern TIM_HandleTypeDef htim16;  // motor time base - 1ms

/*	handy copy'n'paste GPIO SET/RESET for timing
 * 	HAL_GPIO_WritePin(Debug0_GPIO_Port, Debug0_Pin, GPIO_PIN_SET);
 * 	HAL_GPIO_WritePin(Debug0_GPIO_Port, Debug0_Pin, GPIO_PIN_RESET);
 * 	HAL_GPIO_WritePin(Debug1_GPIO_Port, Debug1_Pin, GPIO_PIN_SET);
 * 	HAL_GPIO_WritePin(Debug1_GPIO_Port, Debug1_Pin, GPIO_PIN_RESET);
 *
 */

/* shifting  times (measured via time difference between first and last I2C transaction):
 * 0xB0 - approx. 340ms
 * 0xC0 - approx. 278ms
 * 0xD0 - approx. 246ms
 *
 */

/* rationale: one gear rotation step (measured in angles or pulses) isn't truly an exact multiple of 432 or 108.
 * instead, it is a multiple of 108.1242... and 432.4968... - decimal numbers
 *
 * successive additions of a rounded rotation step will over n rotations introduce errors
 * 	with respect to successive additions of the exact decimal number.
 *
 * we have 14 gears, yet we will not shift more than 13 gears in either direction.
 *	for instance, if we are in gear 5 and shift one, we need to add 1 to the end-of-travel raw angle,
 *		if we shift 7 gears, we need to add 4 to the end-of-travel raw angle
 *		eventually, if we are in gear 1 and want to shift to 14, we need to add 7.
 */

#if USE_AS5601_PULSE
static const uint16_t _PulseCorrection[] = {0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};  // pre-computed correction values
#else
static const uint16_t _RawAngleCorrection[] =
	{1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7};  // ditto
#endif

volatile uint16_t _EndTravel = 0;  // stores end of rotation angle or pusle count
volatile uint16_t _EndTravelTime = 0;  // time in ms when EndTavel was reached
volatile uint16_t abs_DeltaAngle = 0;
volatile uint16_t abs_DeltaAngleEnd = 0;
volatile uint16_t _CurrentIterationAngle = 0;
volatile uint8_t _n;
volatile uint16_t _StartAngle = 0;  // stores the start angle, before rotation
volatile uint32_t _channel;  // set in _RotateE14MagneticCog(), used in HAL_TIM_PeriodElapsedCallback()
volatile uint32_t *_ccr;  // general CCR, depending on which directional channel we use
volatile uint16_t _ISRTicks = 0;  // ticks needed for a shifting operation

#if CURRENT_STATS
static volatile int16_t _Array_IpropOverTime[STAT_BUFFER_LEN] = {0};  // stores Iprop channel value over time
static volatile int16_t _Array_dIprop_dt[STAT_BUFFER_LEN] = {0};  // stores Iprop channel change over time
volatile int16_t _previousIprop = 0;
volatile uint16_t _maxIpropRate = 0;
#endif
#if ANGLE_STATS
static volatile uint16_t _Array_RawAngleOverTime[STAT_BUFFER_LEN] =
	{0};  // stores as5601 raw angle over time
static volatile uint16_t _Array_TravelAngleOverTime[STAT_BUFFER_LEN] =
	{0};  // stores overall raw angle over the gear travel over time
static volatile uint16_t _Array_RawAngleChangeOverTime[STAT_BUFFER_LEN] =
	{0};	// stores raw angel change over time
volatile int16_t _50ms_RawAngleDelta = 0;
volatile uint16_t _RawDegreesPerms = 0;
#endif

typedef struct	// motor_t actual
{
	motor_t public;  // public struct

	adc_t *adc;  // ADC on device
	as5601_t *encoder;  // rotary encoder
} __motor_t;

static __motor_t __Motor __attribute__ ((section (".data")));  // preallocate __Motor object in .data

#if !USE_AS5601_PULSE
//
static uint16_t __ComputeAbsStartToCurrentAngleDelta(const uint16_t CurrentIterationAngle)
{
	static volatile int16_t Delta = 0;

	Delta = (int16_t) (CurrentIterationAngle - _StartAngle);  // compute delta

	// handle rollover using conditional expressions for branch reduction
	Delta = (Delta < -2048) ? (Delta + 4096) :
	        (Delta > 2048) ? (Delta - 4096) : Delta;

	return (uint16_t) (Delta < 0 ? -Delta : Delta);
}
#endif

// starts the motor controller IC and ADC
static void _StartMotorController(void)
{
#if USE_12V0_SHUTDOWN_GPIO
	HAL_GPIO_WritePin(Motor_12V0_SHDN_GPIO_Port, Motor_12V0_SHDN_Pin, GPIO_PIN_SET);	// power on the 12v0 boost converter
	// TODO - check if 1ms is enough
#endif

	// some sort of priority collision is the case of this hack
	HAL_NVIC_SetPriority(TIM16_IRQn, 2, 0);  // motor time base - 1ms
	HAL_NVIC_EnableIRQ(TIM16_IRQn);

	MOTOR_IN1_CCR = MOTOR_OFF;
	MOTOR_IN2_CCR = MOTOR_OFF;
	Device->StartTimer(&htim2);  // start PWM timer
	HAL_TIM_PWM_Start(&htim2, _channel);  // start the rotation

	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_SET);	// put controller into wake state

	// delay about 12us for the motor controller to settle
	for(uint8_t i = 0; i < 10; i++)
		__NOP();

	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_2);
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_3);
	HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);  // motor fault pin
	HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);

	Device->StartTimer(&htim16);	// start timebase and encoder timer
	__Motor.adc->Start();  // start ADC
}

// stops the motor controller IC and ADC
static void _StopMotorController(void)
{
	MOTOR_IN1_CCR = MOTOR_OFF;
	MOTOR_IN2_CCR = MOTOR_OFF;
	HAL_TIM_PWM_Stop(&htim2, _channel);
	Device->StopTimer(&htim2);	// stop PWM timer

	__Motor.adc->Stop();  // stop ADC

	// some sort of priority collision is the case of this hack
	HAL_NVIC_SetPriority(TIM16_IRQn, 3, 0);  // motor time base - 1ms
	HAL_NVIC_EnableIRQ(TIM16_IRQn);
	//	Device->StopTimer(&htim16);  // stop timebase and encoder timer

	HAL_NVIC_DisableIRQ(EXTI2_3_IRQn);	//

	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// put controller into sleep state

	// delay about 12us for the motor controller to settle
	for(uint8_t i = 0; i < 10; i++)
		__NOP();

#if USE_12V0_SHUTDOWN_GPIO
	HAL_GPIO_WritePin(Motor_12V0_SHDN_GPIO_Port, Motor_12V0_SHDN_Pin, GPIO_PIN_RESET);	// power off the 12v0 boost converter
#endif
}

// stop  start interrupts
void _SetInterruptsTo(const uint8_t enable)
{
	if(enable)
		{
			// heartbeat timer
			HAL_NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 2, 0);
			HAL_NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);

			// CAN interrupts
			HAL_NVIC_SetPriority(CEC_CAN_IRQn, 3, 0);
			HAL_NVIC_EnableIRQ(CEC_CAN_IRQn);
		}
	else
		{
			HAL_NVIC_DisableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);  // heartbeat timer
			HAL_NVIC_DisableIRQ(CEC_CAN_IRQn);	// CAN interrupts
		}
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
	 *	motor rotation has its inertia:
	 *		when shifting one gear in slow speed, the "23-tooth magnetic gear" rotates about 1/8 turn
	 *		when shifting one gear in high speed, the "23-tooth magnetic gear" rotates about 1/4 + 1/8 turn
	 *		this means that considerable error is introduced
	 *
	 *
	 * 		AS5601 is mounted on e.g. board top, the sensing magnet is above the IC (mounted on the so-called "magnetic gear")
	 * 			- CW rotation will increase counters angle and timer3's CCR1/2
	 * 			- CCW rotation will do the opposite
	 */
#if USE_DEBUG_GPIO
	HAL_GPIO_WritePin(Debug0_GPIO_Port, Debug0_Pin, GPIO_PIN_SET);  // used to time gear shift
	HAL_GPIO_WritePin(Debug1_GPIO_Port, Debug1_Pin, GPIO_PIN_SET);	// used to time breaking action
#endif

	_StartAngle = __Motor.encoder->Read(RAW_ANGLE);  // read out raw angle so that we know where we are starting
	abs_DeltaAngle = 0;
	_channel = channel;  // store the channel for the ISR
	_ccr = ccr;  // publish the incoming CCR for the ISR to set
	_ISRTicks = 0;

#if USE_AS5601_PULSE
	__Motor.public.PulseCounter = 0;
#endif
#if CURRENT_STATS
	_maxIpropRate = 0;
	_previousIprop = 0;
#endif

	_SetInterruptsTo(OFF);
	_StartMotorController();
}

// rotate a gear to shift up/down
static void _Shift(const direction_t dir, const uint8_t n)
{
	__Motor.public.FlagShiftingDone = 0;	// fist, unset the flag

#if USE_AS5601_PULSE
	_EndTravel = (uint16_t) ((PULSE_PER_GEAR * n) + _PulseCorrection[_n]);  // pre-compute the total rotation pulses
#else
	_EndTravel = (uint16_t) (RAWANGLE_PER_GEAR * n);
#endif
	_n = n;
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
	_StartAngle = __Motor.encoder->Read(RAW_ANGLE);

	__Motor.public.FlagShiftingDone = 0;	// unset the flag
#if USE_AS5601_PULSE
	__Motor.public.PulseCounter = 0;
#endif

	return &__Motor.public;  // set pointer to Motor public part
}

// Motor Fault ISR
void EXTI2_3_IRQHandler(void)
{
	if(__HAL_GPIO_EXTI_GET_IT(Motor_FLT_Pin))  // interrupt source detection
		{
			HAL_GPIO_EXTI_IRQHandler(Motor_FLT_Pin);  // service the interrupt
		}
}

#if USE_AS5601_PULSE
// timer in encoder mode for rotation detection
void TIM3_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim3);  // service the interrupt
}
#endif

// motor time base - 1ms
void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt
}

#if USE_AS5601_PULSE
// count pulses on one rotary encoder channel
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim3)
		{
			// with ABN = 0x08, this will update 1024 times per 360 degree rotation;
			++__Motor.public.PulseCounter;  // one degree of rotation equals 1024 / 360 = 2.84... impulses
		}
}
#endif

// motor time base - 1ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim16)
		{
			++_ISRTicks;  // increment the ISR counter
			HAL_ADC_Start_IT(&hadc);  // trigger ADC conversion

			_CurrentIterationAngle = __Motor.encoder->Read(RAW_ANGLE);  // read current motor angle for this iteration
			abs_DeltaAngle = __ComputeAbsStartToCurrentAngleDelta(_CurrentIterationAngle);  // compute absolute value for later

#if USE_RAMPUP
			// PWM ramp-up
			if(_ISRTicks <= MOTOR_PWM_RAMP_TIME)  // ramp up PWM signal within MOTOR_PWM_RAMP_TIME
				{
					*_ccr = MOTOR_PWM_START + (_ISRTicks * (MOTOR_PWM_END - MOTOR_PWM_START ) / MOTOR_PWM_RAMP_TIME);

#if USE_STALL_DETECTION_RAMPUP
				// FIXME - when devices are asleep and a shifting command comes in, the 1st shift after wakeup is very short.
				if(_ISRTicks > STALL_DETECTION_TIME && abs(_StartAngle - _CurrentIterationAngle) < STALL_ANGLE_THRESHOLD)
					{
						// Motor is considered stalled
						_StopMotorController();
						_SetInterruptsTo(ON);
					}
#endif
				}
#else
			*_ccr = MOTOR_PWM_START;
#endif

#if MOTOR_COAST || MOTOR_BRAKE
			// start to slow down
			if(_ISRTicks > 80)	// FIXME - no rampup causes problems due to timing: if the motor is slow enouhg, it will never reach end travel because it got halted by breaking
				{
#if USE_DEBUG_GPIO
					HAL_GPIO_WritePin(Debug1_GPIO_Port, Debug1_Pin, GPIO_PIN_RESET);  // used to time breaking action
#endif

#if MOTOR_COAST
					// slow down by coasting
					MOTOR_IN1_CCR = MOTOR_OFF;
					MOTOR_IN2_CCR = MOTOR_OFF;
#endif

#if MOTOR_BRAKE
					// slow down by breaking
					MOTOR_IN1_CCR = 0xFFFF;
					MOTOR_IN2_CCR = 0xFFFF;
			#endif
				}
#endif

#if CURRENT_STATS || ANGLE_STATS
			uint16_t i = _ISRTicks;  // each array entry is one millisecond

			// prevents data buffer overruns
			if(i > STAT_BUFFER_LEN)
				i = STAT_BUFFER_LEN - 1;

#if CURRENT_STATS
			// Iprop & Iprop change rate calculation
			_Array_IpropOverTime[i] = __Motor.adc->GetChannel(Iprop);  // current Iprop value at time = i (each round millisecond)

			// dIprop / dt -- change rate of Iprop
			if(i > 0)
				_Array_dIprop_dt[i] = (int16_t) (_Array_IpropOverTime[i] - _Array_IpropOverTime[i - 1]);

			// record maximum Iprop value
			if(_Array_dIprop_dt[i] >= _maxIpropRate)
				_maxIpropRate = abs(_Array_dIprop_dt[i]);
#endif

#if ANGLE_STATS
			// raw angle
			_Array_RawAngleOverTime[i] = _CurrentIterationAngle;
			_Array_TravelAngleOverTime[i] = __ComputeAbsStartToCurrentAngleDelta(_CurrentIterationAngle);

			_50ms_RawAngleDelta = (int16_t) (abs((_Array_RawAngleOverTime[50] - _Array_RawAngleOverTime[1])));

			if(i > 0)
				// temporary
				_Array_RawAngleChangeOverTime[i] = (uint16_t) abs((_Array_RawAngleOverTime[i] - _Array_RawAngleOverTime[i - 1]));
#endif
#endif

#if USE_AS5601_PULSE
			if(__Motor.public.PulseCounter >= _EndTravel)  // travel limit is identified by pulse count; see motor.h
#else
			abs_DeltaAngle = __ComputeAbsStartToCurrentAngleDelta(_CurrentIterationAngle);  // compute absolute value for later

			if(abs_DeltaAngle >= _EndTravel)  // travel limit is identified by delta raw angle; see motor.h
#endif
				{
					// once end of travel is reached but shifting is not yet completed (i.e. flag is not yet set)
					if(__Motor.public.FlagShiftingDone == 0)
						{
							abs_DeltaAngleEnd = __ComputeAbsStartToCurrentAngleDelta(_CurrentIterationAngle);
							_EndTravelTime = _ISRTicks;  // save the time
						}

#if ANGLE_STATS
					_RawDegreesPerms = (uint16_t) ((abs_DeltaAngle * 1000) / _ISRTicks);  // compute rotational speed (divide by 1000 to get actual value)
#endif

					_StopMotorController();  // stop the motor controller
					_SetInterruptsTo(ON);

#if !ANGLE_STATS || !CURRENT_STATS
					Device->StopTimer(&htim16);
#endif

					__Motor.public.FlagShiftingDone = 1;  // set to true
#if USE_DEBUG_GPIO
					HAL_GPIO_WritePin(Debug0_GPIO_Port, Debug0_Pin, GPIO_PIN_RESET);  // used to time gear shift
#endif
				}

#if ANGLE_STATS || CURRENT_STATS
			if(_ISRTicks > STAT_BUFFER_LEN)  // eventually stop the timer
				{
					Device->StopTimer(&htim16);
				}
#endif
		}
}
#endif
