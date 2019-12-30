#ifndef MJ8x8_H_
#define MJ8x8_H_

#include <inttypes.h>

#include "gpio.h"														// macros for pin definitions

#include "mj8x8\attiny4313.h"
#include "mj8x8\mcp2515.h"

#include "message\message.h"
#include "event\event.h"

#include "mj8x8\mj8x8_commands.h"

// definitions of device/PCB layout-independent hardware pins
#define SPI_SS_MCP2515_pin		B,	4,	4								// SPI - SS
#define ICSP_DI_MISO			B,	5,	5								// SPI - MISO; aka. DI; if run in master mode this is ... MISO
#define ICSP_DO_MOSI			B,	6,	6								// SPI - MOSI; aka. DO; ditto
#define SPI_SCK_pin				B,	7,	7								// SPI - SCK

#define MCP2515_INT_pin			D,	3,	3								// INT1
// definitions of device/PCB layout-independent hardware pins

typedef struct															// "base class" struct for mj8x8 devices
{
	can_t *can;															// pointer to the CAN structure
	ATtiny4313_t *mcu;													// pointer to MCU structure

	void (* const HeartBeat)(message_handler_t * const msg);			// default periodic heartbeat for all devices
	void (*EmptyBusOperation)(void);									// device's default operation on empty bus, implemented in derived class
	void (*PopulatedBusOperation)(message_handler_t * const in_msg);	// device operation on populated bus; operates by means of MsgHandler object
} mj8x8_t ;


mj8x8_t *mj8x8_ctor(const uint8_t in_own_sidh);							// declare constructor for abstract class

#endif /* MJ8x8_H_ */