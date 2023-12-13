#ifndef CORE_INC_mj514_mj514_H_
#define CORE_INC_mj514_mj514_H_

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		/*  0x3F - if any of bits 0 though 5 are set - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus on
		 * additionally: if CANBUS_ACTIVE_MASK has bits not set, CANbus will be off
		 */
		// bit0
		uint8_t DoHeartbeat :1;  // DoHB // HeartBeat is running
		uint8_t CANActive :1;  // CAN // CAN is actively being used *is used as a flag to avoid re-entering e.g. __can_go_into_active_mode()

		// 0x3C - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus off
		uint8_t _2 :1;  // act2 // FIXME - describe activity
		uint8_t _3 :1;  // act3 //
		uint8_t _4 :1;  // act4 //
		uint8_t _5 :1;  // act5 //

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj514_activity_t;

#include "main.h"
#if defined(MJ514_)	// if this particular device is active
// FIXME - define if eventhandler is used
#define USE_EVENTHANDLER 0	// shall EventHandler code be included

// FIXME - define activity bit postions

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
// FIXME - define timer defines

#include "mj8x8\mj8x8.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_5	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c
// FIXME - define GPIOs
// definitions of device/PCB layout-dependent hardware pins

typedef struct	// struct describing devices on mj514
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj514_activity_t *activity;  // pointer to struct(union) indicating device activity status

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj514_t;

void mj514_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj514_t *const Device;  // declare pointer to public struct part

#endif // mj514_

#endif /* CORE_INC_mj514_mj514_H_ */
