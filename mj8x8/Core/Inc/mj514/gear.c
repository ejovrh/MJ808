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

#define MEM_GEAR 0x00, 1	// address 0x00, size 1 byte
#define MEM_FOO 0x02, 4	// address 0x01, size 4 bytes
#define MEM_BAR 0x06, 2	// address 0x06, size 2 bytes

volatile uint8_t _curr;

// return current Rohloff gear
static inline uint8_t _GetGear(void)
{
	/* rohloff gears are numbered from 1 to 14,
	 * 	1 is the lowest, lightest - slower speed
	 * 	14 is the highest, hardest - faster speed
	 *
	 * 	a return value of "3" means that the e14 unit thinks the hub is in gear 3
	 */

	return __Gear.FeRAM->Read(MEM_GEAR);  // return saved gear from FeRAM
}

static inline void _SetGear(const int8_t n)
{
	__Gear.FeRAM->Write(n, MEM_GEAR);  // write gear into FeRAM
}

// shifts Rohloff by n (up or down)
static void _ShiftByN(const int8_t n)
{
	/* rohloff gears are numbered from 1 to 14,
	 * 	1 is the lowest, lightest - slower speed
	 * 	14 is the highest, hardest - faster speed
	 *
	 * 	argument "0" is keeping the current gear
	 * 	argument "-2" is shifting e.g. from gear 10 to 8, i.e. shifting down by 2 gears
	 * 	argument "3" is shifting e.g. from gear 2 to 5, i.e. shifting up by 3 gears
	 */

	if(n == 0)  // keep gear
		return;

	// the hub has 14 gears - 1 through 14:
	if(n < -13)  // shift down by more than 13 gears makes no sense
		return;

	if(n > 13)  // shift up by more than 13 gears makes no sense
		return;

	while(_ShiftinginProgress)
		;

	_ShiftinginProgress = 1;	// critical section start
	Device->mj8x8->UpdateActivity(SHIFTING, _ShiftinginProgress);  // update the bus

	_curr = _GetGear();

	if(n > 0)
		{
			if(_curr + n > 14)
				{
					_ShiftinginProgress = 0;
					return;
				}

			__Gear.Motor->Shift(ShiftUp, n);  // shift up n gears
		}

	if(n < 0)
		{
			if((int8_t) (_curr + n < 1))
				{
					_ShiftinginProgress = 0;
					return;
				}

			__Gear.Motor->Shift(ShiftDown, -n);  // shift down n gears
		}

	_SetGear(_curr + n);  // write gear into FeRAM

	_ShiftinginProgress = 0;  // critical section stop
	Device->mj8x8->UpdateActivity(SHIFTING, _ShiftinginProgress);  // update the bus
}

// shift Rohloff into gear n
static void _ShiftToN(const uint8_t n)
{
	/* rohloff gears are numbered from 1 to 14,
	 * 	1 is the lowest, lightest - slower speed
	 * 	14 is the highest, hardest - faster speed
	 *
	 * 	argument "0" is keeping the current gear
	 * 	argument "3" is shifting into gear 3
	 */

	if(n == 0)  // makes no sense
		return;

	if(n > 14)  // shift to a greater gear than 14 makes no sense
		return;  // additionally, e.g. -2 with uint8_t n becomes 0xfe - so negative numbers are also caught

	while(_ShiftinginProgress)
		;

	uint8_t current = _GetGear();

	if(n == current)
		return;

	_ShiftByN(n - current);

	_SetGear(n);	// write gear into FeRAM
}

static __gear_t __Gear =  // instantiate gear_t actual and set function pointers
	{  //
	.public.ShiftByN = &_ShiftByN, 	// set function pointer
	.public.ShiftToN = &_ShiftToN,  // set function pointer
	.public.GetGear = &_GetGear  // set function pointer
	};

gear_t* gear_ctor(void)  //
{
	_ShiftinginProgress = 0;

	__enable_irq();  // PARTLY!!! enable interrupts -- essential for I2C
	__Gear.FeRAM = mb85rc_ctor();  // tie in FeRAM object
	__Gear.Motor = motor_ctor();	// tie in motor controller object

//	_ShiftByN(-4);
//	_ShiftToN(2);

	__disable_irq();	// disable interrupts for the remainder of initialization - mj514.c mostly

	return &__Gear.public;  // set pointer to Gear public part
}

#endif
