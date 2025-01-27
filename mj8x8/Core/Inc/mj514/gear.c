#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "gear.h"	// abstraction of the Rohloff E14 servo motor
#include "mb85rc\mb85rc.h"	// 16kB FeRAM
#include "mj514\motor.h"	// Motor controller

typedef struct	// gear_t actual
{
	gear_t public;  // public struct

	volatile uint8_t _CurrentGear;	// holds the current gear we think we are in...
	volatile uint8_t _FlagShiftinginProgress :1;  // low-key semaphore for shifting in progress
	mb85rc_t *FeRAM;	// 16kb FeRAM module
	motor_t *Motor;  // motor controller
} __gear_t;

static __gear_t __Gear __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

#define MEM_GEAR 0x00, 1	// address 0x00, size 1 byte: current gear, 1 through 14
#define MEM_ACC_PULSES 0x02, 4	// address 0x01, size 4 bytes: accumulated pulses (1024 pulses per one per rotation of the "23-tooth magnetic gear")
#define MEM_ACC_SHIFTS 0x06, 4	// address 0x06, size 4 bytes: cumulative count of how many gears were shifted

#define ABS(x) ((x) < 0 ? -(x) : (x))

// read stored current Rohloff gear (1 to 14) from FeRAM
static inline uint8_t _GetGear(void)
{
	/* rohloff gears are numbered from 1 to 14,
	 * 	1 is the lowest, lightest - slower speed
	 * 	14 is the highest, hardest - faster speed
	 *
	 * 	a return value of "3" means that the e14 unit thinks the hub is in gear 3
	 */

	return __Gear.FeRAM->Read(MEM_GEAR);  // return saved data from FeRAM
}

// reads stored pulse count from FeRAM
static inline uint32_t _GetCumulativePulses(void)
{
	return __Gear.FeRAM->Read(MEM_ACC_PULSES);  // return saved data from FeRAM
}

// reads stored shift count from FeRAM
static inline uint32_t _GetCumulativeShifts(void)
{
	return __Gear.FeRAM->Read(MEM_ACC_SHIFTS);  // return saved data from FeRAM
}

// writes gear n into the FeRAM
static inline void _SetGear(const int8_t n)
{
	__Gear.FeRAM->Write(n, MEM_GEAR);  // write gear into FeRAM, at the proper location
}

// writes pulse count into FeRAM
static inline void _SetCumulativePulses(const uint32_t n)
{
	__Gear.FeRAM->Write(n, MEM_ACC_PULSES);  // return saved data from FeRAM
}

// writes gear shift count into FeRAM
static inline void _SetCumulativeShifts(const uint32_t n)
{
	__Gear.FeRAM->Write(n, MEM_ACC_SHIFTS);  // return saved data from FeRAM
}

// shifts the Rohloff hub n gears (-13 to + 13, except 0) up or down
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

	while(__Gear._FlagShiftinginProgress)
		;  // if some shifting is in progress, wait here...

	__Gear._FlagShiftinginProgress = 1;  // critical section start

	Device->mj8x8->UpdateActivity(SHIFTING, __Gear._FlagShiftinginProgress);  // update the bus

	__Gear._CurrentGear = _GetGear();  // get the current gear (we think we are in...)

	// if we need to shift up
	if(n > 0)
		{
			// guard against shifting past gear 14
			if(__Gear._CurrentGear + n > 14)
				{
					__Gear._FlagShiftinginProgress = 0;
					return;
				}

			__Gear.Motor->Shift(ShiftUp, n);  // shift up n gears
		}

	// if we need to shift down
	if(n < 0)
		{
			// guard against shifting past gear 1
			if((int8_t) (__Gear._CurrentGear + n < 1))
				{
					__Gear._FlagShiftinginProgress = 0;
					return;
				}

			__Gear.Motor->Shift(ShiftDown, -n);  // shift down n gears
		}

	_SetGear(__Gear._CurrentGear + n);  // write shifted gear into FeRAM

	_SetCumulativeShifts(_GetCumulativeShifts() + ABS(n));	// add n to the cumulative gears shifted count
	_SetCumulativePulses(_GetCumulativePulses() + *__Gear.Motor->PulseCount);  //

	__Gear._FlagShiftinginProgress = 0;  // critical section stop
	Device->mj8x8->UpdateActivity(SHIFTING, __Gear._FlagShiftinginProgress);  // update the bus
}

// shift Rohloff into gear n (1 to 14)
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
		return;  // additionally, e.g. -2 with uint8_t n becomes 0xfe - so negative numbers (i.e. very large positives) are also caught

	while(__Gear._FlagShiftinginProgress)
		;  // if some shifting is in progress, wait here...

	uint8_t _curr = _GetGear();  // get the current gear (we think we are in...)

	// if we are already in the gear n
	if(n == _curr)
		return;

	_ShiftByN(n - _curr);  // shift the difference
}

static __gear_t __Gear =  // instantiate gear_t actual and set function pointers
	{  //
	.public.ShiftByN = &_ShiftByN, 	// set function pointer
	.public.ShiftToN = &_ShiftToN,  // set function pointer
	.public.GetGear = &_GetGear  // set function pointer
	};

gear_t* gear_ctor(void)  //
{
	__Gear._FlagShiftinginProgress = 0;

	__enable_irq();  // PARTLY!!! enable interrupts -- essential for I2C
	__Gear.FeRAM = mb85rc_ctor();  // tie in FeRAM object
	__Gear.Motor = motor_ctor();	// tie in motor controller object

	__disable_irq();	// disable interrupts for the remainder of initialization - mj514.c mostly

	return &__Gear.public;  // set pointer to Gear public part
}

#endif
