#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

typedef struct	// autobatt_t actual
{
	autodrive_t public;  // public struct

	volatile float _WheelFrequency;  // wheel rotation frequency
	volatile float _ms;  // speed in m/s
	volatile float _kmh;	// speed in km/h
} __autodrive_t;

static __autodrive_t __AutoDrive __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

// get speed in meters per second
static inline float _GetSpeed_mps(void)
{
	return __AutoDrive._ms;
}

// get speed in kilometres per hour
static inline float _GetSpeed_kph(void)
{
	return __AutoDrive._kmh;
}

// AutoDrive functionality based on detected speed
static void _Do(void)
{
	__AutoDrive._WheelFrequency = Device->ZeroCross->GetZCFrequency() / POLE_COUNT;  // FIXME - validate with real wheel - derive wheel RPS
	__AutoDrive._ms = __AutoDrive._WheelFrequency * WHEEL_CIRCUMFERENCE;	// wheel frequency to m/s
	__AutoDrive._kmh = __AutoDrive._ms * 3.6;  // m/s to km/h
}

static __autodrive_t __AutoDrive =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.FlagLightisOn = 0,	// flag if AutoDrive turned Light on
	.public.GetSpeed_mps = &_GetSpeed_mps,  // get speed in meters per second
	.public.GetSpeed_kph = &_GetSpeed_kph,  // get speed in kilometres per hour
	.public.Do = &_Do  // set function pointer
	};

autodrive_t* autodrive_ctor(void)  //
{
	return &__AutoDrive.public;  // set pointer to AutoBatt public part
}

#endif
