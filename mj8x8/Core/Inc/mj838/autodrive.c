#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

#include <string.h>

extern TIM_HandleTypeDef htim2;  // Timer3 object - measurement/calculation interval of timer2 data - default 250ms
extern TIM_HandleTypeDef htim16;  // Timer16 object - odometer & co. 1s

#if !USE_DYNAMIC_LIGHT
typedef enum   // enum of light levels on this device
{
	  LightOff, 	 	// lights off with delay - front 0%, rear 0%
	  LightDim,	  	// dim light - front 10%, rear 25%
	  LightLow,			// low light - front 35%, rear 50%
	  LightNormal,  // normal light - front 50%, rear 100%
	  LightHigh,  	// high light - front 100%, rear 100%
} autodrive_lightlevels_t;
#endif

#if USE_DYNAMIC_LIGHT
#define FRONT_MIN_BRIGHTNESS 10	// 10% brightness
#define REAR_MIN_BRIGHTNESS 25	// 25% brightness
#define FRONT_MAX_BRIGHTNESS 75	// 75% brightness
#define REAR_MAX_BRIGHTNESS 100	// 100% brightness
#define STOPPED 0.5 // 0.5 km/h - speed below which the light is turned off
#define WALKING_SPEED 5.0	// 5 km/h
#define SLOW_RIDING 10.0 // 10 km/h
#define BURNING_RUBBER 35.0 // 35 km/h
#endif

typedef struct	// autodrive_t actual
{
#if !USE_DYNAMIC_LIGHT
	autodrive_lightlevels_t _LightLevel;	// current light level
	autodrive_lightlevels_t _previousLightLevel;	// previous light level
#endif
	float _WheelFrequency;  // wheel rotation frequency

	autodrive_t public;  // public struct
} __autodrive_t;

static __autodrive_t __AutoDrive __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

#if SIGNAL_GENERATOR_INPUT
static float _last_mps = 0;  // used to check if speed has changed
#endif

uint8_t _Timer16Cntr = 0;  // timer2 counter

// turn on AutoDrive
static inline void _AutoDriveOn(void)
{
	if(Device->mj8x8->GetActivity(AUTODRIVE) == OFF)  // run only if AD is off
		Device->mj8x8->UpdateActivity(AUTODRIVE, ON);  // update the bus
}

// turn off AutoDrive
static inline void _AutoDriveOff(void)
{
	if(Device->mj8x8->GetActivity(AUTODRIVE))  // run only if AD is on
		{
			Device->mj8x8->UpdateActivity(AUTODRIVE, OFF);  // update the bus
			EventHandler->Notify(EVENT07);	// generate event
		}
}

#if USE_DYNAMIC_LIGHT
// adjust light level dynamically,  based on speed
static inline void AdjustLightBasedOnSpeed(void)
{
	uint8_t frontLightLevel = FRONT_MAX_BRIGHTNESS;  //	start with maximal values and work down
	uint8_t rearLightLevel = REAR_MAX_BRIGHTNESS;

	if(__AutoDrive.public.kph.Float <= BURNING_RUBBER)  // dim below BURNING_RUBBER speed
		{
			frontLightLevel = FRONT_MIN_BRIGHTNESS + (uint8_t) ((float) (__AutoDrive.public.kph.Float - WALKING_SPEED) * 2.1667f);	// adjust front light level
			// 2.1667 = (FRONT_MAX_BRIGHTNESS - FRONT_MIN_BRIGHTNESS) / (BURNING_RUBBER - WALKING_SPEED)

			// Check if the speed is less than or equal to SLOW_RIDING
			if(__AutoDrive.public.kph.Float <= SLOW_RIDING)
				rearLightLevel = REAR_MIN_BRIGHTNESS + (uint8_t) ((float) (__AutoDrive.public.kph.Float - WALKING_SPEED) * 15);  // adjust rear light level
			// 15 = (REAR_MAX_BRIGHTNESS - REAR_MIN_BRIGHTNESS) / (SLOW_RIDING - WALKING_SPEED)));
		}

	if(__AutoDrive.public.kph.Float < WALKING_SPEED)	// don't go below a certain light level
		{
			frontLightLevel = FRONT_MIN_BRIGHTNESS;
			rearLightLevel = REAR_MIN_BRIGHTNESS;
		}

	MsgHandler->SendMessage(mj808, MSG_BUTTON_EVENT_00, &frontLightLevel, 2);  // send front light level
	MsgHandler->SendMessage(mj818, MSG_BUTTON_EVENT_00, &rearLightLevel, 2);  // send rear light level
}
#endif

#if !USE_DYNAMIC_LIGHT
// compares current and previous light levels and sets light level change flag
static inline uint8_t _CompareLightLevelsandFlag(const autodrive_lightlevels_t in_level)
{
	if(__AutoDrive._previousLightLevel == in_level)  // if light level was not changed
		return 0;  // do not enable notification flag (see very first if-clause in Do())

	__AutoDrive._previousLightLevel = __AutoDrive._LightLevel;	// save current light level
	__AutoDrive._LightLevel = in_level;  // set new light level
	return 1;  // enable notification flag (see calling if-clause in Do())
}

// adjust light level in steps,  based on speed
static inline void _AdjustLightinSteps(void)
{
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
#endif

// AutoDrive functionality based on detected zero cross frequency - called by timer 3 ISR - usually every 250ms
static void _Do(void)  // this actually runs the AutoDrive application
{
	const float KPH_CONVERSION = 3.6f;
	const float TIME_CONVERSION = 10000.0f;

	__AutoDrive._WheelFrequency = (float) (Device->ZeroCross->ZeroCrossFrequency / POLE_COUNT);  // ZeroCross signal frequency to wheel RPS
	__AutoDrive.public.mps.Float = (float) (__AutoDrive._WheelFrequency * WHEEL_CIRCUMFERENCE);  // wheel frequency to m/s
	__AutoDrive.public.kph.Float = (float) (__AutoDrive.public.mps.Float * KPH_CONVERSION);  // m/s to km/h
	__AutoDrive.public.m.Float += __AutoDrive.public.mps.Float * (float) ((float) (__HAL_TIM_GET_AUTORELOAD(&htim2) + 1) / TIME_CONVERSION);  // distance, mps * measurement interval

#if SIGNAL_GENERATOR_INPUT	// ZeroCross signal is generator input
	if((uint8_t) _last_mps != (uint8_t) __AutoDrive.public.mps.Float)  // only if data has changed
		{
			// send speed over the wire - only when it changes
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.public.mps.Bytes, 1 + sizeof(float));  // send speed over the wire
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.public.mps.Bytes, 1 + sizeof(float));  // send speed over the wire

			_last_mps = __AutoDrive.public.mps.Float;  // store current speed for comparison in the next cycle
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

#if USE_DYNAMIC_LIGHT
	AdjustLightBasedOnSpeed();  // adjust light level dynamically,  based on speed
#endif
#if !USE_DYNAMIC_LIGHT
	// light level determination
	if(__AutoDrive.public.kph.Float < 1)
		__AutoDrive._LightLevel = LightOff;
	else if(__AutoDrive.public.kph.Float > 40)
		__AutoDrive._LightLevel = LightHigh;
	else if(__AutoDrive.public.kph.Float > 10)
		__AutoDrive._LightLevel = LightNormal;
	else if(__AutoDrive.public.kph.Float > 3)
		__AutoDrive._LightLevel = LightLow;
	else
		__AutoDrive._LightLevel = LightDim;

	_AdjustLightinSteps();  // adjust light level in steps,  based on speed
#endif
}

// update odometer value in FeRAM
void _UpdateOdometer(void)
{
	uint32_t oldval = Device->FeRAM->Read(ODOMETER_ADDR);  // read stored odometer value from FeRAM
	memcpy(&__AutoDrive.public.Odometer.Float, &oldval, sizeof(float));  // copy odometer to Odometer

	__AutoDrive.public.Odometer.Float += __AutoDrive.public.m.Float;  // add current odometer to old value
	__AutoDrive.public.m.Float = 0;  // reset current odometer
	oldval = 0;  // reset oldval

	memcpy(&oldval, &__AutoDrive.public.Odometer.Float, sizeof(float));  // copy odometer to oldval
	Device->FeRAM->Write(oldval, ODOMETER_ADDR);  // write odometer to FeRAM)

	_Timer16Cntr = 0;  // reset counter
}

static __autodrive_t __AutoDrive =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do,  // set function pointer
	.public.AutoDriveOff = &_AutoDriveOff,  // set function pointer
	.public.AutoDriveOn = &_AutoDriveOn,  // ditto
	.public.UpdateOdometer = &_UpdateOdometer,  // ditto
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
