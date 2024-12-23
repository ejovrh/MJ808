#ifndef CORE_INC_MJ514_MJ514_H_
#define CORE_INC_MJ514_MJ514_H_

typedef union  // union for activity indication, see mj8x8_t's _Sleep()
{
	struct
	{
		/*  0x3F - if any of bits 0 though 5 are set - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus on
		 * additionally: if CANBUS_ACTIVE_MASK has bits not set, CANbus will be off
		 */
		// bit0
		uint8_t DoHeartbeat :1;  // DoHB // HeartBeat is running
		uint8_t CANActive :1;  // CAN // CAN is actively being used *is used as a flag to avoid re-entering e.g. __can_go_into_active_mode()

		// 0x3C - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus off
		uint8_t Shifting :1;  // act2 // Shifting in progress or not // TODO - test it
		uint8_t _3 :1;  // act3 //
		uint8_t _4 :1;  // act4 //
		uint8_t _5 :1;  // act5 //

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // _6 //
		uint8_t _7 :1;	// _7 //
	// bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj514_activity_t;

#include "main.h"
#if defined(MJ514_)	// if this particular device is active
#define USE_I2C 1	// use I2C
#define USE_EVENTHANDLER 0	// shall EventHandler code be included -- timer17 is used up for ADC timebase !!!

// activity bit positions
#define SHIFTING 2

#define TIMER_PRESCALER 799	// global - 8MHz / 799+1 = 10kHz update rate
#define TIMER2_PERIOD 0x100	// FIXME - find proper value
#define MOTOR_OFF 0x00	//
#define TIMER3_PERIOD  0xFFFF	// use max. range for rotary encoder pulse count
#define TIMER3_IC1_FILTER	0x0	// TODO - figure out filter
#define TIMER3_IC2_FILTER 0x0	// TODO - figure out filter
#define TIMER16_PERIOD 499	// TODO - verify proper timing: 10ms or 100ms may be better - rotary encoder time base - 50ms
#define TIMER17_PERIOD 99	// ADC time base - 10ms

#include "mj8x8/mj8x8.h"
#include "gear.h" // Rohloff electronic shifting unit

#define VREFINT_CAL *((uint16_t*) ((uint32_t) 0x1FFFF7BA)) // value is 1525 - internal reference voltage calibration data: acquired by measuring Vdda = 3V3 (+-10%) at 30 DegC (+-5 DegC), see RM0091l paragraph 13.8, p 260 for conversion formula
#define TS_CAL1	*((uint16_t*) ((uint32_t) 0x1FFFF7B8)) // calibration value at 30 degrees C, value is 1777
#define TS_CAL2	*((uint16_t*) ((uint32_t) 0x1FFFF7C2)) // calibration at 110 degrees C, value is 1319

#define ADC_CHANNELS 3	// how many ADC channels are we using
#define ADC_CHANNEL_MOTOR_IPROP ADC_CHANNEL_4 // PA4

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_5	//	defined here but initialised in mj8x8.c
#define TCAN334_Standby_GPIO_Port GPIOA	//	defined here but initialised in mj8x8.c

#define Motor_IN1_Pin GPIO_PIN_0	// motor direction control PWM output signal channel1
#define Motor_IN1_GPIO_Port GPIOA
#define Motor_IN2_Pin GPIO_PIN_1	// motor direction control PWM output signal
#define Motor_IN2_GPIO_Port GPIOA
#define Motor_FLT_Pin GPIO_PIN_2	// motor controller fault interrupt pin
#define Motor_FLT_GPIO_Port GPIOA
#define Motor_FLT_EXTI_IRQn EXTI2_3_IRQn
#define Motor_SLP_Pin GPIO_PIN_3	// motor controller sleep pin: low - sleep, high - wake
#define Motor_SLP_GPIO_Port GPIOA
#define Motor_IPROP_Pin GPIO_PIN_4	// motor current monitoring ADC channel 4
#define Motor_IPROP_GPIO_Port GPIOA
#define Rotary_A_Pin GPIO_PIN_6	// rotary encoder A/B input for detection/measurement of gear rotation
#define Rotary_A_GPIO_Port GPIOA
#define Rotary_B_Pin GPIO_PIN_7	// rotary encoder A/B input for detection/measurement of gear rotation
#define Rotary_B_GPIO_Port GPIOA
#define I2C_SDA_Pin GPIO_PIN_7 // see i2c_ctor()
#define I2C_SCL_Pin GPIO_PIN_6 // see i2c_ctor()
#define I2C_GPIO_Port GPIOB

#define Debug_Pin GPIO_PIN_1	// TODO - remove debug pin when done
#define Debug_GPIO_Port GPIOF
// definitions of device/PCB layout-dependent hardware pins

enum mj514_adcchannels
{  // order is important! - this MCU has no ranking and the order of channels is by channel number
	  Iprop,  // Motor Iprop - PA4 - proportional motor current
	  Temperature,  // internal temperature sensor
	  Vrefint  // internal reference voltage
};

typedef struct	// struct describing devices on mj514
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj514_activity_t *activity;  // pointer to struct(union) indicating device activity status
	gear_t *gear;  // electronic gear shifting unit

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj514_t;

void mj514_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj514_t *const Device;  // declare pointer to public struct part

#endif // mj514_

#endif /* CORE_INC_MJ514_MJ514_H_ */
