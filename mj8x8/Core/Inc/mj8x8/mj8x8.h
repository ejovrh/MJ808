#ifndef CORE_INC_MJ808_MJ8X8_H_
#define CORE_INC_MJ808_MJ8X8_H_

#include "mj8x8\can.h"
#include "mj8x8\mj8x8_commands.h"
#include "message\message.h"
#include "stm32f0xx_hal.h"	// include HAL layer

#define HEARTBEAT 0
#define CANACTIVE 1

// definitions of device/PCB layout-independent hardware pins
#define CAN_RX_Pin GPIO_PIN_11
#define CAN_RX_GPIO_Port GPIOA
#define CAN_TX_Pin GPIO_PIN_12
#define CAN_TX_GPIO_Port GPIOA
// definitions of device/PCB layout-independent hardware pins

#define TIMER1_PRESCALER 799	// 8MHz / 799+1 = 10kHz update rate
#define TIMER1_PERIOD 1249	// with above pre-scaler and a period of 1249, we have an 125ms interrupt frequency

extern TIM_HandleTypeDef timer;

typedef struct	// "base class" struct for mj8x8 devices
{
	uint8_t **activity;  // device activity indicator from one level down (can_t)
	can_t *can;  // pointer to the CAN structure

	void (*const StartCoreTimer)(void);  // starts timer1
	void (*const StopCoreTimer)(void);  // stops timer1
	void (*const HeartBeat)(message_handler_t *const msg);	// default periodic heartbeat for all devices
	void (*const UpdateActivity)(const uint8_t act, const uint8_t val);  // updates a particular activity and notifies the bus
	uint8_t (*const GetActivity)(const uint8_t act);	// returns whether some activity is ON (1) or OFF(0)
	void (*EmptyBusOperation)(void);	// device's default operation on empty bus, implemented in derived class
	void (*PopulatedBusOperation)(message_handler_t *const in_msg);  // device operation on populated bus, executed by incoming msg ISR; operates by means of MsgHandler object
	void (*PreSleep)(void);  // prepares derived object for sleep mode
	void (*PreStop)(void);  // prepares derived object for stop mode

	void (*Sleep)(void);	// sleep commands - quite common to each device
} mj8x8_t;

mj8x8_t* mj8x8_ctor(const mj8x8_Devices_t in_MJ8x8_ID);  // declare constructor for abstract class

#endif /* CORE_INC_MJ808_MJ8X8_H_ */
