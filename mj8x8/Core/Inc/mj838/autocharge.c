#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autocharge.h"

#include "tlc59208/tlc59208.h"

#define SET_BIT_VAL(word, n, val) \
    ((word) = ((word) & (uint16_t)(~((uint16_t)1 << (n)))) | \
              ((uint16_t)(((val) & 0x01U) << (n))))

#define SW_CA 14 	// LED 3	-	ledout0	-	0xC - 0x40 -- MSB
#define SW_CB 12 // LED 2	-	ledout0	-	0xC - 0x10
#define SW_CC 10	// LED 1	-	ledout0	-	0xC - 0x04
#define SW_CD 8	// LED 0	-	ledout0	-	0xC - 0x01
#define SW1 2	// LED 5	- ledout1 - 0xD - 0x04
#define SW2 0	// LED 4	-	ledout1	-	0xD - 0x01 -- LSB

typedef enum   // enum of lights on this device
{
	  SpeedStopped,  // stopped
	  SpeedbelowChargerThres,  // below charger threshold
	  SpeedaboveChargerThres,  // above charger threshold
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

	autocharge_speedlevels_t _SpeedLevel;  // current speed level
	autocharge_speedlevels_t _previousSpeedLevel;  // previous speed level

	// 0 - switch open, 1 - switch closed -- regardless of NC or NO
	uint8_t __SW1 :1;  // SSR SW1 state (NC) - 0 open, 1 closed
	uint8_t __SW2 :1;  // SSR SW2 state (NC) - 0 open, 1 closed
	uint8_t __SW_CA :1;  // SSR SW-CA state (NO) - 0 open, 1 closed
	uint8_t __SW_CB :1;  // SSR SW-CB state (NO) - 0 open, 1 closed
	uint8_t __SW_CC :1;  // SSR SW-CC state (NO) - 0 open, 1 closed
	uint8_t __SW_CD :1;  // SSR SW-D state (NO) - 0 open, 1 closed
//	uint8_t __SW_X :1;  // SSR SW-CD state (NO) - 0 open, 1 closed

	uint8_t __LoadSwitch :1;  // High-Side Load Switch state - 0 open, 1 closed

	tlc59208_t *_LEDDriver;  // pointer to TLC59208 object
} __autocharge_t;

static __autocharge_t __AutoCharge __attribute__ ((section (".data")));  // preallocate __AutoCharge object in .data

static uint16_t _LEDword = 0;

// returns High-Side load switch state: 0 - disconnected, 1 - connected
static inline uint8_t _IsLoadConnected(void)
{
	return HAL_GPIO_ReadPin(LoadFet_GPIO_Port, LoadFet_Pin);	// explicitly return load switch hardware state
}

// connect the load via a high-side load switch
static inline void _ConnectLoad(const uint8_t state)
{
	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, state);  // drive the n-fet gate
	__AutoCharge.__LoadSwitch = (unsigned char) (state & 0x01);
	Device->mj8x8->UpdateActivity(AUTOCHARGE, state);  // update the bus
}

// starts/stops the peripheral
static inline void _SetChargerState(uint8_t state)
{
	if(_IsLoadConnected() == state)  // if already in the desired state
		return;  // get out, nothing to do here

	_ConnectLoad(state);  // set the load state

	EventHandler->Notify(EVENT03);  // notify event
}

// SW1 control, NC
static inline void _SSR_SW1(const uint8_t state)
{
	SET_BIT_VAL(_LEDword, SW1, state);  // set the bit in valD
	__AutoCharge.__SW1 = (unsigned char) (state & 0x01);
}

// SW2 control, NC
static inline void _SSR_SW2(const uint8_t state)
{
	SET_BIT_VAL(_LEDword, SW2, state);  // set the bit in valD
	__AutoCharge.__SW2 = (unsigned char) (state & 0x01);
}

// SW-CA control, NO
static inline void _SSR_SW_CA(const uint8_t state)
{
	SET_BIT_VAL(_LEDword, SW_CA, state);  // set the bit in valC
	__AutoCharge.__SW_CA = (unsigned char) (state & 0x01);
}

// SW-CB control, NO
static inline void _SSR_SW_CB(const uint8_t state)
{
	SET_BIT_VAL(_LEDword, SW_CB, state);  // set the bit in valC
	__AutoCharge.__SW_CB = (unsigned char) (state & 0x01);
}

// SW-CC control, NO
static inline void _SSR_SW_CC(const uint8_t state)
{
	SET_BIT_VAL(_LEDword, SW_CC, state);  // set the bit in valC
	__AutoCharge.__SW_CC = (unsigned char) (state & 0x01);
}

// SW-CD control, NO
static inline void _SSR_SW_CD(const uint8_t state)
{
	SET_BIT_VAL(_LEDword, SW_CD, state);  // set the bit in valC
	__AutoCharge.__SW_CD = (unsigned char) (state & 0x01);
}

// compares current and previous speed levels and sets speed level change flag
static inline uint8_t _CompareSpeedLevelsandFlag(const autocharge_speedlevels_t in_level)
{
	if(__AutoCharge._previousSpeedLevel == in_level)  // if speed level was not changed
		return 0;  // do not enable notification flag (see very first if-clause in Do())

	__AutoCharge._previousSpeedLevel = __AutoCharge._SpeedLevel;	// save current speed level
	__AutoCharge._SpeedLevel = in_level;  // set new light level
	return 1;  // enable notification flag (see calling if-clause in Do())
}

// AutoCharge functionality - called by timer 3 ISR - usually every 250ms
static void _Do(void)  // this actually runs the AutoCharge application
{
	// set speed flags on each measurement
	float speed = Device->AutoDrive->GetSpeed_mps();
	if(speed > 12)
		__AutoCharge._SpeedLevel = Speed12;
	else if(speed > 10)
		__AutoCharge._SpeedLevel = Speed10;
	else if(speed > 8)
		__AutoCharge._SpeedLevel = Speed8;
	else if(speed > 6)
		__AutoCharge._SpeedLevel = Speed6;
	else if(speed > 4)
		__AutoCharge._SpeedLevel = Speed4;
	else if(speed > 2)
		__AutoCharge._SpeedLevel = Speed2;
	else if(speed > 0.1)
		__AutoCharge._SpeedLevel = SpeedbelowChargerThres;
	else
		__AutoCharge._SpeedLevel = SpeedStopped;

	if(_CompareSpeedLevelsandFlag(__AutoCharge._SpeedLevel))	// if the speed flags have changed
		{
			__AutoCharge._LEDDriver->Power(__AutoCharge._SpeedLevel);  // if movement, then turn on LED Driver

			if(__AutoCharge._SpeedLevel <= SpeedbelowChargerThres)  // low speed - load is disconnected
				_SetChargerState(OFF);  // stop the charger
			else
				// high speed - load is connected
				_SetChargerState(ON);  // start the charger

			// SSR control based on speed flags - logic will change
			_SSR_SW_CD(__AutoCharge._SpeedLevel == Speed12 ? ON : OFF);  // NO to NC
			_SSR_SW_CC(__AutoCharge._SpeedLevel == Speed10 ? ON : OFF);  // NO to NC
			_SSR_SW_CB(__AutoCharge._SpeedLevel == Speed8 ? ON : OFF);  // NO to NC
			_SSR_SW_CA(__AutoCharge._SpeedLevel == Speed6 ? ON : OFF);	// NO to NC
			// FIXME - on rev.1 board SW2 is broken: replace IC
			_SSR_SW2(__AutoCharge._SpeedLevel == Speed4 ? ON : OFF);  // NC to NO
			_SSR_SW1(__AutoCharge._SpeedLevel == Speed2 ? ON : OFF);  // NC to NO

			__AutoCharge._LEDDriver->Write(0x0C, &_LEDword);  // write LED state to driver
		}
}

static __autocharge_t __AutoCharge =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.IsLoadConnected = &_IsLoadConnected,	// set function pointer
	.public.Do = &_Do  // ditto
	};

autocharge_t* autocharge_ctor(void)  //
{
	__AutoCharge._LEDDriver = tlc59208_ctor();  // tie in TLC59208 object

	HAL_GPIO_WritePin(LoadFet_GPIO_Port, LoadFet_Pin, GPIO_PIN_RESET);	// load n-fet disconnected
	__AutoCharge.__LoadSwitch = HAL_GPIO_ReadPin(LoadFet_GPIO_Port, LoadFet_Pin);  // read out initial switch states

	return &__AutoCharge.public;  // set pointer to AutoCharge public part
}

#endif
