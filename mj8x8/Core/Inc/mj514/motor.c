#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "motor.h"
#include "as5601.h"
#include "mj514_adc.c"	// concrete device-specific ADC functions

extern TIM_HandleTypeDef htim2;  // motor control PWM signal generation
#define CH1_CCR htim2.Instance->CCR1	// Output Compare Register for motor PWM  channel 1
#define CH2_CCR htim2.Instance->CCR2	// Output Compare Register for motor PWM  channel 2

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
	// FIXME - motor fault interrupt is triggered constantly
//	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_SET);	// put controller into wake state
}

// stops the motor controller IC and ADC
static void _StopMotorController(void)
{
//	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// put controller into sleep state
	Device->StopTimer(&htim2);	// stop PWM timer
	__Motor.adc->Stop();  // start ADC and its timebase
	__Motor.encoder->Stop();	// stop the encoder timebase
}

//
static void _RotateUp(void)
{
	// FIXME - define up and down in relation to rotary encoder direction
	_StartMotorController();	// wake up

//	while(__Motor.encoder->Read(ANGLE) != 360)  // bogus command
//		{
	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
	CH2_CCR = 0x0050;
//		}

	_StopMotorController();  // sleep
}

//
static void _RotateDown(void)
{
	// FIXME - define up and down in relation to rotary encoder direction
	_StartMotorController();	// wake up

	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	CH1_CCR = 0x0050;
	_StopMotorController();  // sleep

}

//
static uint8_t _Rotate(const direction_t dir, const uint8_t n)
{
	if(dir == Up)  // shift up
		_RotateUp();

	if(dir == Down)  // shift down
		_RotateDown();

	return __Motor.encoder->CountRotation();
}

static __motor_t __Motor =  // instantiate motorc_t actual and set function pointers
	{  //
	.public.Rotate = &_Rotate,  // set function pointer
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
			HAL_GPIO_TogglePin(Debug_GPIO_Port, Debug_Pin);  // toggling of debug pin
		}
}
#endif
