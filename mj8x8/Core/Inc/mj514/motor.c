#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "motor.h"
#include "as5601.h"
#include "mj514_adc.c"	// concrete device-specific ADC functions

extern TIM_HandleTypeDef htim2;  // motor control PWM signal generation
#define MOTOR_IN1_CCR htim2.Instance->CCR1	// Output Compare Register for motor PWM  channel 1
#define MOTOR_IN2_CCR htim2.Instance->CCR2	// Output Compare Register for motor PWM  channel 2

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
	__Motor.encoder->Start();  // start the encoder timebase
	__Motor.adc->Start();  // start ADC and its timebase
	Device->StartTimer(&htim2);  // start PWM timer

	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_SET);	// put controller into wake state
}

// stops the motor controller IC and ADC
static void _StopMotorController(void)
{
	Device->StopTimer(&htim2);	// stop PWM timer
	__Motor.adc->Stop();  // start ADC and its timebase
	__Motor.encoder->Stop();	// stop the encoder timebase

	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// put controller into sleep state
}

// shift up - rotate the "23-tooth magnetic gear" clockwise
static void _RotateE14MagneticCogCW(const uint8_t n)
{
	/* definition of rotation direction
	 * e14 shifting unit backpanel disassembled, viewed from e14 unit right side:
	 * 	(viewing the e14 unit from the back)
	 * 	rotation of the "23-tooth magnetic gear": rotation is counter-clockwise when this function is called
	 * 	the as5601 encoder sees this as the same rotation direction:
	 * 		"23-tooth magnetic gear" CW is as5601 CW
	 */

	uint32_t curr = __Motor.encoder->PulseCounter;

	_StartMotorController();	// wake up

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	MOTOR_IN2_CCR = 0x0090;

	while(__Motor.encoder->PulseCounter - curr < (108 * n))
		;

	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);

	_StopMotorController();  // sleep
}

// shift down - rotate the "23-tooth magnetic gear" counter-clockwise
static void _RotateE14MagneticCogCCW(const uint8_t n)
{
	/* definition of rotation direction
	 * e14 shifting unit backpanel disassembled, viewed from e14 unit right side:
	 * 	(viewing the e14 unit from the back)
	 * 	rotation of the "23-tooth magnetic gear": rotation is counter-clockwise when this function is called
	 * 	the as5601 encoder sees this as the same rotation direction:
	 * 		"23-tooth magnetic gear" CCW is as5601 CCW
	 */

	uint32_t curr = __Motor.encoder->PulseCounter;

	_StartMotorController();	// wake up

	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	MOTOR_IN1_CCR = 0x0090;

	while(__Motor.encoder->PulseCounter - curr < (108 * n))
		;

	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);

	_StopMotorController();  // sleep
}

// rotate a gear to shift up/down
static void _Shift(const direction_t dir, const uint8_t n)
{
	if(dir == ShiftUp)  // shift up: from gear 1 towards 14
		{
			_RotateE14MagneticCogCW(n);  // rotate accordingly
			return;
		}

	if(dir == ShiftDown)  // shift down: from gear 14 towards 1
		{
			_RotateE14MagneticCogCCW(n);  // rotate accordingly
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

	__Motor.public.PulseCount = &__Motor.encoder->PulseCounter;  // tie in rotary encoder pulse count

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
#endif
