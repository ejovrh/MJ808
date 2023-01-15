#ifndef CORE_INC_MJ828_MJ828_H_
#define CORE_INC_MJ828_MJ828_H_

#include "main.h"
#if defined(MJ828_)	// if this particular device is active

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"

// definitions of device/PCB layout-specific hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_8
#define TCAN334_Standby_GPIO_Port GPIOB
#define TCAN334_Shutdown_Pin GPIO_PIN_1
#define TCAN334_Shutdown_GPIO_Port GPIOF
#define LeverFront_Pin GPIO_PIN_1
#define LeverFront_GPIO_Port GPIOA
#define LeverFront_EXTI_IRQn EXTI0_1_IRQn
#define LeverBrake_Pin GPIO_PIN_2
#define LeverBrake_GPIO_Port GPIOA
#define LeverBrake_EXTI_IRQn EXTI2_3_IRQn
#define VBat_Pin GPIO_PIN_3
#define VBat_GPIO_Port GPIOA
#define Pushbutton_Pin GPIO_PIN_0
#define Pushbutton_GPIO_Port GPIOB
#define Pushbutton_EXTI_IRQn EXTI0_1_IRQn
#define Phototransistor_Pin GPIO_PIN_1
#define Phototransistor_GPIO_Port GPIOB
#define CP4_Pin GPIO_PIN_15
#define CP4_GPIO_Port GPIOA
#define CP2_Pin GPIO_PIN_3
#define CP2_GPIO_Port GPIOB
#define CP3_Pin GPIO_PIN_4
#define CP3_GPIO_Port GPIOB
#define CP1_Pin GPIO_PIN_5
#define CP1_GPIO_Port GPIOB
// definitions of device/PCB layout-specific hardware pins

enum mj828_leds  // enum of lights on this device
{
	  Red,	// 0
	  Green,	// 1
	  Yellow,  // 2
	  Blue,  // 3
	  Battery1,  // 4
	  Battery2,  // 5
	  Battery3,  // 6
	  Battery4	// 7
};

enum mj828_buttons	// enum of buttons on this device
{
	  PushButton,
	  LeverBrake,
	  LeverFront
};

typedef struct	// struct describing devices on MJ828
{
	mj8x8_t *mj8x8;  // pointer to the base class
	composite_led_t *led;  // pointer to LED structure
	button_t *button;  // array of button_t - two buttons
} mj828_t;

void mj828_ctor();	// declare constructor for concrete class

extern mj828_t *const Device;  // declare pointer to public struct part

#endif // MJ828_

#endif /* CORE_INC_MJ828_MJ828_H_ */
