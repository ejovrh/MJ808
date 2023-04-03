#ifndef CORE_INC_MJ838_MJ838_H_
#define CORE_INC_MJ838_MJ838_H_

#include "main.h"
#if defined(MJ838_)	// if this particular device is active

#include "mj8x8\mj8x8.h"
#include "button\button.h"

// definitions of device/PCB layout-dependent hardware pins
#define TCAN334_Standby_Pin GPIO_PIN_1	//	defined here but initialised in mj8x8.c // TODO - move to PA?
#define TCAN334_Standby_GPIO_Port GPIOB	//	defined here but initialised in mj8x8.c
// definitions of device/PCB layout-dependent hardware pins

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
		uint8_t _6 :1;  //
		uint8_t _7 :1;  //
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} mj838_activity_t;

typedef struct	// struct describing devices on MJ838
{
	mj8x8_t *mj8x8;  // pointer to the base class
	mj838_activity_t *activity;  // pointer to struct(union) indicating device activity status

	void (*StopTimer)(TIM_HandleTypeDef *timer);	// stops timer identified by argument
	void (*StartTimer)(TIM_HandleTypeDef *timer);  // starts timer identified by argument
} mj838_t;

void mj838_ctor(void);	// declare constructor for concrete class

// all devices have the object name "Device", hence the preprocessor macro
extern mj838_t *const Device;  // declare pointer to public struct part

#endif // MJ838_

#endif /* CORE_INC_MJ838_MJ838_H_ */
