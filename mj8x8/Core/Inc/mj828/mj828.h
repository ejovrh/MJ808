#ifndef CORE_INC_MJ828_MJ828_H_
#define CORE_INC_MJ828_MJ828_H_

#include "main.h"
#if defined(MJ828_)	// if this particular device is active

#define BUTTON_COUNT 3	// how many buttons are there
#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD 2499	// ADC - 250ms
#define TIMER14_PERIOD 19	// charlieplexing - 2ms
#define TIMER16_PERIOD 249	// button handling - 25ms
#define TIMER17_PERIOD 24	// event handling - 2.5ms

#define AUTOLIGHT_DEBUG true

#ifdef AUTOLIGHT_DEBUG
#define AUTOLIGHT_THRESHOLD_LIGHT_OFF 3200 // debug values
#define AUTOLIGHT_THRESHOLD_LIGHT_ON 3500	// debug values
#else
// POTI wiper set at approx. 75% - early dusk
#define AUTOLIGHT_THRESHOLD_LIGHT_OFF 1843	// high light threshold - 4096/2=2048; 2048-10%
#define AUTOLIGHT_THRESHOLD_LIGHT_ON 2253 // low light threshold - 4096/2=2048; 2048+10%
#endif

#include "mj8x8\mj8x8.h"
#include "led\led.h"
#include "button\button.h"
#include "adc\adc.h"

#include "mj828/autolight.h"
#include "mj828/autobatt.h"

// definitions of device/PCB layout-specific hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_8	// TODO - move to PA?
#define TCAN334_Standby_GPIO_Port GPIOB
#define LeverFront_Pin GPIO_PIN_1
#define LeverFront_GPIO_Port GPIOA
#define LeverFront_EXTI_IRQn EXTI0_1_IRQn
#define LeverBrake_Pin GPIO_PIN_2
#define LeverBrake_GPIO_Port GPIOA
#define LeverBrake_EXTI_IRQn EXTI2_3_IRQn
#define VBat_Pin GPIO_PIN_3
#define VBat_GPIO_Port GPIOA
#define Pushbutton_Pin GPIO_PIN_0	// TODO - move to PA?
#define Pushbutton_GPIO_Port GPIOB
#define Pushbutton_EXTI_IRQn EXTI0_1_IRQn
#define Phototransistor_Pin GPIO_PIN_1	// TODO - move to PA?
#define Phototransistor_GPIO_Port GPIOB
#define CP4_Pin GPIO_PIN_15		// TODO - consolidate charlieplex pins into one port
#define CP4_GPIO_Port GPIOA
#define CP2_Pin GPIO_PIN_3	// TODO - move to PA?
#define CP2_GPIO_Port GPIOB
#define CP3_Pin GPIO_PIN_4	// TODO - move to PA?
#define CP3_GPIO_Port GPIOB
#define CP1_Pin GPIO_PIN_5	// TODO - move to PA?
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

enum mj828_adcchannels
{
	  Vbat,  // battery voltage - PA3
	  Darkness,  // Phototransistor - PB1 - low value = little darkness, high value = lots of darkness
	  Temperature  // internal temperature sensor
};

enum mj828_buttons	// enum of buttons on this device
{
	  PushButton,  //	pushbutton on lamp body
	  LeverFront,  // left brake lever pushed forward
	  LeverBrake	// left brake lever in braking action
};

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		/* 0x0F - lower nibble
		 * CAN has to be active
		 */
		uint8_t DoHeartbeat :1;  // bit 0 - HeartBeat is running
		uint8_t _1 :1;  //
		uint8_t _2 :1;  //
		uint8_t _3 :1;  //

		/* 0xF0 - upper nibble
		 * CAN can be in standby mode
		 */
		uint8_t CANActive :1;  // CAN is actively being used *is used as a flag to avoid re-entering e.g. __can_go_into_active_mode()
		uint8_t _5 :1;	//
		uint8_t ButtonPressed :1;  // any button is in use
		uint8_t LEDsOn :1;  // UI LEDS are on: timer ISR - stop mode will break functionality
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj828_activity_t;

typedef struct	// struct describing devices on MJ828
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj828_activity_t *activity;  // pointer to struct(union) indicating device activity status
	composite_led_t *led;  // pointer to LED structure
	button_t *button;  // array of button_t - two buttons
	adc_t *adc;  // ADC on device
	autolight_t *autolight;  // automatic light handling feature
	autobatt_t *autobatt;  // automatic battery handling feature

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj828_t;

void mj828_ctor();	// declare constructor for concrete class

extern mj828_t *const Device;  // declare pointer to public struct part

#endif // MJ828_

#endif /* CORE_INC_MJ828_MJ828_H_ */
