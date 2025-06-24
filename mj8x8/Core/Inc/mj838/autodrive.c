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

#define FIXED_POINT_SCALE 1000 // Scaling factor for fixed-point math
#define KPH_CONVERSION 3600 // Fixed-point equivalent of 3.6 (scaled by FIXED_POINT_SCALE)
#define TIME_CONVERSION 10000 // Fixed-point equivalent of 1.0 (scaled by FIXED_POINT_SCALE)

#if USE_DYNAMIC_LIGHT
#define FRONT_MIN_BRIGHTNESS 10	// 10% brightness
#define REAR_MIN_BRIGHTNESS 25	// 25% brightness
#define FRONT_MAX_BRIGHTNESS 75	// 75% brightness
#define REAR_MAX_BRIGHTNESS 100	// 100% brightness
#define STOPPED 500 // 0.5 km/h - speed below which the light is turned off
#define WALKING_SPEED 5000	// 5 km/h
#define SLOW_RIDING 10000 // 10 km/h
#define BURNING_RUBBER 35000 // 35 km/h
#endif

typedef struct	// autodrive_t actual
{
#if !USE_DYNAMIC_LIGHT
	autodrive_lightlevels_t _LightLevel;	// current light level
	autodrive_lightlevels_t _previousLightLevel;	// previous light level
#endif
	uint32_t _WheelFrequency;  // wheel rotation frequency

	autodrive_t public;  // public struct
} __autodrive_t;

static __autodrive_t __AutoDrive __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

#if SIGNAL_GENERATOR_INPUT
static uint32_t _last_mps = 0;  // used to check if speed has changed
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

	if(__AutoDrive.public.kph.uint32 <= BURNING_RUBBER)  // dim below BURNING_RUBBER speed
		frontLightLevel = FRONT_MIN_BRIGHTNESS + (uint8_t) ((__AutoDrive.public.kph.uint32 - WALKING_SPEED) / 500);  // adjust front light level
	/* (FRONT_MAX_BRIGHTNESS - FRONT_MIN_BRIGHTNESS) / (BURNING_RUBBER - WALKING_SPEED)
	 * 	= (75 - 10) / (35km/h - 5km/h)
	 * 	= 65 / 30 = 2.1667
	 * 	approximately 1/500 in fixed-point math
	 */

	// Check if the speed is less than or equal to SLOW_RIDING
	if(__AutoDrive.public.kph.uint32 <= SLOW_RIDING)
		rearLightLevel = REAR_MIN_BRIGHTNESS + (uint8_t) ((__AutoDrive.public.kph.uint32 - WALKING_SPEED) / 67);  // adjust rear light level
	/*
	 * (REAR_MAX_BRIGHTNESS - REAR_MIN_BRIGHTNESS) / (SLOW_RIDING - WALKING_SPEED)
	 * 	= (100 - 25) / (10km/h - 5km/h)
	 * 	= 75 / 5 = 15
	 * 	approximately 1/67 in fixed-point math
	 */

	if(__AutoDrive.public.kph.uint32 < WALKING_SPEED)  // don't go below a certain light level
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
	__AutoDrive._WheelFrequency = Device->ZeroCross->ZeroCrossFrequency / POLE_COUNT;  // ZeroCross signal frequency to wheel RPS
	__AutoDrive.public.mps.uint32 = (__AutoDrive._WheelFrequency * WHEEL_CIRCUMFERENCE) / FIXED_POINT_SCALE;  // wheel frequency to m/s
	__AutoDrive.public.kph.uint32 = (__AutoDrive.public.mps.uint32 * KPH_CONVERSION) / FIXED_POINT_SCALE;  // m/s to km/h
	__AutoDrive.public.m.uint32 += (__AutoDrive.public.mps.uint32 * (__HAL_TIM_GET_AUTORELOAD(&htim2) + 1)) / TIME_CONVERSION;  // distance, mps * measurement interval

#if SIGNAL_GENERATOR_INPUT  // ZeroCross signal is generator input
	if((uint8_t) (_last_mps / FIXED_POINT_SCALE) != (uint8_t) (__AutoDrive.public.mps.uint32 / FIXED_POINT_SCALE))  // only if data has changed
		{
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.public.mps.Bytes, 1 + sizeof(int32_t));  // send speed over the wire
			MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.public.mps.Bytes, 1 + sizeof(int32_t));  // send speed over the wire

			_last_mps = __AutoDrive.public.mps.uint32;  // store current speed for comparison in the next cycle
		}
#else // ZeroCross signal is from the wheel
    MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_SPEED, __AutoDrive.public.mps.Bytes, 1 + sizeof(int32_t));  // send speed over the wire
    MsgHandler->SendMessage(mj828, MSG_MEASUREMENT_ACCEL, __AutoDrive.public.mps.Bytes, 1 + sizeof(int32_t));  // send speed over the wire
#endif

	// Adjust light levels based on speed
#if USE_DYNAMIC_LIGHT
	AdjustLightBasedOnSpeed();
#endif
#if !USE_DYNAMIC_LIGHT
    // Light level determination
    if(__AutoDrive.public.kph.Fixed < (1 * FIXED_POINT_SCALE))
        __AutoDrive._LightLevel = LightOff;
    else if(__AutoDrive.public.kph.Fixed > (40 * FIXED_POINT_SCALE))
        __AutoDrive._LightLevel = LightHigh;
    else if(__AutoDrive.public.kph.Fixed > (10 * FIXED_POINT_SCALE))
        __AutoDrive._LightLevel = LightNormal;
    else if(__AutoDrive.public.kph.Fixed > (3 * FIXED_POINT_SCALE))
        __AutoDrive._LightLevel = LightLow;
    else
        __AutoDrive._LightLevel = LightDim;

    _AdjustLightinSteps();
#endif
}

// Update odometer value in FeRAM
void _UpdateOdometer(void)
{
	uint32_t oldval = Device->FeRAM->Read(ODOMETER_ADDR);  // read stored odometer value from FeRAM
	memcpy(&__AutoDrive.public.Odometer.uint32, &oldval, sizeof(int32_t));  // copy odometer to Odometer

	__AutoDrive.public.Odometer.uint32 += __AutoDrive.public.m.uint32;  // add current odometer to old value
	__AutoDrive.public.m.uint32 = 0;  // reset current odometer
	oldval = 0;  // reset oldval

	memcpy(&oldval, &__AutoDrive.public.Odometer.uint32, sizeof(int32_t));  // copy odometer to oldval
	Device->FeRAM->Write(oldval, ODOMETER_ADDR);  // write odometer to FeRAM

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
	Device->Humidity->Measure();  // measure humidity
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
