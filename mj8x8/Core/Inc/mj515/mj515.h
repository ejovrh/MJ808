#ifndef CORE_INC_MJ515_MJ515_H_
#define CORE_INC_MJ515_MJ515_H_

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
		uint8_t _2 :1;  // act2 //
		uint8_t _3 :1;  // act3 //
		uint8_t _4 :1;  // act4 //
		uint8_t _5 :1;  // act5 //

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj515_activity_t;

#include "main.h"
#if defined(MJ515_)	// if this particular device is active
#define USE_I2C 0	// use I2C
#define USE_EVENTHANDLER 0	// shall EventHandler code be included -- timer17 is used up for ADC timebase !!!

// activity bit positions
#define SHIFTING 2	// TODO

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
//#define TIMER2_PERIOD 0x100	// FIXME - find proper value
//#define TIMER3_PERIOD  0xFFFF	// use max. range for rotary encoder pulse count
//#define TIMER3_IC1_FILTER	0x0	// TODO - figure out filter
//#define TIMER3_IC2_FILTER 0x0	// TODO - figure out filter
//#define TIMER16_PERIOD 499	// TODO - verify proper timing: 10ms or 100ms may be better - rotary encoder time base - 50ms
//#define TIMER17_PERIOD 99	// ADC time base - 10ms

#include "mj8x8/mj8x8.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_5	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c

#define Debug_Pin GPIO_PIN_1	// TODO - remove debug pin when done
#define Debug_GPIO_Port GPIOF
// definitions of device/PCB layout-dependent hardware pins

typedef struct	// struct describing devices on mj515
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj515_activity_t *activity;  // pointer to struct(union) indicating device activity status

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj515_t;

void mj515_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj515_t *const Device;  // declare pointer to public struct part

#endif // mj515_

#endif /* CORE_INC_MJ515_MJ515_H_ */
