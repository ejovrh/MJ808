#ifndef CORE_INC_MJ838_MJ838_H_
#define CORE_INC_MJ838_MJ838_H_

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
		uint8_t Motion :1;  // M // Motion detected
		uint8_t AutoCharge :1;	// AC // AutoCharge is operating
		uint8_t _5 :1;  // _5 //

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj838_activity_t;

#include "main.h"
#if defined(MJ838_)	// if this particular device is active
#define USE_EVENTHANDLER 1	// shall EventHandler code be included

#define ZEROCROSS 2
#define	MOTION 3
#define AUTOCHARGE 4

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD	0xFFFFFFFF // ZeroCross frequency measurement (rollover every 119 hours of constant use...)
#define TIMER3_PERIOD 2499 // by default 250ms
#define TIMER17_PERIOD 24	// event handling - 2.5ms
#define TIMER2_IC_FILTER 0xF	// with TIM_ICPSC_DIV8 and 0xF the pulse needs to be at least 35us wide

#define WHEEL_CIRCUMFERENCE 1.945	// red training wheel circumference in meters
#define POLE_COUNT 13	// number of dynamo pole pairs
#define SLEEPTIMEOUT_COUNTER 4 // N * 0.25s = foo seconds - time to stay idle, then stop zero-cross

#include "mj8x8\mj8x8.h"
#include "button\button.h"

#include "zerocross\zerocross.h"
#include "motion\motion.h"
#include "mj838\autodrive.h"
#include "mj838\autocharge.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_15	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c
#define ZeroCross_Pin GPIO_PIN_0	// ZeroCross signal in -- GnZC pin 1
#define ZeroCross_GPIO_Port GPIOA
#define AutoMotion_Pin GPIO_PIN_0	// TODO - mj838 - implement AutoMotion via accelerometer
#define AutoMotion_GPIO_Port GPIOF	// TODO - mj838 - give motion detection a proper pin & port
#define SW1_CTRL_Pin	GPIO_PIN_1	// solid-state relay NC -- GnZC pin 2
#define SW1_CTRL_GPIO_Port GPIOA
#define SW2_CTRL_Pin	GPIO_PIN_2	// solid-state relay NC -- GnZC pin 3
#define SW2_CTRL_GPIO_Port GPIOA
#define SW_CA_CTRL_Pin	GPIO_PIN_3	// solid-state relay NO -- GnZC pin 4
#define SW_CA_CTRL_GPIO_Port GPIOA
#define SW_CB_CTRL_Pin	GPIO_PIN_4	// solid-state relay NO -- GnZC pin 5
#define SW_CB_CTRL_GPIO_Port GPIOA
#define SW_CC_CTRL_Pin	GPIO_PIN_5	// solid-state relay NO -- GnZC pin 6
#define SW_CC_CTRL_GPIO_Port GPIOA
#define SW_D_CTRL_Pin	GPIO_PIN_6	// solid-state relay NO -- GnZC pin 7
#define SW_D_CTRL_GPIO_Port GPIOA
//#define SW_X_CTRL_Pin	GPIO_PIN_6	// solid-state relay NO -- GnZC pin 8
//#define SW_X_CTRL_GPIO_Port GPIOA
#define LoadFet_Pin GPIO_PIN_7	// Load N-Fet Gate -- GnZC pin 11
#define LoadFet_GPIO_Port GPIOA

// TODO - mj838 debug GPIO - remove once debugging is complete
#define SIGNAL_GENERATOR_INPUT 1	// ZeroCross signal input is signal generator output
#define GPIO_DEBUG_OUT 1
# if GPIO_DEBUG_OUT
#define DEBUG0_Pin GPIO_PIN_0	// HAL_GPIO_TogglePin(DEBUG0_Port, DEBUG0_Pin);
#define DEBUG0_Port GPIOB
#define DEBUG1_Pin GPIO_PIN_1	// HAL_GPIO_TogglePin(DEBUG1_Port, DEBUG1_Pin);
#define DEBUG1_Port GPIOB
#endif

// definitions of device/PCB layout-dependent hardware pins

typedef struct	// struct describing devices on MJ838
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj838_activity_t *activity;  // pointer to struct(union) indicating device activity status
	zerocross_t *ZeroCross;  // zero-cross object
	autodrive_t *AutoDrive;  // automatic drive handling feature
	motion_t *Motion;  // motion detection
	autocharge_t *AutoCharge;  // automatic charger

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj838_t;

void mj838_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj838_t *const Device;  // declare pointer to public struct part

#endif // MJ838_

#endif /* CORE_INC_MJ838_MJ838_H_ */
