#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "gear.h"	//
#include "mb85rc\mb85rc.h"	// 16kB FeRAM
#include "mj514\motor.h"	// Motor controller

static uint8_t _ShiftinginProgress;

typedef struct	// gear_t actual
{
	gear_t public;  // public struct
	mb85rc_t *FeRAM;	// 16kb FeRAM module
	motor_t *Motor;  // motor controller
} __gear_t;

static __gear_t __Gear __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

// return current Rohloff gear
static uint8_t _GetGear(void)
{
	while(_ShiftinginProgress)
		;  // guard against function call while shifting is in progress

	return __Gear.FeRAM->Read(0x03, 1);  // FIXME - bogus address & size return saved gear from FeRAM
}

// shifts Rohloff by n (up or down)
static void _ShiftGear(const int8_t n)
{
	/*  Begriffserklärung:
	 *
	 * rohloff gears are numbered from 1 to 14,
	 * 	1 is the lowest, lightest - slower speed
	 * 	14 is the highest, hardest - faster speed
	 *
	 * 	argument 0 is keeping the current gear
	 * 	argument -2 is shifting e.g. from gear 10 to 8, i.e. shifting down by 2 gears
	 * 	argument 3 is shifting e.g. from gear 2 to 5, i.e. shifting up by 3 gears
	 *
	 * 	shifting is achieved by rotating a cog which sits on the hub (on some 8mm hex) in either CW or CCW direction:
	 * 		CCW shifts from gear 1 towards 14 - into heavier gear
	 * 			this is shifting up
	 * 		CW shifts from gear 14 towards 1 = into lighter gear
	 * 			this is shifting down
	 *
	 * 	shifting direction (direction_t ShiftUp or ShiftDown) is eventually a question of rotating some motor one or another direction.
	 * 		here in this file, ShiftUp and ShiftDown are defined as directions from gear 1 to 14 or vice versa.
	 *
	 * 		shifting direction (ShiftUp and ShiftDown) are relevant when actually riding (pedalling) the bike, otherwise it is not discernible.
	 * 		when debugging, e14 cog rotation is what _is_ visible, therefore it is central to rotational definitions:
	 * 			gear_t (this file): gears 1 to 14
	 * 			motor_t: rotate the so called magnetic cog CW or CCW
	 * 			as5601_t: detect the rotation of said magnetic cog
	 *
	 * 		the motor object translates e.g. ShiftUp into a "e14 cog" CW-rotation.
	 * 			the "e14 cog" is the one just visible cog in the e14 shifting unit.
	 * 			it is what drives the hub cog in either direction, similar to the external (cable actuated) gearbox.
	 *
	 * 			e14 cog CW rotation is hub cog CCW rotation, and vice versa
	 * 				this means that e.g. e14 cog CW rotation translates to hub cog CCW rotation and thereby is shifting up.
	 * 				...and vice versa
	 *
	 * 	see motor.h for direction_t and follow it into the motor C code until one reaches the final CW/CCW cog rotation as detected by the encoder
	 */

	if(n == 0)  // keep gear
		return;

	// the hub has 14 gears - 0 through 13:
	if(n < -13)  // shift down by more than 13 gears makes no sense
		return;

	if(n > 13)  // shift up by more than 13 gears makes no sense
		return;

	_ShiftinginProgress = 1;	// critical section start
	Device->mj8x8->UpdateActivity(SHIFTING, _ShiftinginProgress);  // update the bus

// FIXME	__Gear.FeRAM->Write(0, (const uint8_t*) &n);  // write gear into FeRAM

	if(n > 0)
		__Gear.Motor->Rotate(ShiftUp, n);  // shift up n gears

	if(n < 0)
		__Gear.Motor->Rotate(ShiftDown, n);  // shift down n gears

	_ShiftinginProgress = 0;  // critical section stop
	Device->mj8x8->UpdateActivity(SHIFTING, _ShiftinginProgress);  // update the bus
}

static __gear_t __Gear =  // instantiate gear_t actual and set function pointers
	{  //
	.public.ShiftGear = &_ShiftGear,  // set function pointer
	.public.GetGear = &_GetGear  // set function pointer
	};

gear_t* gear_ctor(void)  //
{
	_ShiftinginProgress = 0;

	__enable_irq();  // PARTLY!!! enable interrupts -- essential for I2C
	__Gear.FeRAM = mb85rc_ctor();  // tie in FeRAM object
	__Gear.Motor = motor_ctor();	// tie in motor controller object

	__disable_irq();	// disable interrupts for the remainder of initialization - mj514.c mostly
	return &__Gear.public;  // set pointer to Gear public part
}

#endif
