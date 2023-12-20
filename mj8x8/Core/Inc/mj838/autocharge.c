#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autocharge.h"

typedef struct	// autocharge_t actual
{
	autocharge_t public;  // public struct

} __autocharge_t;

static uint8_t _FlagStopChargerCalled = 0;	// flag indicating if _StopCharger() was called already

static __autocharge_t __AutoCharge __attribute__ ((section (".data")));  // preallocate __AutoCharge object in .data

// starts the peripheral
static inline void _StartCharger(void)
{
	if(__AutoCharge.public.FlagLoadisConnected == ON)  // if already on
		return;  // get out, nothing to do here

	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_SET);	// connect the load

	Device->mj8x8->UpdateActivity(AUTOCHARGE, ON);	// mark device as on
	__AutoCharge.public.FlagLoadisConnected = ON;

	EventHandler->Notify(EVENT03);	// notify event

	_FlagStopChargerCalled = 0;  // reset flag
}

// stops the peripheral
static inline void _StopCharger(void)
{
	if(__AutoCharge.public.FlagLoadisConnected == OFF && _FlagStopChargerCalled == 1)  // if already off and not called before
		return;

	EventHandler->Notify(EVENT03);	// notify event

	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_RESET);	// disconnect the load

	Device->mj8x8->UpdateActivity(AUTOCHARGE, OFF);  // mark device as on
	__AutoCharge.public.FlagLoadisConnected = OFF;

	_FlagStopChargerCalled = 1;  // mark as called
}

// AutoCharge functionality
static void _Do(void)
{
	if(Device->AutoDrive->GetSpeed_mps() < LOAD_CONNECT_THRESHOLD_SPEED_LOW)  // low speed - load is disconnected
		{
			_StopCharger();  // stop, but with a caveat
		}

	if(Device->AutoDrive->GetSpeed_mps() > LOAD_CONNECT_THRESHOLD_SPEED_HIGH)  // high enough speed - load is connected
		{
			_StartCharger();	// start, if not already started
		}
}

static __autocharge_t __AutoCharge =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do  // set function pointer
	};

autocharge_t* autocharge_ctor(void)  //
{
	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_RESET);	// explicitly again for clarity - start with load disconnected !!!

	return &__AutoCharge.public;  // set pointer to AutoCharge public part
}

#endif
