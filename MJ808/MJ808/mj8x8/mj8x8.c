#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/io.h>

#include "mj8x8.h"

extern void DoNothing(void);

typedef struct															// mj8x8_t actual
{
	mj8x8_t public;														// public struct

	uint8_t __NumericalCAN_ID;											// private - ordered device number - A0 (0th device) until 3C (15th device), used in Heartbeat()
	uint8_t __FlagDoHeartbeat : 1;										// private - shall the heartbeat be initiated?
	uint8_t __BeatIterationCount : 4;									// private - how many times did we wakeup, sleep and wakeup again
	uint8_t __FlagDoDefaultOperation : 2;								// we are alone on the bus - shall we do our device-specific default operation?
} __mj8x8_t;

extern __mj8x8_t __MJ8x8;												// declare mj8x8_t actual

// provides a periodic heartbeat based on the watchdog timer interrupt
static void _Heartbeat(message_handler_t * const msg)
{
	if (__MJ8x8.__FlagDoHeartbeat)										// if we are in heartbeat mode
	{
		if (__MJ8x8.__BeatIterationCount == __MJ8x8.__NumericalCAN_ID)	// see if this counter iteration is our turn
		{
			msg->SendMessage(CMND_ANNOUNCE, 0x00, 1);					// broadcast CAN heartbeat message

			__MJ8x8.__FlagDoHeartbeat = 0;								// heartbeat mode of for the remaining counter iterations
			WDTCR |= (_BV(WDCE) | _BV(WDE));							// WDT change enable sequence
			WDTCR = ( _BV(WDIE) | _BV(WDP2) | _BV(WDP1) );				// set watchdog timer set to 1s

			#if defined(MJ808_)
			// TODO - access via object
			//Device->led->led[Utility].Shine(UTIL_LED_RED_BLINK_1X);
			//_util_led_mj808(UTIL_LED_RED_BLINK_1X);
			#endif
		}
	}

	if ((!__MJ8x8.__BeatIterationCount) && (!__MJ8x8.__FlagDoHeartbeat))// counter roll-over, change from slow to fast
	{
		__MJ8x8.__FlagDoHeartbeat = 1;									// set heartbeat mode on
		++__MJ8x8.__FlagDoDefaultOperation;								// essentially count how many times we are in non-heartbeat count mode

		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = ( _BV(WDIE) | _BV(WDP2)  );								// watchdog timer set to 0.25
	}
	++__MJ8x8.__BeatIterationCount;										// increment the iteration counter
};

__mj8x8_t __MJ8x8 =														// instantiate mj8x8_t actual and set function pointers
{
	.public.HeartBeat = &_Heartbeat,									// implement device-agnostic default behavior - heartbeat
	.public.HeartbeatPeriodic = &DoNothing,								// every invocation for the heartbeat ISR runs this, implemented by derived classes
	.public.EmptyBusOperation = &DoNothing,								// implement device-agnostic default behavior - do nothing, usually an override happens
	.__FlagDoHeartbeat = 1,												// start with discovery mode
	.__FlagDoDefaultOperation = 0
};

mj8x8_t * mj8x8_ctor(const uint8_t in_own_sidh)
{
	// GPIO state definitions
	{
	// state initialization of device-unspecific pins
	gpio_conf(MCP2515_INT_pin, INPUT, HIGH);							// INT1, active low

	gpio_conf(ICSP_DO_MOSI, OUTPUT, LOW);								// data out - output pin
	gpio_conf(ICSP_DI_MISO, INPUT, LOW);								// data in - input pin
	gpio_conf(SPI_SCK_pin, OUTPUT, LOW);								// low for proper CPOL = 0 waveform
	gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);						// high (device inert), low (device selected)
	// state initialization of device-unspecific pins
	}

	__MJ8x8.__NumericalCAN_ID = (uint8_t) ( (in_own_sidh >>2 ) & 0x0F );

	__MJ8x8.public.can = can_ctor();									// pass on CAN public part
	__MJ8x8.public.mcu = attiny_ctor();									// pass on MCU public part

	__MJ8x8.public.can->own_sidh = in_own_sidh;							// high byte
	__MJ8x8.public.can->own_sidl = (RCPT_DEV_BLANK | BLANK);			// low byte

	return &__MJ8x8.public;												// return address of public part; calling code accesses it via pointer
};

ISR(WDT_OVERFLOW_vect, ISR_NOBLOCK)										// heartbeat of device on bus - aka. active CAN bus device discovery
{
	/* method of operation:
	 *	- on the CAN bus there is room for max. 16 devices - in numeric form devices #0 up to #15
	 *	- each device has its own unique NumericalCAN_ID, which is derived from its unique CAN bus ID
	 *
	 *	- on each watchdog timer iteration, BeatIterationCount is incremented (it is able to roll over from 0xf to 0x0)
	 *	- when BeatIterationCount is equal to the device's NumericalCAN_ID, a heartbeat message is sent, otherwise nothing is done
	 *
	 *	- there are two modes: fast count and slow count - each set via WDTCR, the WatchDog Timer Control Register
	 *		- fast count is performed when a heartbeat is supposed to be sent
	 *			this mode speeds up the heartbeat procedure itself
	 *
	 *		- slow count is performed when no heartbeat is supposed to be sent
	 *			this mode acts as a delay for heartbeat messages themselves
	 *
	 *	i.e. start in heartbeat mode, count fast to one's own ID, send the message and then exit heartbeat mode,
	 *		continue counting slow until a counter rollover occurs and enter heartbeat mode again.
	 *
	 *	each device on the bus does this procedure.
	 *	after one complete iteration each device should have received some other device's heartbeat message.
	 *	on each and every message reception the senders ID is recorded in the canbus_t struct. thereby one device keeps track of its neighbors on the bus.
	 *
	 *	if there are no devices on the bus, a device-specific default operation is executed.
	 */

	// TODO - implement sleep cycles for processor and CAN bus hardware
	sleep_disable();													// wakey wakey

	WDTCR |= _BV(WDIE);													// setting the bit prevents a reset when the timer expires
	//Device.mj8x8->mcu->wdtcr |= _BV(WDIE);

	__MJ8x8.public.HeartBeat(MsgHandler);								// execute the heartbeat

	__MJ8x8.public.HeartbeatPeriodic();									// execute something heatbeat-ISR periodic, implemented by derived classes

	if ( (! MsgHandler->Devices) && (__MJ8x8.__FlagDoDefaultOperation > 1) )		// if we have passed one iteration of non-heartbeat mode and we are alone on the bus
		__MJ8x8.public.EmptyBusOperation();								// perform the device-specific default operation (is overridden in specific device constructor)

	sleep_enable();														// back to sleep
}