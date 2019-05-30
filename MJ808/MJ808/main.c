#include <avr/interrupt.h>
#include <avr/sleep.h>

#define MJ808_															// what device to compile for?

#include "gpio.h"														// macros for pin definitions

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
can_message_t can_msg_outgoing;
can_message_t can_msg_incoming;

volatile canbus_t canbus_status =										// bit-wise info about CAN bus status
{
	.status = 0x80,														// bitwise representation of CAN bus status, start with discovery mode on
	.devices.uint16_val = 0x0000 ,										// bitwise representation of devices discovered, see #defines below CMND_ANNOUNCE
	.broadcast_iteration_count = 0, 									// counter
	.numerical_self_id = 0,												// decimal representation of SID
	.sleep_iteration = 0												// counter for sleep iterations, incremented by WDT_OVERFLOW_vect() ISR
};


volatile	uint8_t canintf;											// CAN debug variables for MCP2515 status readout from main()
volatile	uint8_t canstat;
volatile	uint8_t canctrl;
volatile	uint8_t eflg;
volatile	uint8_t rec;
volatile	uint8_t tec;


int main(void)
{
	#if defined(MJ808_)													// MJ808 GPIO state definitions
	#include "gpio_modes_mj808.h"
	#endif
	#if defined(MJ818_)													// MJ818 GPIO state definitions
	#include "gpio_modes_mj818.h"
	#endif
	#if defined(MJ828_)													// MJ828 GPIO state definitions
	#include "gpio_modes_mj828.h"
	#endif

	// instantiate the base class & call its constructor
	mj8x8_t MJ8X8;														// declare MJ8X8 object
	attiny4313_t MCU;													// declare MCU object
	can_t CAN;															// declare CAN object - its address will be used soon in the constructor ...

	mj8x8_ctor(&MJ8X8, &CAN, &MCU);

	// instantiate the derived class & tie to base class
	// instantiation of mj808_t is for now global in its header file
	// call mj808_t constructor
	#if defined(MJ808_)
	mj808_ctor(&device, &MJ8X8);
	#endif
	#if defined(MJ818_)
	mj818_ctor(&device, &MJ8X8);
	#endif
	#if defined(MJ828_)
	mj828_ctor(&device, &MJ8X8);
	#endif


// TODO - refactor away
	canbus_status.numerical_self_id = (uint8_t) ( (can_msg_outgoing.sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID


// TODO - put into constructors
	#if defined(MJ808_)													// device init for MJ808

	device.led = &LED;													// pass reference to LED struct

	LED.led_count = 2;

	device.button[0].PIN = (uint8_t *) 0x30; 							// 0x020 offset plus address - PIND register
	device.button[0].pin_number = 4;									// sw2 is connected to pin D0
	#endif

	#if defined(MJ818_)													// device init for MJ818
	device.led = &LED;													// pass reference to LED struct

	LED.led_count = 2;
	#endif

	#if defined(MJ828_)													// device init for MJ828
	device.led = &LED;													// pass reference to LED struct

	LED.led_count = 7;
	LED.flag_any_glow = 1;
	LED.leds[green].on = 1;

	device.button[0].pin_number = 0;									// sw2 is connected to pin D0
	device.button[0].pin_number = 1;									// sw2 is connected to pin D1
	device.button[0].PIN = (uint8_t *) 0x30;							// 0x020 offset plus address - PIND register
	device.button[1].PIN = (uint8_t *) 0x30;							// ditto
	#endif
// TODO - put into constructors

	// TODO - implement micro controller sleep cycles
	set_sleep_mode(SLEEP_MODE_IDLE);									// 11mA
	//set_sleep_mode(SLEEP_MODE_STANDBY);								// 10mA
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN);								// 10mA
	sleep_enable();
	sleep_cpu();

	while (1)															// forever loop
	{
		asm("nop");														// on purpose kept as empty as possible !!
		// FIXME - refactor away

		device.mj8x8->can->ReadBytes(CANINTF, &canintf, 1);				// download the interrupt flag register
		device.mj8x8->can->ReadBytes(CANSTAT, &canstat, 1);
		device.mj8x8->can->ReadBytes(CANSTAT, &canctrl, 1);
		device.mj8x8->can->ReadBytes(EFLG, &eflg, 1);
		device.mj8x8->can->ReadBytes(REC, &rec, 1);
		device.mj8x8->can->ReadBytes(TEC, &tec, 1);

		if (MCUCR & _BV(SE))											// if sleep is enabled
			sleep_cpu();												// ...sleep
	}
}


ISR(INT1_vect)															// ISR for INT1 - triggered by CAN message reception of the MCP2515
{
	sleep_disable();													// wakey wakey
	can_t *can = device.mj8x8->can;										// get pointer to CAN instance

	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	void handle_message_error(volatile can_t *in_can)					// handles message error interrupts
	{
		in_can->BitModify(CANINTF, _BV(MERRF), 0x00);					// clear the flag
	};

	void helper_handle_rx(volatile can_t *in_can)						// handles incoming message interrupts
	{
		in_can->ReceiveMessage(&can_msg_incoming);								// load the CAN message into its structure & clear the RX int flag

		// update the CAN BUS status structure; if we get a message from SID foo, then _BV(foo) shall be marked as "on the bus"
		canbus_status.devices.uint16_val |= ( 1 << ( (can_msg_incoming.sidh >> 2) & 0x0F ) ); // shift as many bits as the originating SID is in decimal

		// command for device
		if (can_msg_incoming.COMMAND & CMND_DEVICE)						//  we received a command for some device...
		{
			#if defined(SENSOR)
			if ((can_msg_incoming.COMMAND & DEV_SENSOR) == DEV_SENSOR)	// ...a sensor
			{
				dev_sensor(&can_msg_incoming);							// deal with it
				return;
			}
			#endif

			#if defined(MJ808_) || defined(MJ818_)
			if (can_msg_incoming.COMMAND & ( CMND_DEVICE | DEV_LIGHT ) )// ...a LED device
			{
				dev_light(&can_msg_incoming);							// deal with it
				return;
			}
			#endif

			#if defined(PWR_SRC)
			if ((can_msg_incoming.COMMAND & DEV_PWR_SRC) == DEV_PWR_SRC)// ...a power source
			{
				dev_pwr_src(&can_msg_incoming);							// deal with it
				return;
			}
			#endif

			#if defined(LOGIC_UNIT)
			if ((can_msg_incoming.COMMAND & DEV_LU) == DEV_LU)			// ...a logic unit
			{
				dev_logic_unit(&can_msg_incoming);						// deal with it
				return;
			}
			#endif
		}

		#if defined(MJ808_)
		if ( (can_msg_incoming.COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)// utility LED command
		{
			util_led(can_msg_incoming.COMMAND);							// blinky thingy
			return;
		}
		#endif

		#if defined(MJ828_)
		if ( (can_msg_incoming.COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)// utility LED command
		{
			return;														// HACK - can be removed once CMND_UTIL_LED is of new command structure

			if (can_msg_incoming.ARGUMENT == 0)
			return;

			LED.flag_any_glow = (can_msg_incoming.ARGUMENT & ( LED_STATE_MASK | LED_BLINK_MASK) ); // figure out if anything shall glow at all

			uint8_t n = (uint8_t) ( (can_msg_incoming.COMMAND & CMND_UTIL_LED) & LEDS);			// translate numeric LED ID from command to LED on device
			LED.leds[n].on = (can_msg_incoming.ARGUMENT & LED_STATE_MASK);						// set the state command for that particular LED
			LED.leds[n].blink_count = (can_msg_incoming.ARGUMENT & LED_BLINK_MASK);				// set the blink command for that particular LED
			return;
		}
		#endif
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
			can_sleep(in_can, 1);										// put to sleep
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
			helper_handle_rx(in_can);									// handle the message
			in_can->BitModify(EFLG, _BV(RX0OVR), 0x00);					// clear the overflow bit
			in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);				// clear the error interrupt flag
			return;
		}

		if (in_can->eflg & _BV(RX1OVR))									// RXB1 overflow - datasheet figure 4.3, p. 26
		{
			helper_handle_rx(in_can);									// handle the message
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

		switch (can->icod)												// handling of cases depending on ICOD value - sort of priority-style

		{																// while loops over ICOD bit values, each case handles an ICOD situation
			case 0:														// no interrupt
				break;

			case 1:														// error interrupt
				// FIXME - refactor away
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
				helper_handle_rx(can);
				break;

			case 7:														// RXB1 interrupt
				helper_handle_rx(can);
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
//
	//mj828.led->leds[blue].on = mj828.button[0].toggle;
	//mj828.led->leds[yellow].on = mj828.button[1].is_pressed;
	//mj828.led->leds[red].blink_count = (mj828.button[0].hold_error || mj828.button[1].hold_error);
	//mj828.led->leds[battery_led1].on = mj828.button[0].hold_temp;
	//mj828.led->leds[battery_led2].on = mj828.button[1].hold_temp;

	sleep_enable();														// back to sleep
}

#if ( defined(MJ808_) | defined(MJ828_) )								// ISR for timers 1 A compare match - button handling
ISR(TIMER1_COMPA_vect)													// timer/counter 1 - button debounce - foo ms
{
	sleep_disable();													// wakey wakey

	// code to be executed every 25ms

	#if defined(MJ808_)													// pushbutton code for mj808
	button_debounce(&device.button[0]);									// from here on the button is debounced and states can be consumed

	if (device.button[0].hold_error)
		util_led(UTIL_LED_RED_BLINK_6X);

	// FIXME - on really long button press (far beyond hold error) something writes crap into memory, i.e. the address of PIND in button struct gets overwritten, as does the adders of the led struct
	if (!flag_lamp_is_on && device.button[0].hold_temp)					// turn front light on
	{
		if (canbus_status.devices._MJ818)								// if rear light is present
		{
			can_msg_outgoing.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT);	// assemble appropriate command
			can_msg_outgoing.ARGUMENT = 0xFF;							// argument to turn on
			can_msg_outgoing.dlc = 2;
			device.mj8x8->can->SendMessage(&can_msg_outgoing);
		}

		if (canbus_status.devices._MJ828)								// dashboard is present
		{
			// TODO - write dashboard code
			;

			//CAN_OUT.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT); // assemble appropriate command
			//CAN_OUT.ARGUMENT = 0xFF;									// argument to turn on
			//CAN_OUT.dlc = 2;
			//mcp2515_can_msg_send(&CAN_OUT);
		}

		fade(0x20, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);

		util_led(UTIL_LED_GREEN_ON);									// power on green LED
		msg_button(&can_msg_outgoing, BUTTON0_ON);						// convey button press via CAN
		flag_lamp_is_on = 1;
	}

	if ((flag_lamp_is_on && !device.button[0].hold_temp) || device.button->hold_error)	// turn front light off
	{
		if (canbus_status.devices._MJ818)								// if rear light is present
		{
			can_msg_outgoing.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT);	// assemble appropriate command
			can_msg_outgoing.ARGUMENT = 0x00;							// argument to turn off
			can_msg_outgoing.dlc = 2;
			device.mj8x8->can->SendMessage(&can_msg_outgoing);
		}

		fade(0x00, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);				// turn off

		util_led(UTIL_LED_GREEN_OFF);									// power off green LED
		msg_button(&can_msg_outgoing, BUTTON0_OFF);						// convey button release via CAN
		flag_lamp_is_on = 0;
	}
	#endif

	#if defined(MJ828_)													// pushbutton code for mj828
	button_debounce(&device.button[0]);									// from here on the button is debounced and states can be consumed
	button_debounce(&device.button[1]);									// ditto

	device.led->leds[blue].on = device.button[0].toggle;
	device.led->leds[yellow].on = device.button[1].is_pressed;
	device.led->leds[red].blink_count = (device.button[0].hold_error || device.button[1].hold_error);
	device.led->leds[battery_led1].on = device.button[0].hold_temp;
	device.led->leds[battery_led2].on = device.button[1].hold_temp;
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

ISR(WDT_OVERFLOW_vect, ISR_NOBLOCK)										// TODO - state machine - active CAN bus device discovery & default operation on empty bus
{

	// TODO - implement sleep cycles for processor and CAN bus hardware
	sleep_disable();													// wakey wakey

	//*(device.mcu->wdtcr) |= _BV(WDIE);								// setting the bit prevents a reset when the timer expires
	WDTCR |= _BV(WDIE);													// setting the bit prevents a reset when the timer expires

	//if (gpio_tst(MCP2561_standby_pin))								// if in sleep...
	//{
		//gpio_clr(MCP2561_standby_pin);								// put pin to low -> ...wakeup of MCP2561
		//mcp2515_opcode_bit_modify(CANINTF, _BV(WAKIF), _BV(WAKIF));	// wakeup - put into normal mode
	//}

	if (canbus_status.status & 0x80)									// discovery mode, once on power up and 1s-periodic
		discovery_announce(&canbus_status, &can_msg_outgoing);
	else																// not discovery mode - every 1s
		discovery_behave(&canbus_status);								// behave according to what was announced

	++canbus_status.sleep_iteration;

	sleep_enable();														// back to sleep
}