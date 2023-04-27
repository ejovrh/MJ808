#ifndef CORE_INC_MJ808_MJ808_H_
#define CORE_INC_MJ808_MJ808_H_

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
		uint8_t UtilLEDOn :1;  // utility LED blinking (not shining) is in progress
		uint8_t ButtonPressed :1;  // some button is being pressed
		uint8_t HighBeamOn :1;  // high beam is on: PWM - stop mode will break functionality
		uint8_t FrontLightOn :1;  // front light is on: PWM - stop mode will break functionality

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  //
		uint8_t _7 :1;  // bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj808_activity_t;

#include "main.h"
#if defined(MJ808_)	// if this particular device is active
#define CANID_SELF CANID_MJ808

#define UTILLED	2
#define BUTTONPRESSED 3
#define HIGHBEAM 4
#define FRONTLIGHT 5

#define BUTTON_COUNT 1	// how many buttons are there
#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER14_PERIOD 49	// LED handling - 20ms
#define TIMER16_PERIOD 249	// button handling - 25ms
#define TIMER17_PERIOD 24	// event handling - 2.5ms

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_1	//	defined here but initialised in mj8x8.c // TODO - move to PA?
#define TCAN334_Standby_GPIO_Port GPIOB	//	defined here but initialised in mj8x8.c
#define Pushbutton_Pin GPIO_PIN_0	// TODO - move to PA?
#define Pushbutton_GPIO_Port GPIOB
#define Switch_EXTI_IRQn EXTI0_1_IRQn	// TODO - move to PA?
#define FrontLED_Pin GPIO_PIN_3	// TODO - move to PA?
#define FrontLED_GPIO_Port GPIOB
#define RedLED_Pin GPIO_PIN_4	// TODO - move to PA?
#define RedLED_GPIO_Port GPIOB
#define GreenLED_Pin GPIO_PIN_5	// TODO - move to PA?
#define GreenLED_GPIO_Port GPIOB
// definitions of device/PCB layout-dependent hardware pins

#define FRONT_LIGHT_CCR htim2.Instance->CCR2	// Output Compare Register for PWM of front light

enum mj808_leds  // enum of lights on this device
{
	  Red,
	  Green,
	  Front
};

enum mj808_buttons	// enum of buttons on this device
{
	  PushButton
};

typedef struct	// struct describing devices on MJ808
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj808_activity_t *activity;  // pointer to struct(union) indicating device activity status
	composite_led_t *led;  // pointer to LED structure
	button_t *button;  // array of button_t - one buttons

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj808_t;

void mj808_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj808_t *const Device;  // declare pointer to public struct part

#endif // MJ808_

#endif /* CORE_INC_MJ808_MJ808_H_ */
