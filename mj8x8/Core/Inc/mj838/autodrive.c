#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

#include <string.h>

extern TIM_HandleTypeDef htim2;  // Timer3 object - measurement/calculation interval of timer2 data - default 250ms
extern TIM_HandleTypeDef htim16;  // Timer16 object - odometer & co. 1s

typedef enum   // enum of light levels on this device
{
	  LightOff, 	 	// lights off with delay - front 0%, rear 0%
	  LightDim,	  	// dim light - front 10%, rear 25%
	  LightLow,			// low light - front 35%, rear 50%
	  LightNormal,  // normal light - front 50%, rear 100%
	  LightHigh,  	// high light - front 100%, rear 100%
} autodrive_lightlevels_t;

typedef struct	// autodrive_t actual
{
	autodrive_t public;  // public struct

	autodrive_lightlevels_t _LightLevel;
	autodrive_lightlevels_t _previousLightLevel;
	float _WheelFrequency;  // wheel rotation frequency

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
	union Odometer  // distance in meters
	{
		float Float;
		uint8_t Bytes[sizeof(float)];
	} Odometer;
} __autodrive_t;

static __autodrive_t __AutoDrive __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

#if SIGNAL_GENERATOR_INPUT
static float _last_mps = 0;  // used to check if speed has changed
#endif

uint8_t _Timer16Cntr = 0;  // timer2 counter

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
	if(Device->mj8x8->GetActivity(AUTODRIVE))  // run only if AD is on
		{
			Device->mj8x8->UpdateActivity(AUTODRIVE, OFF);  // update the bus
			EventHandler->Notify(EVENT07);	// generate event
		}
}

// compares current and previous light levels and sets light level change flag
static inline uint8_t _CompareLightLevelsandFlag(const autodrive_lightlevels_t in_level)
{
	if(__AutoDrive._previousLightLevel == in_level)  // if light level was not changed
		return 0;  // do not enable notification flag (see very first if-clause in Do())

	__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;	// save current light level
	__AutoDrive._LightLevel = in_level;  // set new light level
	return 1;  // enable notification flag (see calling if-clause in Do())
}

// AutoDrive functionality based on detected zero cross frequency - called by timer 3 ISR - usually every 250ms
static void _Do(void)  // this actually runs the AutoDrive application
{
	const float KPH_CONVERSION = 3.6f;
	const float TIME_CONVERSION = 10000.0f;

	__AutoDrive._WheelFrequency = (float) (Device->ZeroCross->GetZCFrequency() / POLE_COUNT);  // ZeroCross signal frequency to wheel RPS
	__AutoDrive.mps.Float = (float) (__AutoDrive._WheelFrequency * WHEEL_CIRCUMFERENCE);  // wheel frequency to m/s
	__AutoDrive.kph.Float = (float) (__AutoDrive.mps.Float * KPH_CONVERSION);  // m/s to km/h
	__AutoDrive.m.Float += __AutoDrive.mps.Float * (float) ((float) (__HAL_TIM_GET_AUTORELOAD(&htim2) + 1) / TIME_CONVERSION);  // distance, mps * measurement interval

#if SIGNAL_GENERATOR_INPUT	// ZeroCross signal is generator input
	if((uint8_t) _last_mps != (uint8_t) __AutoDrive.mps.Float)  // only if data has changed
		{
			// send speed over the wire - only when it changes
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire

			_last_mps = __AutoDrive.mps.Float;  // store current speed for comparison in the next cycle
		}
#else // ZeroCross signal is from the wheel
	// send speed over the wire - since this is not as constant as the generator input, send it every time
	MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
	MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
#endif

	/* light level determination
	 *
	 * speed (i.e. zerocross frequency) is measured constantly as long as there is a signal.
	 * if there is no signal, measurement (i.e. timers) are stopped and EXTI0_1 should wake the whole device up
	 *
	 * speed thresholds (light levels) are examined constantly, yet only on light level state change (e.g. low to high due to increased speed),
	 * 	 one event will be sent out.
	 *
	 */

	// light level determination
	if(__AutoDrive.kph.Float < 1)
		__AutoDrive._LightLevel = LightOff;
	else if(__AutoDrive.kph.Float > 40)
		__AutoDrive._LightLevel = LightHigh;
	else if(__AutoDrive.kph.Float > 10)
		__AutoDrive._LightLevel = LightNormal;
	else if(__AutoDrive.kph.Float > 3)
		__AutoDrive._LightLevel = LightLow;
	else
		__AutoDrive._LightLevel = LightDim;

	if(_CompareLightLevelsandFlag(__AutoDrive._LightLevel))  // compares current and previous light levels and sets light level change flag
		{  // notifications should be sent only once on state change
			if(__AutoDrive._LightLevel > LightOff && Device->mj8x8->GetActivity(AUTODRIVE) == OFF)  // any speed greater than standstill and AD is off
				Device->mj8x8->UpdateActivity(AUTODRIVE, ON);  // update the bus

			uint8_t event = 0;  // event to be sent

			switch(__AutoDrive._LightLevel)
				// set event according to light level
				{
				case LightDim:  // dim light - front 10%, rear 25%
					event = EVENT02;
					break;
				case LightLow:  // low light - front 35%, rear 50%
					event = EVENT04;
					break;
				case LightNormal:  // normal light - front 50%, rear 100%
					event = EVENT05;
					break;
				case LightHigh:  // high light - front 100%, rear 100%
					event = EVENT06;
					break;
				case LightOff: 	// lights off with delay - front 0%, rear 0%
					event = EVENT07;
					break;

				default:
					break;
				}

			EventHandler->Notify(event);  // generate event
		}
}

// update odometer value in FeRAM
void _UpdateOdometer(void)
{
	uint32_t oldval = Device->FeRAM->Read(ODOMETER_ADDR);  // read stored odometer value from FeRAM
	memcpy(&__AutoDrive.Odometer.Float, &oldval, sizeof(float));  // copy odometer to Odometer

	__AutoDrive.Odometer.Float += __AutoDrive.m.Float;  // add current odometer to old value
	__AutoDrive.m.Float = 0;  // reset current odometer
	oldval = 0;  // reset oldval

	memcpy(&oldval, &__AutoDrive.Odometer.Float, sizeof(float));  // copy odometer to oldval
	Device->FeRAM->Write(oldval, ODOMETER_ADDR);  // write odometer to FeRAM)

	_Timer16Cntr = 0;  // reset counter
}

static __autodrive_t __AutoDrive =  // instantiate autobatt_t actual and set function pointers
	{  //
//	.public.FlagLightisOn = 0,	// flag if AutoDrive turned Light on
	.public.GetSpeed_mps = &_GetSpeed_mps,  // get speed in meters per second
	.public.GetSpeed_kph = &_GetSpeed_kph,  // get speed in kilometres per hour
	.public.GetDistance_m = &_GetDistance_m,  // get distance in meters
	.public.Do = &_Do,  // set function pointer
	.public.LightOff = &_LightOff,  // set function pointer
	.public.UpdateOdometer = &_UpdateOdometer,  // set function pointer
	};

autodrive_t* autodrive_ctor(void)  //
{
	_UpdateOdometer();
	return &__AutoDrive.public;  // set pointer to AutoBatt public part
}

// timer 16 ISR - odometer & co. timer
void TIM16_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim16);  // service the interrupt
	++_Timer16Cntr;

	if(_Timer16Cntr % ODOMETER_REFRESH_PERIOD == 0)  // once per minute
		{
			__AutoDrive.public.UpdateOdometer();  // update odometer value in FeRAM
			Device->Humidity->Measure();  // measure humidity
		}
}
#endif
