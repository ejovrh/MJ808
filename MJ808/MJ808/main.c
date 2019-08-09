#include <avr/interrupt.h>
#include <avr/sleep.h>

#define MJ808_															// what device to compile for?

#if defined(MJ808_)														// mj808 header include
#include "mj808.h"
#endif
#if defined(MJ818_)														// mj818 header include
#include "mj818.h"
#endif
#if defined(MJ828_)														// mj828 header include
#include "mj828.h"
#endif

// TODO - refactor away
volatile uint8_t flag_lamp_is_on = 0;									// flag - indicates if button turned the device on, used for pushbutton handling

int main(void)
{
	mj8x8_ctor(&MJ8X8, &CAN, &MCU);										// call base class constructor & tie in associated object addresses

	message_handler_ctor(&message, &CAN, &BUS, &msg_in, &msg_out);		// call message handler constructor

	#if defined(MJ808_)													// MJ808 - call derived class constructor and tie in base class
	mj808_ctor(&device, &MJ8X8, &LED, &BUTTON, &message);
	#endif
	#if defined(MJ818_)													// MJ818 - call derived class constructor and tie in base class
	mj818_ctor(&device, &MJ8X8, &LED, &message);
	#endif
	#if defined(MJ828_)													// MJ828 - call derived class constructor and tie in base class
	mj828_ctor(&device, &MJ8X8, &LED, &BUTTON, &message);
	#endif

	// TODO - implement micro controller sleep cycles
	set_sleep_mode(SLEEP_MODE_IDLE);									// 11mA
	//set_sleep_mode(SLEEP_MODE_STANDBY);								// 10mA
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN);								// 10mA
	sleep_enable();
	sleep_cpu();

	while (1)															// forever loop
	{
		if (MCUCR & _BV(SE))											// if sleep is enabled
			sleep_cpu();												// ...sleep
	}
}


ISR(INT1_vect)															// ISR for INT1 - triggered by CAN message reception of the MCP2515
{
	sleep_disable();													// wakey wakey

	volatile can_t *can = device.mj8x8->can;							// get pointer to CAN instance

	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	void handle_message_error(volatile can_t *in_can)					// handles message error interrupts
	{
		in_can->BitModify(CANINTF, _BV(MERRF), 0x00);					// clear the flag
	};

	inline void helper_handle_rx(void)									// handles incoming message interrupts
	{
		device.mj8x8->PopulatedBusOperation(message.ReceiveMessage(&message), &device);	// let the device deal with the message
	};

	void helper_handle_error(volatile can_t *in_can)					// handles RXBn overflow interrupts
	{
		if (in_can->eflg & _BV(TXBO))									// TODO - handle bus off situation
		{
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
		}

		if (in_can->eflg & _BV(TXEP))									// handle TX error-passive situation
		{
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
			device.mj8x8->can->Sleep(in_can, 1);						// put to sleep
		}

		if (in_can->eflg & _BV(RXEP))									// TODO - handle RX error-passive situation
		{
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
		}

		if (in_can->eflg & _BV(TXWAR))									// TODO - handle TX waring situation
		{
			// TODO - log it
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
		}

		if (in_can->eflg & _BV(RXWAR))									// TODO - handle RX warning situation
		{
			// TODO - log it
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
		}

		if (in_can->eflg & _BV(RX0OVR))									// RXB0 overflow - datasheet figure 4.3, p. 26
		{
			// FIXME - check for correct RX buffer clearing
			helper_handle_rx();											// handle the message
			in_can->BitModify(EFLG, _BV(RX0OVR), 0x00);					// clear the overflow bit
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
			return;
		}

		if (in_can->eflg & _BV(RX1OVR))									// RXB1 overflow - datasheet figure 4.3, p. 26
		{
			helper_handle_rx();											// handle the message
			in_can->BitModify(EFLG, _BV(RX1OVR), 0x00);					// clear the overflow bit
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
			return;
		}

		//mcp2515_opcode_bit_modify(CANINTF, _BV(ERRIF), 0x00);			// clear the error interrupt flag
	};

	void helper_handle_wakeup(volatile can_t *in_can)					// handles wakeup interrupts
	{
		// functionally, this fucntion is similar to can_sleep(), but still different in one aspect:
			// can_sleep(foo_can, 0) wakes up by triggering a wake up interrupt, which helper_handle_wakeup() handles
			// helper_handle_wakeup() can be triggered by any external event while can_sleep(foo_can, 0) is called from within this program

		in_can->ChangeOpMode(REQOP_CONFIG);								// put into config mode -> clears all error counters; other settings appear to remain consistent
		in_can->ChangeOpMode(REQOP_NORMAL);								// put back into normal mode

		in_can->BitModify(CANINTF, _BV(WAKIF), _BV(WAKIF));				// clear the wakeup flag
		in_can->BitModify(CANINTF, 0xFF, 0x00);							// clear the wakeup flag
		gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);					// wake up MCP2561

		in_can->in_sleep = 0;
	};

	void helper_handle_tx(volatile can_t *in_can)
	{
		in_can->BitModify(CANINTF, 0x1C, 0x00);
	};

	do		// ICOD loop handler - runs while ICOD != 0
	{
		can->ReadBytes(TEC, &can->tec, 2);								// read in TEC and REC
		can->ReadBytes(CANINTF, &can->canintf, 3);						// read in CANINTF and EFLG
		can->ReadBytes(CANCTRL, &can->canctrl, 1);

		can->icod =  ((can->canstat & 0x0E) >> 1);						// right shift so that CANSTAT.U0 cant interfere

		// TODO - implement something like a vpointer lookup table or branch table instead of this shit:
		switch (can->icod)												// handling of cases depending on ICOD value - sort of priority-style

		{																// while loops over ICOD bit values, each case handles an ICOD situation
			case 0:														// no interrupt
				break;

			case 1:														// error interrupt
				helper_handle_error(can);
				break;

			case 2:														// wake-up interrupt
				helper_handle_wakeup(can);
				break;

			case 3:														// TXB0 interrupt
				helper_handle_tx(can);
				break;

			case 4:														// TXB1 interrupt
				helper_handle_tx(can);
				break;

			case 5:														// TXB2 interrupt
				helper_handle_tx(can);
				break;

			case 6:														// RXB0 interrupt
				helper_handle_rx();
				break;

			case 7:														// RXB1 interrupt
				helper_handle_rx();
				break;
		};

	} while (can->icod);

	sleep_enable();														// back to sleep
}

ISR(PCINT2_vect)														// ISR for pushbuttons
{
	sleep_disable();													// wakey wakey

	;
	//button_debounce(&mj808.button[0]);								// from here on the button is debounced and states can be consumed
	//button_debounce(&mj828.button[1]);								// ditto

	//mj828.led->leds[blue].on = mj828.button[0].toggle;
	//mj828.led->leds[yellow].on = mj828.button[1].is_pressed;
	//mj828.led->leds[red].blink_count = (mj828.button[0].hold_error || mj828.button[1].hold_error);
	//mj828.led->leds[battery_led1].on = mj828.button[0].hold_temp;
	//mj828.led->leds[battery_led2].on = mj828.button[1].hold_temp;

	sleep_enable();														// back to sleep
}

#if ( defined(MJ808_) | defined(MJ828_) )								// ISR for timers 1 A compare match - button handling
ISR(TIMER1_COMPA_vect)													// timer/counter 1 - button debounce - 25ms
{	// code to be executed every 25ms

	sleep_disable();													// wakey wakey

	#if defined(MJ808_)													// pushbutton code for mj808
	button_debounce(&device.button[0]);									// from here on the button is debounced and states can be consumed

	if (device.button[0].hold_error)
		// TODO - access via object
		util_led(UTIL_LED_RED_BLINK_6X);

	// FIXME - on really long button press (far beyond hold error) something writes crap into memory, i.e. the address of PIND in button struct gets overwritten, as does the adders of the led struct
	if (!flag_lamp_is_on && device.button[0].hold_temp)					// turn front light on
	{
		if (message.bus->devices._MJ818)								// if rear light is present
			message.SendMessage(&message, (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0xFF, 2);	// turn on rear light

		if (message.bus->devices._MJ828)								// dashboard is present
			message.SendMessage(&message, (CMND_DEVICE | DEV_LU | DASHBOARD), 0x00, 1);		// dummy command to dashboard

		// TODO - access via object
		fade(0x20, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);
		util_led(UTIL_LED_GREEN_ON);									// power on green LED

		message.SendMessage(&message, (MSG_BUTTON_EVENT | BUTTON0_ON), 0x00, 1);			// convey button press via CAN

		flag_lamp_is_on = 1;
	}

	if ((flag_lamp_is_on && !device.button[0].hold_temp) || device.button->hold_error)		// turn front light off
	{
		if (message.bus->devices._MJ818)								// if rear light is present
			message.SendMessage(&message, (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0x00, 2);	// turn off rear light

		// TODO - access via object
		fade(0x00, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);				// turn off
		util_led(UTIL_LED_GREEN_OFF);									// power off green LED

		message.SendMessage(&message, (MSG_BUTTON_EVENT | BUTTON0_OFF), 0x00, 1);			// convey button release via CAN
		flag_lamp_is_on = 0;
	}
	#endif

	#if defined(MJ828_)													// pushbutton code for mj828
	button_debounce(&device.button[0]);									// from here on the button is debounced and states can be consumed
	button_debounce(&device.button[1]);									// ditto

	device.led->led_array[blue].on = device.button[0].toggle;
	device.led->led_array[yellow].on = device.button[1].is_pressed;
	device.led->led_array[red].blink_count = (device.button[0].hold_error || device.button[1].hold_error);
	device.led->led_array[battery_led1].on = device.button[0].hold_temp;
	device.led->led_array[battery_led2].on = device.button[1].hold_temp;
	#endif

	sleep_enable();														// back to sleep
}

#if defined(MJ828_)														// ISR for timer0 - 16.25ms - charlieplexing timer
ISR(TIMER0_COMPA_vect)													// timer/counter0 - 16.25ms - charlieplexed blinking
{
	if (LED.flag_any_glow)												// if there is any LED to glow at all
		charlieplexing_handler(&LED);									// handles LEDs according to CAN message (of type CMND_UTIL_LED)
}
#endif

#endif

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

	// TODO - refactor into message handler - where the message gets transmitted
	if (device.mj8x8->can->in_sleep)									// if the CAN infra. is sleeping
		device.mj8x8->can->Sleep(device.mj8x8->can, 0);					// wake it up

	device.mj8x8->HeartBeat(&message);

	if ( (! message.bus->devices.uint16_val) && (message.bus->FlagDoDefaultOperation > 1) )		// if we have passed one iteration of non-heartbeat mode and we are alone on the bus
		device.mj8x8->EmptyBusOperation();								// perform the device-specific default operation

	sleep_enable();														// back to sleep
}