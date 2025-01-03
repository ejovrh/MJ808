#ifndef CORE_INC_MJ818_MJ818_H_
#define CORE_INC_MJ818_MJ818_H_

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
		uint8_t _2 :1;  // _2 //
		uint8_t _3 :1;  // _3 //
		uint8_t BrakeLightOn :1;  // BRK // brake light is on: PWM - stop mode will break functionality
		uint8_t RearLightOn :1;  // Rear // rear light is on: PWM - stop mode will break functionality

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj818_activity_t;

#include "main.h"
#if defined(MJ818_)	// if this particular device is active
#define USE_EVENTHANDLER 0	// shall EventHandler code be included

#define BRAKELIGHT 4
#define REARLIGHT 5

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER14_PERIOD 49	// LED handling - 20ms
#define TIMER16_PERIOD 249	// button handling - 25ms
#define TIMER17_PERIOD 24	// event handling - 2.5ms

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "event\event.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_5
#define TCAN334_Standby_GPIO_Port GPIOA
#define BrakeLED_Pin GPIO_PIN_6
#define BrakeLED_GPIO_Port GPIOA
#define RearLED_Pin GPIO_PIN_15
#define RearLED_GPIO_Port GPIOA
// definitions of device/PCB layout-dependent hardware pins

#define REAR_LIGHT_CCR htim2.Instance->CCR1	// Output Compare Register for PWM of rear light
#define BRAKE_LIGHT_CCR htim3.Instance->CCR1	// Output Compare Register for PWM of brake light

enum mj818_leds  // enum of lights on this device
{
	  Rear,
	  Brake
};

typedef struct	// struct describing devices on MJ818
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj818_activity_t *activity;  // pointer to struct(union) indicating device activity status
	composite_led_t *led;  // pointer to LED structure

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj818_t;

void mj818_ctor();	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj818_t *const Device;  // declare pointer to public struct part

#endif // MJ818_

#endif /* CORE_INC_MJ818_MJ818_H_ */
