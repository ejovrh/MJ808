#ifndef CORE_INC_MJ818_MJ818_H_
#define CORE_INC_MJ818_MJ818_H_

#include "main.h"
#if defined(MJ818_)	// if this particular device is active

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "event\event.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_5
#define TCAN334_Standby_GPIO_Port GPIOA
#define BrakeLED_Pin GPIO_PIN_1
#define BrakeLED_GPIO_Port GPIOB
#define RearLED_Pin GPIO_PIN_15
#define RearLED_GPIO_Port GPIOA
#define TCAN334_Shutdown_Pin GPIO_PIN_4
#define TCAN334_Shutdown_GPIO_Port GPIOB
// definitions of device/PCB layout-dependent hardware pins

enum mj818_leds  // enum of lights on this device
{
	  Rear,
	  Brake
};

typedef struct	// struct describing devices on MJ818
{
	mj8x8_t *mj8x8;  // pointer to the base class
	composite_led_t *led;  // pointer to LED structure

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj818_t;

void mj818_ctor();	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj818_t *const Device;  // declare pointer to public struct part

#endif // MJ818_

#endif /* CORE_INC_MJ818_MJ818_H_ */
