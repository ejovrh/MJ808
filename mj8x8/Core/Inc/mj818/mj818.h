#ifndef CORE_INC_MJ818_MJ818_H_
#define CORE_INC_MJ818_MJ818_H_

#include "main.h"
#if defined(MJ818_)	// if this particular device is active

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "event\event.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Shutdown_Pin GPIO_PIN_4
#define TCAN334_Shutdown_GPIO_Port GPIOB
#define TCAN334_Standby_Pin GPIO_PIN_5
#define TCAN334_Standby_GPIO_Port GPIOA
#define BrakeLED_Pin GPIO_PIN_1
#define BrakeLED_GPIO_Port GPIOB
#define RearLED_Pin GPIO_PIN_15
#define RearLED_GPIO_Port GPIOA
// definitions of device/PCB layout-dependent hardware pins

enum mj818_leds  // enum of lights on this device
{
	  Rear,
	  Brake
};

typedef union  // union for activity indication
{
	struct
	{
		// CAN has to be active - 0x0F - lower nibble
		uint8_t HeartBeatRunning :1;  // bit 0 - HeartBeat is running
		uint8_t one :1;  // bit 1 -
		uint8_t two :1;  // bit 2 -
		uint8_t three :1;  // bit 3 -

		// CAN can be in standby mode - 0xF0 - upper nibble
		uint8_t four :1;	// bit 4 -
		uint8_t five :1;	// bit 5 -
		uint8_t BrakeLightOn :1;  // bit 6 - brake light is on
		uint8_t RearLightOn :1;  // bit 7 - rear light is on
	};
	uint8_t uactivity;  // byte-wise representation of the above bitfield
} mj818_activity_t;

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
