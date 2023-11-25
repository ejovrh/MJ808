#ifndef CORE_INC_MJ828_MJ828_H_
#define CORE_INC_MJ828_MJ828_H_

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
		uint8_t _2 :1;  //
		uint8_t _3 :1;  //
		uint8_t ButtonPressed :1;  // BTN // some button is being pressed
		uint8_t LEDsOn :1;  // LEDs // UI LEDS are on: timer ISR - stop mode will break functionality

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t AutoBatt :1;	// AB // automatic light behaviour based on battery charge state
		uint8_t AutoLight :1;  // AL // automatic light behaviour based on ambient light
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj828_activity_t;

#include "main.h"
#if defined(MJ828_)	// if this particular device is active
#define CANID_SELF CANID_MJ828

#define BUTTONPRESSED 4
#define LEDS 5
#define AUTOBATT 6
#define AUTOLIGHT 7

#define VREFINT_CAL *((uint16_t*) ((uint32_t) 0x1FFFF7BA)) // value is 1525 - internal reference voltage calibration data: acquired by measuring Vdda = 3V3 (+-10%) at 30 DegC (+-5 DegC), see RM0091l paragraph 13.8, p 260 for conversion formula
#define TS_CAL1	*((uint16_t*) ((uint32_t) 0x1FFFF7B8)) // calibration value at 30 degrees C, value is 1777
#define TS_CAL2	*((uint16_t*) ((uint32_t) 0x1FFFF7C2)) // calibration at 110 degrees C, value is 1319

#define ADC_CHANNELS 4	// how many ADC channels are we using
#define ADC_CHANNEL_VBATT ADC_CHANNEL_3	// PA3
#define ADC_CHANNEL_PHOTOTRANSISTOR ADC_CHANNEL_1 // PA1
#define BUTTON_COUNT 3	// how many buttons are there
#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD 2499	// ADC - 250ms
#define TIMER14_PERIOD 19	// charlieplexed LED handling - 2ms
#define TIMER16_PERIOD 249	// button handling - 25ms
#define TIMER17_PERIOD 24	// event handling - 2.5ms

#define AUTOLIGHT_DEBUG true

#ifdef AUTOLIGHT_DEBUG
#define AUTOLIGHT_THRESHOLD_LIGHT_OFF 3200 // debug values so that ambient light sensor turns lights on/off in a room (as opposed to the outdoors)
#define AUTOLIGHT_THRESHOLD_LIGHT_ON 3500	// ditto
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
#define TCAN334_Standby_Pin GPIO_PIN_15
#define TCAN334_Standby_GPIO_Port GPIOA
#define LeverFront_Pin GPIO_PIN_3
#define LeverFront_GPIO_Port GPIOA
#define LeverFront_EXTI_IRQn EXTI2_3_IRQn
#define LeverBrake_Pin GPIO_PIN_2
#define LeverBrake_GPIO_Port GPIOA
#define LeverBrake_EXTI_IRQn EXTI2_3_IRQn
#define VBat_Pin GPIO_PIN_0
#define VBat_GPIO_Port GPIOB
#define Pushbutton_Pin GPIO_PIN_0
#define Pushbutton_GPIO_Port GPIOA
#define Pushbutton_EXTI_IRQn EXTI0_1_IRQn
#define Phototransistor_Pin GPIO_PIN_1
#define Phototransistor_GPIO_Port GPIOA
#define CP1_Pin GPIO_PIN_7
#define CP1_GPIO_Port GPIOA
#define CP2_Pin GPIO_PIN_6
#define CP2_GPIO_Port GPIOA
#define CP3_Pin GPIO_PIN_5
#define CP3_GPIO_Port GPIOA
#define CP4_Pin GPIO_PIN_4
#define CP4_GPIO_Port GPIOA

// definitions of device/PCB layout-specific hardware pins

enum mj828_leds  // enum of lights on this device
{
	  Red,	// bit pos. 0,
	  Green,	// bit pos. 1
	  Yellow,  // bit pos. 2
	  Blue,  // bit pos. 3
	  Battery1,  // bit pos. 4
	  Battery2,  // bit pos. 5
	  Battery3,  // bit pos. 6
	  Battery4	// bit pos. 7
};

enum mj828_adcchannels
{  // order is important! - this MCU has no ranking and the order of channels is by channel number
	  Vbat,  // battery voltage - PA3
	  Darkness,  // Phototransistor - PA1 - low value = little darkness, high value = lots of darkness
	  Temperature,  // internal temperature sensor
	  Vrefint  // internal reference voltage
};

enum mj828_buttons	// enum of buttons on this device
{
	  PushButton,  //	pushbutton on lamp body
	  LeverFront,  // left brake lever pushed forward
	  LeverBrake	// left brake lever in braking action
};

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
