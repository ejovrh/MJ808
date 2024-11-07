#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

extern TIM_HandleTypeDef htim3;  // Timer3 object - measurement/calculation interval of timer2 data - default 250ms

typedef enum   // enum of lights on this device
{
	  LightOff,  // bit pos. 0,
	  LightDim,  //
	  LightLow,  // bit pos. 1
	  LightNormal,  // bit pos. 2
	  LightHigh,  // bit pos. 3
} autodrive_lightlevels;

typedef struct	// autodrive_t actual
{
	autodrive_t public;  // public struct

	volatile uint8_t _FlagLightlevelChange;  // flag indicating light level should change according to changed speed
	volatile autodrive_lightlevels _LightLevel;
	volatile autodrive_lightlevels _previousLightLevel;
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

// get distance in meters
static inline float _GetDistance_m(void)
{
	return __AutoDrive.m.Float;
}

// we are at standstill
static inline void _LightOff(void)
{
	EventHandler->Notify(EVENT07);	// generate event
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

	// TODO - implement light intensity based on speed

	if(__AutoDrive._FlagLightlevelChange)
		{
			if(__AutoDrive._LightLevel == LightDim)
				EventHandler->Notify(EVENT02);	// generate event

			if(__AutoDrive._LightLevel == LightLow)
				EventHandler->Notify(EVENT04);	// generate event

			if(__AutoDrive._LightLevel == LightNormal)
				EventHandler->Notify(EVENT05);	// generate event

			if(__AutoDrive._LightLevel == LightHigh)
				EventHandler->Notify(EVENT06);	// generate event

			__AutoDrive._FlagLightlevelChange = 0;
			return;
		}
	else
		{
			if(__AutoDrive.kph.Float < 1)  // fZC of below approx. 1.87 Hz - considered standstill
				{  // lights off with delay
				   // front 0%, rear 0%
					__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;
					__AutoDrive._LightLevel = LightOff;

					if(__AutoDrive._previousLightLevel != __AutoDrive._LightLevel)
						__AutoDrive._FlagLightlevelChange = 1;

					return;
				}

			if(__AutoDrive.kph.Float < 3)  // fZC of approx. 5.58 Hz - walking speed
				{  // dim light
				   // front 10%, rear 25%
					__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;
					__AutoDrive._LightLevel = LightDim;

					if(__AutoDrive._previousLightLevel != __AutoDrive._LightLevel)
						__AutoDrive._FlagLightlevelChange = 1;

					return;
				}

			if(__AutoDrive.kph.Float <= 10)  // fZC of approx. 18.57 Hz - slow ride
				{  // low light
				   // front 35%, rear 50%
					__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;
					__AutoDrive._LightLevel = LightLow;

					if(__AutoDrive._previousLightLevel != __AutoDrive._LightLevel)
						__AutoDrive._FlagLightlevelChange = 1;

					return;
				}

			if(__AutoDrive.kph.Float > 40)  // fZC of approx. 74.29 Hz - faster than light travel
				{
					// front 100%, rear 100%
					__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;
					__AutoDrive._LightLevel = LightHigh;

					if(__AutoDrive._previousLightLevel != __AutoDrive._LightLevel)
						__AutoDrive._FlagLightlevelChange = 1;

					return;
				}

			if(__AutoDrive.kph.Float > 10)  // fZC of approx. 18.57 Hz - normal ride
				{
					// front 50%, rear 100%
					__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;
					__AutoDrive._LightLevel = LightNormal;

					if(__AutoDrive._previousLightLevel != __AutoDrive._LightLevel)
						__AutoDrive._FlagLightlevelChange = 1;

					return;
				}
		}
}

static __autodrive_t __AutoDrive =  // instantiate autobatt_t actual and set function pointers
	{  //
//	.public.FlagLightisOn = 0,	// flag if AutoDrive turned Light on
	.public.GetSpeed_mps = &_GetSpeed_mps,  // get speed in meters per second
	.public.GetSpeed_kph = &_GetSpeed_kph,  // get speed in kilometres per hour
	.public.GetDistance_m = &_GetDistance_m,  // get distance in meters
	.public.Do = &_Do,  // set function pointer
	.public.LightOff = &_LightOff  // set function pointer
	};

autodrive_t* autodrive_ctor(void)  //
{
	return &__AutoDrive.public;  // set pointer to AutoBatt public part
}

#endif
