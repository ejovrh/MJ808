#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

typedef struct	// autobatt_t actual
{
	autodrive_t public;  // public struct

	volatile float _WheelFrequency;  // wheel rotation frequency
	union mps  // meters per second
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} mps;
	union kph  // kilometres per hour
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} kph;

	volatile float _kmh;	// speed in km/h
} __autodrive_t;

static __autodrive_t __AutoDrive __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

static float last_mps = 0;

// get speed in meters per second
static inline float _GetSpeed_mps(void)
{
	return __AutoDrive.mps.Float;
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
	__AutoDrive.mps.Float = __AutoDrive._WheelFrequency * WHEEL_CIRCUMFERENCE;	// wheel frequency to m/s
	__AutoDrive.kph.Float = __AutoDrive.mps.Float * 3.6;  // m/s to km/h

	// TODO - verify correct working
	if(last_mps != __AutoDrive.mps.Float)  // although the direct comparison of two floats is nonsense, this works...
		{  // ... because of "rounding" in ZeroCross _CalculateZCFrequency()
		   // FIXME - correct MsgHandler->SendMessage() so that data packets can be sent
			MsgHandler->SendMessage(mj828, 0xDE, __AutoDrive.mps.Bytes, sizeof(float));	// send speed over the wire
			last_mps = __AutoDrive.mps.Float;
		}
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
