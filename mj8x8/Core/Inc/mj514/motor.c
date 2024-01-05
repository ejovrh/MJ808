#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "motor.h"

#include "as5601.h"

typedef struct	// motorc_t actual
{
	motor_t public;  // public struct

	as5601_t *as5601;  //
} __motor_t;

static __motor_t __Motor __attribute__ ((section (".data")));  // preallocate __Motor object in .data

static void _RotateUp(void)
{
	while(__Motor.as5601->Read(ANGLE) != 360)  // bogus command
		HAL_GPIO_WritePin(Motor_Direction_Up_GPIO_Port, Motor_Direction_Up_Pin, GPIO_PIN_SET);
}

static void _RotateDown(void)
{
	HAL_GPIO_WritePin(Motor_Direction_Down_GPIO_Port, Motor_Direction_Down_Pin, GPIO_PIN_SET);  // bogus command
	;
}

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
	__Motor.as5601 = as5601_ctor();

	return &__Motor.public;  // set pointer to Motor public part
}

#endif
