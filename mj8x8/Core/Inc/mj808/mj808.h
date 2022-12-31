#ifndef CORE_INC_MJ808_MJ808_H_
#define CORE_INC_MJ808_MJ808_H_

#include "main.h"
#if defined(MJ808_)	// if this particular device is active

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Shutdown_Pin GPIO_PIN_4	//	defined here but initialised in mj8x8.c
#define TCAN334_Shutdown_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c
#define Switch_Pin GPIO_PIN_0
#define Switch_GPIO_Port GPIOB
#define Switch_EXTI_IRQn EXTI0_1_IRQn
#define TCAN334_Standby_Pin GPIO_PIN_1	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOB	//	defined here but initialised in mj8x8.c
#define FrontLED_Pin GPIO_PIN_3
#define FrontLED_GPIO_Port GPIOB
#define RedLED_Pin GPIO_PIN_4
#define RedLED_GPIO_Port GPIOB
#define GreenLED_Pin GPIO_PIN_5
#define GreenLED_GPIO_Port GPIOB
// definitions of device/PCB layout-dependent hardware pins

enum mj808_leds  // enum of lights on this device
{
	  Utility,
	  Front
};

enum mj808_buttons	// enum of buttons on this device
{
	  Center
};

typedef struct	// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;  // pointer to the base class
	composite_led_t *led;  // pointer to LED structure
	button_t *button;  // array of button_t - one buttons
} mj808_t;

void mj808_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj808_t *const Device;  // declare pointer to public struct part

#endif // MJ808_

#endif /* CORE_INC_MJ808_MJ808_H_ */
