#ifndef CORE_INC_MJ8X8_CAN_H_
#define CORE_INC_MJ8X8_CAN_H_

#include <inttypes.h>
#include "can_msg.h"

#define POWERSAVE_CANBUS_ACTIVE_MASK 0x03
#define	POWERSAVE_DEVICE_SLEEPONEXIT_ACTIVE_MASK 0x3F

typedef union  // union for core activity indication and CAN standby control (is exposed to end device via mj8x8_t)
{
	/* the purpose of activity_t ...
	 * ... is to facilitate powersaving.
	 * the uC can be in two states: stop mode and sleep on exit (see datasheet)
	 *
	 * if the byte representation of activity_t is zero, the device will enter stop mode.
	 * wakeup from stop mode can only be done by EXTI - either button press or CAN activity (once the GPIO has been configured accordingly)
	 * if the byte representation of activity_t is non-zero, the device will enter sleep on exit (if timers permit it).
	 *
	 * while entering sleep on exit, CAN can be stopped or kept running by setting/clearing flags. see the bitfield below.
	 *
	 * the type activity_t by itself is used only in can_t, however it is exposed to all upper objects.
	 * for instance, mj8x8_t sees it as part of the can_t member and the final device implementation (e.g. mj808_t) in turn
	 * can see it as part of the mj8x8_t base object.
	 *
	 * this finds its utilization in e.g. mj808_led.c, where the front light/high beam set/unset flags while in operation.
	 *
	 * names in the lower nibble have to be copied "up" (into e.g. mj808.h) manually, while the upper nibble's names are reserved for the actual implementations.
	 */
	struct
	{
		/*  0x3F - if any of bits 0 though 5 are set - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus on
		 * additionally: if CANBUS_ACTIVE_MASK has bits not set, CANbus will be off
		 */
		uint8_t DoHeartbeat :1;  // bit 0
		uint8_t CANActive :1;  // bit 1

		// 0x3C - the device will execute HAL_PWR_EnableSleepOnExit() w. CANbus off
		uint8_t _2 :1;  // bit 2
		uint8_t _3 :1;  // bit 3
		uint8_t _4 :1;  // bit 4
		uint8_t _5 :1;	// bit 5

		// 0xC0 - don't care - the device will execute HAL_PWR_EnterSTOPMode()
		uint8_t _6 :1;  // bit 6
		uint8_t _7 :1;  // bit 7
	};
	uint8_t byte;  // byte-wise representation of the above bitfield
} activity_t;

typedef struct can_t	// can_t struct describing the CAN device as a whole
{
	uint8_t own_sidh;  // SIDH for device
	uint8_t own_sidl;  // SIDL for device
	activity_t *activity;  // device activity indicator

	void (*Timer1Start)(void);  // starts timer1 (see mj8x8_t for the stop function)
	void (*const GoBusActive)(const uint8_t in_val);	// puts the CAN infrastructure to sleep (and wakes it up)
	void (*const RequestToSend)(can_msg_t *const msg);  // requests message to be sent to the CAN bus
} can_t __attribute__((aligned(8)));

can_t* can_ctor();	// CAN object constructor - does function pointer & hardware initialisation

#endif /* CORE_INC_MJ8X8_CAN_H_ */
