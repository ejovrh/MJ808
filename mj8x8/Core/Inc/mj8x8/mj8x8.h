#ifndef CORE_INC_MJ808_MJ8X8_H_
#define CORE_INC_MJ808_MJ8X8_H_

#include "mj8x8\can.h"
#include "mj8x8\mj8x8_commands.h"
#include "message\message.h"
#include "stm32f0xx_hal.h"	// include HAL layer

// definitions of device/PCB layout-independent hardware pins
#define CAN_RX_Pin GPIO_PIN_11
#define CAN_RX_GPIO_Port GPIOA
#define CAN_TX_Pin GPIO_PIN_12
#define CAN_TX_GPIO_Port GPIOA
// definitions of device/PCB layout-independent hardware pins

extern TIM_HandleTypeDef timer;

typedef struct	// "base class" struct for mj8x8 devices
{
	uint8_t **activity;  // device activity indicator from one level down (can_t)
	can_t *can;  // pointer to the CAN structure

	void (*const HeartBeat)(message_handler_t *const msg);	// default periodic heartbeat for all devices
	void (*EmptyBusOperation)(void);	// device's default operation on empty bus, implemented in derived class
	void (*PopulatedBusOperation)(message_handler_t *const in_msg);  // device operation on populated bus, executed by incoming msg ISR; operates by means of MsgHandler object

	void (*Sleep)(void);	// sleep commands - quite common to each device
} mj8x8_t;

mj8x8_t* mj8x8_ctor(const uint8_t in_own_sidh);  // declare constructor for abstract class

#endif /* CORE_INC_MJ808_MJ8X8_H_ */
