#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autocharge.h"

typedef struct	// autocharge_t actual
{
	autocharge_t public;  // public struct

	uint8_t __SW1 :1;  // SSR SW1 state (NC) - 0 open, 1 closed
	uint8_t __SW2 :1;  // SSR SW1 state (NC) - 0 open, 1 closed
	uint8_t __SW_CA :1;  // SSR SW1 state (NO) - 0 open, 1 closed
	uint8_t __SW_CB :1;  // SSR SW1 state (NO) - 0 open, 1 closed
	uint8_t __SW_CC :1;  // SSR SW1 state (NO) - 0 open, 1 closed
	uint8_t __SW9 :1;  // SSR SW1 state (NO) - 0 open, 1 closed
	uint8_t __LoadSwitch :1;  // High-Side Load Switch state - 0 open, 1 closed
} __autocharge_t;

static uint8_t _FlagStopChargerCalled = 0;	// flag indicating if _StopCharger() was called already

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
	Device->mj8x8->UpdateActivity(AUTOCHARGE, state);  // mark device as on
}

// starts the peripheral
static inline void _StartCharger(void)
{
	if(_IsLoadConnected() == ON)  // if already on
		return;  // get out, nothing to do here

	_ConnectLoad(ON);  // connect the load

	EventHandler->Notify(EVENT03);	// notify event

	_FlagStopChargerCalled = 0;  // reset flag
}

// stops the peripheral
static inline void _StopCharger(void)
{
	if(_IsLoadConnected() == OFF && _FlagStopChargerCalled == 1)  // if already off and not called before
		return;

	EventHandler->Notify(EVENT03);	// notify event

	_ConnectLoad(OFF);  // disconnect the load

	_FlagStopChargerCalled = 1;  // mark as called
}

// SW1 NC - 0 - open, 1 - closed
static inline void _SSR_SW1(const uint8_t state)
{  // TODO - validate _SSR_SW1 operation
	if(state)
		HAL_GPIO_WritePin(SW1_CTRL_GPIO_Port, SW1_CTRL_Pin, GPIO_PIN_SET);	// switch closed (NC type switch - default)
	else
		HAL_GPIO_WritePin(SW1_CTRL_GPIO_Port, SW1_CTRL_Pin, GPIO_PIN_RESET);	// switch open

	__AutoCharge.__SW1 = state;
}

// SW2 NC - 0 - open, 1 - closed
static inline void _SSR_SW2(const uint8_t state)
{  // TODO - validate _SSR_SW2 operation
	if(state)
		HAL_GPIO_WritePin(SW2_CTRL_GPIO_Port, SW2_CTRL_Pin, GPIO_PIN_RESET);	// switch closed (NC type switch - default)
	else
		HAL_GPIO_WritePin(SW2_CTRL_GPIO_Port, SW2_CTRL_Pin, GPIO_PIN_SET);	// switch open

	__AutoCharge.__SW2 = state;
}

// SW-CA NO - 0 - open, 1 - closed
static inline void _SSR_SW_CA(const uint8_t state)
{  // TODO - validate _SSR_SW_CA operation
	if(state)
		HAL_GPIO_WritePin(SW_CA_CTRL_GPIO_Port, SW_CA_CTRL_Pin, GPIO_PIN_SET);	// switch closed
	else
		HAL_GPIO_WritePin(SW_CA_CTRL_GPIO_Port, SW_CA_CTRL_Pin, GPIO_PIN_RESET);	// switch open (NO type switch - default)

	__AutoCharge.__SW_CA = state;
}

// SW-CB NO - 0 - open, 1 - closed
static inline void _SSR_SW_CB(const uint8_t state)
{  // TODO - validate _SSR_SW_CB operation
	if(state)
		HAL_GPIO_WritePin(SW_CB_CTRL_GPIO_Port, SW_CB_CTRL_Pin, GPIO_PIN_SET);	// switch closed
	else
		HAL_GPIO_WritePin(SW_CB_CTRL_GPIO_Port, SW_CB_CTRL_Pin, GPIO_PIN_RESET);	// switch open (NO type switch - default)

	__AutoCharge.__SW_CB = state;
}

// SW-CC NO - 0 - open, 1 - closed
static inline void _SSR_SW_CC(const uint8_t state)
{  // TODO - validate _SSR_SW_CC operation
	if(state)
		HAL_GPIO_WritePin(SW_CC_CTRL_GPIO_Port, SW_CC_CTRL_Pin, GPIO_PIN_SET);	// switch closed
	else
		HAL_GPIO_WritePin(SW_CC_CTRL_GPIO_Port, SW_CC_CTRL_Pin, GPIO_PIN_RESET);	// switch open (NO type switch - default)

	__AutoCharge.__SW_CC = state;
}

// SW9 NO - 0 - open, 1 - closed
static inline void _SSR_SW9(const uint8_t state)
{  // TODO - validate _SSR_SW9 operation
	if(state)
		HAL_GPIO_WritePin(SW9_CTRL_GPIO_Port, SW9_CTRL_Pin, GPIO_PIN_SET);	// switch closed
	else
		HAL_GPIO_WritePin(SW9_CTRL_GPIO_Port, SW9_CTRL_Pin, GPIO_PIN_RESET);	// switch open (NO type switch - default)

	__AutoCharge.__SW9 = state;
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

	if(Device->AutoDrive->GetSpeed_mps() > 2)  // high enough speed - load is connected
		_SSR_SW1(ON);
	else
		_SSR_SW1(OFF);

	if(Device->AutoDrive->GetSpeed_mps() > 4)  // high enough speed - load is connected
		_SSR_SW2(ON);
	else
		_SSR_SW2(OFF);

	if(Device->AutoDrive->GetSpeed_mps() > 6)  // high enough speed - load is connected
		_SSR_SW_CA(ON);
	else
		_SSR_SW_CA(OFF);

	if(Device->AutoDrive->GetSpeed_mps() > 8)  // high enough speed - load is connected
		_SSR_SW_CB(ON);
	else
		_SSR_SW_CB(OFF);

	if(Device->AutoDrive->GetSpeed_mps() > 10)  // high enough speed - load is connected
		_SSR_SW_CC(ON);
	else
		_SSR_SW_CC(OFF);

	if(Device->AutoDrive->GetSpeed_mps() > 12)  // high enough speed - load is connected
		_SSR_SW9(ON);
	else
		_SSR_SW9(OFF);

}

static __autocharge_t __AutoCharge =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.IsLoadConnected = &_IsLoadConnected,	// set function pointer
	.public.Do = &_Do  // ditto
	};

autocharge_t* autocharge_ctor(void)  //
{
//	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_RESET);	// explicitly again for clarity - start with load disconnected !!!

	__AutoCharge.__LoadSwitch = HAL_GPIO_ReadPin(LoadFet_GPIO_Port, LoadFet_Pin);  // read out initial switch states
	__AutoCharge.__SW1 = HAL_GPIO_ReadPin(SW1_CTRL_GPIO_Port, SW1_CTRL_Pin);	// ditto
	__AutoCharge.__SW2 = HAL_GPIO_ReadPin(SW2_CTRL_GPIO_Port, SW2_CTRL_Pin);
	__AutoCharge.__SW_CA = HAL_GPIO_ReadPin(SW_CA_CTRL_GPIO_Port, SW_CA_CTRL_Pin);
	__AutoCharge.__SW_CB = HAL_GPIO_ReadPin(SW_CB_CTRL_GPIO_Port, SW_CB_CTRL_Pin);
	__AutoCharge.__SW_CC = HAL_GPIO_ReadPin(SW_CC_CTRL_GPIO_Port, SW_CC_CTRL_Pin);
	__AutoCharge.__SW9 = HAL_GPIO_ReadPin(SW9_CTRL_GPIO_Port, SW9_CTRL_Pin);

	return &__AutoCharge.public;  // set pointer to AutoCharge public part
}

#endif
