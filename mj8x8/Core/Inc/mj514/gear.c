#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "gear.h"	//
#include "mb85rc/mb85rc.h"	// 16kB FeRAM
#include "mj514/motor.h"	// Motor controller

static uint8_t _ShiftinginProgress;

typedef struct	// gear_t actual
{
	gear_t public;  // public struct
	mb85rc_t *FeRAM;	// 16kb FeRAM module
	motor_t *Motor;  // motor controller
} __gear_t;

static __gear_t __Gear __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

static uint8_t _GetGear(void)
{
	while(_ShiftinginProgress)
		;  // guard against function call while shifting is in progress

	return __Gear.FeRAM->Read(0);  // return saved gear from FeRAM
}

static void _ShiftGear(const int8_t n)
{
	if(n == 0)  // keep gear
		return;

	// the hub has 14 gears - 0 through 13:
	if(n < -13)  // one can't shift down more than 13 gears
		return;

	if(n > 13)  // one can't shift up more than 13 gears
		return;

	_ShiftinginProgress = 1;	// critical section start

	__Gear.FeRAM->Write(0, n);	// write gear into FeRAM

	if(n > 0)
		__Gear.Motor->Rotate(Up, n);  // shift up n gears

	if(n < 0)
		__Gear.Motor->Rotate(Down, n);  // shift down n gears

	_ShiftinginProgress = 0;  // critical section stop
}

static __gear_t __Gear =  // instantiate gear_t actual and set function pointers
	{  //
	.public.ShiftGear = &_ShiftGear,  // set function pointer
	.public.GetGear = &_GetGear  // set function pointer
	};

gear_t* gear_ctor(void)  //
{
	_ShiftinginProgress = 0;

	__Gear.FeRAM = mb85rc_ctor();
	__Gear.Motor = motor_ctor();

	return &__Gear.public;  // set pointer to Gear public part
}

#endif
