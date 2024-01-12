#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "motor.h"
#include "as5601.h"
#include "mj514/mj514_adc.c"	// concrete device-specific ADC functions

extern TIM_HandleTypeDef htim2;  // motor control PWM signal generation

typedef struct	// motor_t actual
{
	motor_t public;  // public struct

	adc_t *adc;  // ADC on device
	as5601_t *as5601;  // rotary encoder
} __motor_t;

static __motor_t __Motor __attribute__ ((section (".data")));  // preallocate __Motor object in .data

// starts the motor controller IC and ADC
static void _StartMotorController(void)
{
	__Motor.adc->Start();  // start ADC and its timebase
	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_SET);	// put controller into wake state
}

// stops the motor controller IC and ADC
static void _StopMotorController(void)
{
	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// put controller into sleep state
	__Motor.adc->Stop();  // start ADC and its timebase
}

//
static void _RotateUp(void)
{
	_StartMotorController();	// wake up

	while(__Motor.as5601->Read(ANGLE) != 360)  // bogus command
		{
			HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1);
			HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
		}

	_StopMotorController();  // sleep
}

//
static void _RotateDown(void)
{
	_StartMotorController();	// wake up

	HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_2);
	HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

	_StopMotorController();  // sleep

}

//
static uint8_t _Rotate(const direction_t dir, const uint8_t n)
{
	if(dir == Up)  // shift up
		_RotateUp();

	if(dir == Down)  // shift down
		_RotateDown();

	return __Motor.as5601->CountRotation();
}

static __motor_t __Motor =  // instantiate motorc_t actual and set function pointers
	{  //
	.public.Rotate = &_Rotate,  // set function pointer
	};

motor_t* motor_ctor(void)  //
{
	HAL_GPIO_WritePin(Motor_SLP_GPIO_Port, Motor_SLP_Pin, GPIO_PIN_RESET);	// start with sleep state

	__Motor.adc = adc_ctor();  // call ADC constructor
	__Motor.as5601 = as5601_ctor();  // call rotary encoder constructor

	return &__Motor.public;  // set pointer to Motor public part
}

#endif
