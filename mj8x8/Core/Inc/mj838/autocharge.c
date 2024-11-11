#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autocharge.h"

typedef enum   // enum of lights on this device
{
	  SpeedStopped,  //
	  SpeedbelowChargerThres,  //
	  SpeedaboveChargerThres,  //
	  Speed0,  // standstill
	  Speed2,  // 2
	  Speed4,  // 4
	  Speed6,  // 6
	  Speed8,  // 8
	  Speed10,  // 10
	  Speed12  // 12
} autocharge_speedlevels_t;

typedef struct	// autocharge_t actual
{
	autocharge_t public;  // public struct

	volatile autocharge_speedlevels_t _SpeedLevel;	//
	volatile autocharge_speedlevels_t _previousSpeedLevel;	//

	// 0 - switch open, 1 - switch closed -- regardless of NC or NO
	uint8_t __SW1 :1;  // SSR SW1 state (NC) - 0 open, 1 closed
	uint8_t __SW2 :1;  // SSR SW2 state (NC) - 0 open, 1 closed
	uint8_t __SW_CA :1;  // SSR SW-CA state (NO) - 0 open, 1 closed
	uint8_t __SW_CB :1;  // SSR SW-CB state (NO) - 0 open, 1 closed
	uint8_t __SW_CC :1;  // SSR SW-CC state (NO) - 0 open, 1 closed
	uint8_t __SW_D :1;  // SSR SW-D state (NO) - 0 open, 1 closed
//	uint8_t __SW_X :1;  // SSR SW-CD state (NO) - 0 open, 1 closed

	uint8_t __LoadSwitch :1;  // High-Side Load Switch state - 0 open, 1 closed
} __autocharge_t;

static uint8_t _FlagStopChargerCalled = 0;	// flag indicating if _StopCharger() was called already
static volatile uint8_t _StartChargercntr = 0;
static volatile uint8_t _StopChargercntr = 0;
static volatile uint8_t _foocntr = 0;

static __autocharge_t __AutoCharge __attribute__ ((section (".data")));  // preallocate __AutoCharge object in .data

// returns High-Side load switch state: 0 - disconnected, 1 - connected
static inline uint8_t _IsLoadConnected(void)
{
	return HAL_GPIO_ReadPin(LoadFet_GPIO_Port, LoadFet_Pin);	// explicitly return load switch hardware state
}

// connect the load via a high-side load switch
static inline void _ConnectLoad(const uint8_t state)
{
	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, state);  // connect or disconnect  the load
	__AutoCharge.__LoadSwitch = state;
	Device->mj8x8->UpdateActivity(AUTOCHARGE, state);  // update the bus
}

// starts the peripheral
static inline void _StartCharger(void)
{
	if(_IsLoadConnected() == ON)  // if already on
		return;  // get out, nothing to do here
	++_StartChargercntr;
	_ConnectLoad(ON);  // connect the load

	EventHandler->Notify(EVENT03);	// notify event

	_FlagStopChargerCalled = 0;  // reset flag
}

// stops the peripheral
static inline void _StopCharger(void)
{
	if(_IsLoadConnected() == OFF && _FlagStopChargerCalled == 1)  // if already off and not called before
		return;

//	if(Device->ZeroCross->GetZCFrequency() <= 0)
//		return;

	++_StopChargercntr;
	_ConnectLoad(OFF);  // disconnect the load

	EventHandler->Notify(EVENT03);	// notify event

	_FlagStopChargerCalled = 1;  // mark as called
}

// SW1 control, NC
static inline void _SSR_SW1(const uint8_t state)
{
	HAL_GPIO_WritePin(SW1_CTRL_GPIO_Port, SW1_CTRL_Pin, !state);  // NC switch: 0 closed, 1 open
	__AutoCharge.__SW1 = state;
}

// SW2 control, NC
static inline void _SSR_SW2(const uint8_t state)
{
	HAL_GPIO_WritePin(SW2_CTRL_GPIO_Port, SW2_CTRL_Pin, !state);	// NC switch: 0 closed, 1 open
	__AutoCharge.__SW2 = state;
}

// SW-CA control, NO
static inline void _SSR_SW_CA(const uint8_t state)
{
	HAL_GPIO_WritePin(SW_CA_CTRL_GPIO_Port, SW_CA_CTRL_Pin, state);  // NO switch: 0 - open, 1 - closed
	__AutoCharge.__SW_CA = state;
}

// SW-CB control, NO
static inline void _SSR_SW_CB(const uint8_t state)
{
	HAL_GPIO_WritePin(SW_CB_CTRL_GPIO_Port, SW_CB_CTRL_Pin, state);  // NO switch: 0 - open, 1 - closed
	__AutoCharge.__SW_CB = state;
}

// SW-CC control, NO
static inline void _SSR_SW_CC(const uint8_t state)
{
	HAL_GPIO_WritePin(SW_CC_CTRL_GPIO_Port, SW_CC_CTRL_Pin, state);  // NO switch: 0 - open, 1 - closed
	__AutoCharge.__SW_CC = state;
}

// SW-D control, NO
static inline void _SSR_SW_D(const uint8_t state)
{
	HAL_GPIO_WritePin(SW_D_CTRL_GPIO_Port, SW_D_CTRL_Pin, state);  // NO switch: 0 - open, 1 - closed
	__AutoCharge.__SW_D = state;
}

//// SW-X control, NO
//static inline void _SSR_SW_CD(const uint8_t state)
//{
//		HAL_GPIO_WritePin(SW_X_CTRL_GPIO_Port, SW_X_CTRL_Pin, state);	// NO switch: 0 - open, 1 - closed
//	__AutoCharge.__SW_X = state;
//}

// AutoCharge functionality - called by timer 3 ISR - usually every 250ms
// FIXME - verify last remaining SSR operation (IIRC one SSR was fried and would not connect when activated)

// compares current and previous speed levels and sets speed level change flag
static inline uint8_t _CompareSpeedLevelsandFlag(const autocharge_speedlevels_t in_level)
{
	if(__AutoCharge._previousSpeedLevel == in_level)  // if speed level was not changed
		return 0;  // do not enable notification flag (see very first if-clause in Do())

	__AutoCharge._previousSpeedLevel = __AutoCharge._SpeedLevel;	// save current speed level
	__AutoCharge._SpeedLevel = in_level;  // set new light level
	return 1;  // enable notification flag (see calling if-clause in Do())
}

static void _Do(void)  // this actually runs the AutoCharge application
{
	// set speed flags on each measurement
	if(Device->AutoDrive->GetSpeed_mps() > 12)  // high enough speed - load is connected
		__AutoCharge._SpeedLevel = Speed12;
	else if(Device->AutoDrive->GetSpeed_mps() > 10)  // high enough speed - load is connected
		__AutoCharge._SpeedLevel = Speed10;
	else if(Device->AutoDrive->GetSpeed_mps() > 8)  // high enough speed - load is connected
		__AutoCharge._SpeedLevel = Speed8;
	else if(Device->AutoDrive->GetSpeed_mps() > 6)  // high enough speed - load is connected
		__AutoCharge._SpeedLevel = Speed6;
	else if(Device->AutoDrive->GetSpeed_mps() > 4)  // high enough speed - load is connected
		__AutoCharge._SpeedLevel = Speed4;
	else if(Device->AutoDrive->GetSpeed_mps() > LOAD_CONNECT_THRESHOLD_SPEED_HIGH)  // high enough speed - load is connected
		__AutoCharge._SpeedLevel = SpeedaboveChargerThres;
	else if(Device->AutoDrive->GetSpeed_mps() > 2)  // low speed - load is disconnected
		__AutoCharge._SpeedLevel = Speed2;
	else if(Device->AutoDrive->GetSpeed_mps() <= 0.1)  // stopped
		__AutoCharge._SpeedLevel = SpeedStopped;
	else if(Device->AutoDrive->GetSpeed_mps() < LOAD_CONNECT_THRESHOLD_SPEED_LOW)  // low speed - load is disconnected
		__AutoCharge._SpeedLevel = SpeedbelowChargerThres;

	if(_CompareSpeedLevelsandFlag(__AutoCharge._SpeedLevel))	// if the speed flags have changed
		{
			++_foocntr;

			if(__AutoCharge._SpeedLevel <= SpeedbelowChargerThres)  // low speed - load is disconnected
				{
					if(__AutoCharge._SpeedLevel == SpeedStopped)
						_FlagStopChargerCalled = 0;  // mark as not called

					_StopCharger();  // stop, but with a caveat

					_FlagStopChargerCalled = 0;  // mark as not called

					return;
				}

			// SSR control based on speed flags - logic will change
			if(__AutoCharge._SpeedLevel >= SpeedaboveChargerThres)  // high enough speed - load is connected
				_StartCharger();  // start, if not already started

			if(__AutoCharge._SpeedLevel == Speed12)
				_SSR_SW_D(ON);
			else
				_SSR_SW_D(OFF);

			if(__AutoCharge._SpeedLevel == Speed10)
				_SSR_SW_CC(ON);
			else
				_SSR_SW_CC(OFF);

			if(__AutoCharge._SpeedLevel == Speed8)
				_SSR_SW_CB(ON);
			else
				_SSR_SW_CB(OFF);

			if(__AutoCharge._SpeedLevel == Speed6)
				_SSR_SW_CA(ON);
			else
				_SSR_SW_CA(OFF);

			if(__AutoCharge._SpeedLevel == Speed4)
				_SSR_SW2(ON);
			else
				_SSR_SW2(OFF);

			if(__AutoCharge._SpeedLevel == Speed2)
				_SSR_SW1(ON);
			else
				_SSR_SW1(OFF);
		}
}

static __autocharge_t __AutoCharge =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.IsLoadConnected = &_IsLoadConnected,	// set function pointer
	.public.Do = &_Do  // ditto
	};

autocharge_t* autocharge_ctor(void)  //
{
//	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_RESET);	// explicitly again for clarity - start with load disconnected !!!

	_FlagStopChargerCalled = 0;
	__AutoCharge.__LoadSwitch = HAL_GPIO_ReadPin(LoadFet_GPIO_Port, LoadFet_Pin);  // read out initial switch states
	__AutoCharge.__SW1 = HAL_GPIO_ReadPin(SW1_CTRL_GPIO_Port, SW1_CTRL_Pin);	// ditto
	__AutoCharge.__SW2 = HAL_GPIO_ReadPin(SW2_CTRL_GPIO_Port, SW2_CTRL_Pin);
	__AutoCharge.__SW_CA = HAL_GPIO_ReadPin(SW_CA_CTRL_GPIO_Port, SW_CA_CTRL_Pin);
	__AutoCharge.__SW_CB = HAL_GPIO_ReadPin(SW_CB_CTRL_GPIO_Port, SW_CB_CTRL_Pin);
	__AutoCharge.__SW_CC = HAL_GPIO_ReadPin(SW_CC_CTRL_GPIO_Port, SW_CC_CTRL_Pin);
	__AutoCharge.__SW_D = HAL_GPIO_ReadPin(SW_D_CTRL_GPIO_Port, SW_D_CTRL_Pin);
//	__AutoCharge.__SW_X = HAL_GPIO_ReadPin(SW_X_CTRL_GPIO_Port, SW_X_CTRL_Pin);

	return &__AutoCharge.public;  // set pointer to AutoCharge public part
}

#endif
