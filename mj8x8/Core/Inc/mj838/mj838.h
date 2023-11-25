#ifndef CORE_INC_MJ838_MJ838_H_
#define CORE_INC_MJ838_MJ838_H_

// TODO - mj838 - determine max. zero-cross frequency for speeds up to 75 km/h

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		/*  0x3F - if any of bits 0 though 5 are set - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus on
		 * additionally: if CANBUS_ACTIVE_MASK has bits not set, CANbus will be off
		 */
		// bit 0
		uint8_t DoHeartbeat :1;  // DoHB // HeartBeat is running
		uint8_t CANActive :1;  // CAN // CAN is actively being used *is used as a flag to avoid re-entering e.g. __can_go_into_active_mode()

		// 0x3C - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus off
		uint8_t ZeroCross :1;  // ZC // zero-cross detection is active/inactive
		uint8_t AutoDrive :1;  // AD // AutoDrive functionality is on/off
		uint8_t _4 :1;  // _4 //
		uint8_t _5 :1;	// _5 //

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj838_activity_t;

#include "main.h"
#if defined(MJ838_)	// if this particular device is active
#define CANID_SELF CANID_MJ838

#define ZEROCROSS 2
#define AUTODRIVE 3

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD	4294967295 // ZeroCross frequency measurement (rollover every 119 hours of constant use...)
#define TIMER3_PERIOD 2499 // by default 250ms
#define TIMER17_PERIOD 24	// event handling - 2.5ms

#define ZC_BUFFER_LEN 2
#define WHEEL_CIRCUMFERENCE 1.945	// red training wheel circumference in meters
#define SON_DYNAMO_CORRECTION 6	// the dynamo has 26 poles; for some reason its frequency has to be divided by 6 to get the wheel frequency
#define SLEEPTIMEOUT_COUNTER 8	// 120 * 0.25s = 30s - time to stay idle, then stop zero-cross

#include "mj8x8\mj8x8.h"
#include "button\button.h"

#include "zerocross\zerocross.h"
#include "mj838\autodrive.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_15	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c
#define ZeroCross_Pin GPIO_PIN_0
#define ZeroCross_GPIO_Port GPIOA
#define SW1_CTRL_Pin	GPIO_PIN_2	// solid-state relay NC
#define SW1_CTRL_GPIO_Port GPIOA
#define SW2_CTRL_Pin	GPIO_PIN_3	// solid-state relay NC
#define SW2_CTRL_GPIO_Port GPIOA
#define SW_CA_CTRL_Pin	GPIO_PIN_4	// solid-state relay NO
#define SW_CA_CTRL_GPIO_Port GPIOA
#define SW_CB_CTRL_Pin	GPIO_PIN_5	// solid-state relay NO
#define SW_CB_CTRL_GPIO_Port GPIOA
#define SW_CC_CTRL_Pin	GPIO_PIN_6	// solid-state relay NO
#define SW_CC_CTRL_GPIO_Port GPIOA
#define SW9_CTRL_Pin	GPIO_PIN_7	// solid-state relay NO
#define SW9_CTRL_GPIO_Port GPIOA

// definitions of device/PCB layout-dependent hardware pins

typedef struct	// struct describing devices on MJ838
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj838_activity_t *activity;  // pointer to struct(union) indicating device activity status
	zerocross_t *ZeroCross;  // zero-cross object
	autodrive_t *AutoDrive;  // automatic drive handling feature

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj838_t;

void mj838_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj838_t *const Device;  // declare pointer to public struct part

#endif // MJ838_

#endif /* CORE_INC_MJ838_MJ838_H_ */
