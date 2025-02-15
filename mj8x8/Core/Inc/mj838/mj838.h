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
		uint8_t ZeroCross :1;  // ZC // zero-cross signal present or not
		uint8_t _3 :1;  // _3 //
		uint8_t AutoCharge :1;	// AC // AutoCharge is operating
		uint8_t AutoDrive :1;  // AD // AutoDrive is active

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj838_activity_t;

#include "main.h"
#if defined(MJ838_)	// if this particular device is active
#define WIPE_FRAM 0	// if activated, statistics will be zeroed out in EXTI0 ISR

#define USE_I2C 1	// use I2C
#define USE_FERAM 1 // use FERAM
#define USE_SHT40 1 // use humidity sensor
#define USE_PAC1952 1 // use Power Monitor
#define USE_TLC59208 1 // use LED driver for SSRs
#define USE_DAC121C081 1 // use USE_DAC121C081 ADC for adjustable 12R load

#define USE_EVENTHANDLER 1	// shall EventHandler code be included

#define USE_APPLICATION_LOAD 1	// use application load switch (not the 12R adjustable load)
#define USE_ADJUSTABLE_LOAD 0	// use 12R adjustable load switch (not the application load switch)

#define ZEROCROSS 2
//#define	MOTION 3
#define AUTOCHARGE 4
#define AUTODRIVE 5

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD 2499 // periodic frequency measurement of timer3 data - default 250ms
#define TIMER3_PERIOD	0xFFFFFFFF // input capture of zero-cross signal on rising edge
#define TIMER14_PERIOD 99	// power measurement time base - 10ms
#define TIMER16_PERIOD 9999	// odometer & co. - 1s
#define TIMER17_PERIOD 24	// event handling - 2.5ms
#define TIMER3_IC_FILTER 0xF	// with TIM_ICPSC_DIV8 and 0xF the pulse needs to be at least 35us wide

#define F_CPU 8000000	// 8MHz
#define SCALED_CPU_TICK (uint16_t)(F_CPU / (TIMER_PRESCALER + 1)) // re-compute scaled down CPU freq. due to prescaler

//#define WHEEL_CIRCUMFERENCE 1.945f	// red 26" training wheel circumference in meters
#define WHEEL_CIRCUMFERENCE 2.350f	// Schwalbe Marathon Mondial 29x2.25"
//#define WHEEL_CIRCUMFERENCE 2.342f	// Schwalbe Jumbo Jim 26x4", in meters
//#define WHEEL_CIRCUMFERENCE 2.095f	// Marathon Mondial 26x2", in meters

// FeRAM memory addresses
#define ODOMETER_ADDR	0x0000, 4	// 4 bytes for odometer float
#define NEXT_FREE_ADDR 0x0004, 2	//

#define POLE_COUNT 13	// number of dynamo pole pairs
// 5 minutes #define SLEEPTIMEOUT_COUNTER 1200 // N * 0.25s = foo seconds - time to stay idle, then stop zero-cross
#define SLEEPTIMEOUT_COUNTER 20 // N * 0.25s = foo seconds - time to stay idle, then stop zero-cross

#include "mj8x8\mj8x8.h"
#include "button\button.h"
#include "mb85rc\mb85rc.h"	// 16kB FeRAM
#include "sht40\sht40.h" // SHT40 humidity sensor

#include "zerocross\zerocross.h"
#include "mj838\autodrive.h"
#include "mj838\autocharge.h"

// TODO - mj838 debug GPIO - remove once debugging is complete
#define SIGNAL_GENERATOR_INPUT 1	// ZeroCross signal input is signal generator output
#define GPIO_DEBUG_OUT 1

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_15	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c

// TODO - remove once HW rev.1a is there
//#define NEW_GPIO_LAYOUT // use HW rev. 1b GPIO layout

#ifdef NEW_GPIO_LAYOUT
#define PowerMonitorPower_Pin GPIO_PIN_8	// low - off; high - on
#define PowerMonitorPower_GPIO_Port GPIOB

#define LED1_Pin GPIO_PIN_5	// dual colour LED pin 2
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_6	// dual colour LED pin 2
#define LED2_GPIO_Port GPIOA

#define LED_Reset_Pin GPIO_PIN_0	// SSR LED Driver reset: low - in reset/standby; high - active
#define LED_Reset_GPIO_Port GPIOA

#define ZeroCross_Pin GPIO_PIN_0	// ZeroCross signal in
#define ZeroCross_GPIO_Port GPIOB

#define AppLoadFet_Pin GPIO_PIN_1	// Application Load Switch
#define AppLoadFet_GPIO_Port GPIOA

#define I2C_SDA_Pin GPIO_PIN_0 // see i2c_ctor()
#define I2C_SCL_Pin GPIO_PIN_1// see i2c_ctor()
#define I2C_GPIO_Port GPIOF

#if GPIO_DEBUG_OUT
#define YellowTestPad_Pin GPIO_PIN_6// debug pin 0
#define YellowTestPad_GPIO_Port GPIOB
#define BlueTestPad_Pin GPIO_PIN_7 // debug pin 1
#define BlueTestPad_GPIO_Port GPIOB
#endif

//#define PA5_Pin GPIO_PIN_5	// general GPIO
//#define PA5_GPIO_Port GPIOA
//#define PA6_Pin GPIO_PIN_6	// general GPIO
//#define PA6_GPIO_Port GPIOA
#define PA7_Pin GPIO_PIN_7	// general GPIO
#define PA7_GPIO_Port GPIOA

#define PB3_Pin GPIO_PIN_3	// general GPIO
#define PB3_GPIO_Port GPIOB
#define PB4_Pin GPIO_PIN_4	// general GPIO
#define PB4_GPIO_Port GPIOB
#define PB5_Pin GPIO_PIN_5	// general GPIO
#define PB5_GPIO_Port GPIOB
#define PB6_Pin GPIO_PIN_6	// general GPIO
#define PB6_GPIO_Port GPIOB
#define PB7_Pin GPIO_PIN_7	// general GPIO
#define PB7_GPIO_Port GPIOB
#else
#define PowerMonitorPower_Pin GPIO_PIN_0	// low - off; high - on
#define PowerMonitorPower_GPIO_Port GPIOA

#define LED1_Pin GPIO_PIN_6	// dual colour LED pin 2
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_3	// dual colour LED pin 2
#define LED2_GPIO_Port GPIOB

#define LED_Reset_Pin GPIO_PIN_7	// SSR LED Driver reset: low - in reset/standby; high - active
#define LED_Reset_GPIO_Port GPIOA

#define ZeroCross_Pin GPIO_PIN_0	// ZeroCross signal in
#define ZeroCross_GPIO_Port GPIOB
#define AppLoadFet_Pin GPIO_PIN_1	// Load Switch
#define AppLoadFet_GPIO_Port GPIOB

#define I2C_SDA_Pin GPIO_PIN_0 // see i2c_ctor()
#define I2C_SCL_Pin GPIO_PIN_1// see i2c_ctor()
#define I2C_GPIO_Port GPIOF

#if GPIO_DEBUG_OUT
#define YellowTestPad_Pin GPIO_PIN_1// debug pin 0
#define YellowTestPad_GPIO_Port GPIOA
#define BlueTestPad_Pin GPIO_PIN_8 // debug pin 1
#define BlueTestPad_GPIO_Port GPIOB
#endif

#define PA2_Pin GPIO_PIN_2	// general GPIO
#define PA2_GPIO_Port GPIOA
#define PA3_Pin GPIO_PIN_3	// general GPIO
#define PA3_GPIO_Port GPIOA
#define PA4_Pin GPIO_PIN_4	// general GPIO
#define PA4_GPIO_Port GPIOA
#define PA5_Pin GPIO_PIN_5	// general GPIO
#define PA5_GPIO_Port GPIOA
#define PB4_Pin GPIO_PIN_4	// general GPIO
#define PB4_GPIO_Port GPIOB
#define PB5_Pin GPIO_PIN_5	// general GPIO
#define PB5_GPIO_Port GPIOB
#define PB6_Pin GPIO_PIN_6	// general GPIO
#define PB6_GPIO_Port GPIOB
#define PB7_Pin GPIO_PIN_7	// general GPIO
#define PB7_GPIO_Port GPIOB
#endif

// definitions of device/PCB layout-dependent hardware pins

typedef struct	// struct describing devices on MJ838
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj838_activity_t *activity;  // pointer to struct(union) indicating device activity status
	zerocross_t *ZeroCross;  // zero-cross object
	autodrive_t *AutoDrive;  // automatic drive handling feature
	autocharge_t *AutoCharge;  // automatic charger
	mb85rc_t *FeRAM;	// pointer to FeRAM object
	sht40_t *Humidity;  // pointer to humidity sensor object

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj838_t;

void mj838_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj838_t *const Device;  // declare pointer to public struct part

#endif // MJ838_

#endif /* CORE_INC_MJ838_MJ838_H_ */
