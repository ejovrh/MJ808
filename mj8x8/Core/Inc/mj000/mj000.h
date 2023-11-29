#ifndef CORE_INC_MJ000_MJ000_H_
#define CORE_INC_MJ000_MJ000_H_

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		/*  0x3F - if any of bits 0 though 5 are set - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus on
		 * additionally: if CANBUS_ACTIVE_MASK has bits not set, CANbus will be off
		 */
		uint8_t DoHeartbeat :1;  // bit 0 - HeartBeat is running
		uint8_t CANActive :1;  // CAN is actively being used *is used as a flag to avoid re-entering e.g. __can_go_into_active_mode()

		// 0x3C - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus off
		// TODO - mj000 - activity bit defines

		uint8_t _2 :1;  //
		uint8_t _3 :1;  //
		uint8_t _4 :1;  //
		uint8_t _5 :1;  //

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;	//
		uint8_t _7 :1;  // bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj000_activity_t;

#include "main.h"
#if defined(MJ000_)	// if this particular device is active
#define CANID_SELF CANID_MJ000

// activity bit pos
// TODO - mj000 - activity bit defines

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate

#include "mj8x8\mj8x8.h"

// definitions of device/PCB layout-specific hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_15
#define TCAN334_Standby_GPIO_Port GPIOA
// TODO - mj000 - gpio defines

// definitions of device/PCB layout-specific hardware pins

typedef struct	// struct describing devices on MJ000
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj000_activity_t *activity;  // pointer to struct(union) indicating device activity status

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj000_t;

void mj000_ctor();	// declare constructor for concrete class

extern mj000_t *const Device;  // declare pointer to public struct part

#endif // MJ000_

#endif /* CORE_INC_MJ000_MJ000_H_ */
