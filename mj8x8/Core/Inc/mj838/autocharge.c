#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autocharge.h"

typedef struct	// autocharge_t actual
{
	autocharge_t public;  // public struct

} __autocharge_t;

static __autocharge_t __AutoCharge __attribute__ ((section (".data")));  // preallocate __AutoCharge object in .data

// starts the peripheral
static inline void _StartCharger(void)
{
	__AutoCharge.public.FlagChargerisActive = ON;
	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_SET);	// connect the load
	// TODO - autocharger - implement event
}

// stops the peripheral
static inline void _StopCharger(void)
{
	__AutoCharge.public.FlagChargerisActive = OFF;
	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_RESET);	// disconnect the load
	// TODO - autocharger - implement event
}

// AutoCharge functionality
static void _Do(void)
{
	if(__AutoCharge.public.FlagChargerisActive == OFF && Device->AutoDrive->GetSpeed_mps() > LOAD_CONNECT_THRESHOLD)  // load is disconnected
		{
			_StartCharger();
		}

	if(__AutoCharge.public.FlagChargerisActive == ON && Device->AutoDrive->GetSpeed_mps() < LOAD_CONNECT_THRESHOLD)  // load is disconnected
		{
			_StopCharger();
		}
}

static __autocharge_t __AutoCharge =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do  // set function pointer
	};

autocharge_t* autocharge_ctor(void)  //
{
	return &__AutoCharge.public;  // set pointer to AutoCharge public part
}

#endif
