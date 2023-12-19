#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

extern TIM_HandleTypeDef htim3;  // Timer3 object - measurement/calculation interval of timer2 data - default 250ms

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
	union m  // distance in meters
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} m;
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
	return __AutoDrive.kph.Float;
}

// get speed in kilometres per hour
static inline float _GetDistance_m(void)
{
	return __AutoDrive.m.Float;
}

// AutoDrive functionality based on detected zero cross frequency
static void _Do(void)
{
	__AutoDrive._WheelFrequency = Device->ZeroCross->GetZCFrequency() / POLE_COUNT;  // ZeroCross signal frequency to wheel RPS
	__AutoDrive.mps.Float = __AutoDrive._WheelFrequency * WHEEL_CIRCUMFERENCE;	// wheel frequency to m/s
	__AutoDrive.kph.Float = __AutoDrive.mps.Float * 3.6;  // m/s to km/h
	__AutoDrive.m.Float += __AutoDrive.mps.Float * (float) ((__HAL_TIM_GET_AUTORELOAD(&htim3) + 1) / 10000.0);  // distance, mps * measurement interval

#if SIGNAL_GENERATOR_INPUT
	if((uint8_t) last_mps != (uint8_t) __AutoDrive.mps.Float)  // only if data has changed
		{
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire

			last_mps = __AutoDrive.mps.Float;  // store current speed for comparison in the next cycle
		}
#else
	MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
	MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
#endif
}

static __autodrive_t __AutoDrive =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.FlagLightisOn = 0,	// flag if AutoDrive turned Light on
	.public.GetSpeed_mps = &_GetSpeed_mps,  // get speed in meters per second
	.public.GetSpeed_kph = &_GetSpeed_kph,  // get speed in kilometres per hour
	.public.GetDistance_m = &_GetDistance_m,  // get distance in meters
	.public.Do = &_Do  // set function pointer
	};

autodrive_t* autodrive_ctor(void)  //
{
	return &__AutoDrive.public;  // set pointer to AutoBatt public part
}

#endif
