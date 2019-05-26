#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define MJ808_															// what device to compile for?

#include "gpio.h"														// macros for pin definitions
#include "gpio_definitions.h"											// pin layout

#include "mcp2515.h"													// CAN driver
#include "mj8x8.h"														// MJ-specific functions

volatile uint8_t flag_lamp_is_on = 0;									// flag - indicates if button turned the device on, used for pushbutton handling

/*
 * self, template of an outgoing CAN message; SID intialized to this device
 * NOTE:
 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
 */
#if defined(MJ808_)														// CAN_OUT SID for front light
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK)									// low byte
};
#endif

#if defined(MJ818_)														// CAN_OUT SID for rear light
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK)									// low byte
};
#endif

#if defined(MJ828_)														// CAN_OUT SID for dashboard
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK)									// low byte
};
#endif

can_message_t CAN_IN;													// structure holding an incoming CAN message

volatile canbus_t canbus_status =										// bit-wise info about CAN bus status
{
	.status = 0x80,														// bitwise representation of CAN bus status, start with discovery mode on
	.devices.uint16_val = 0x0000 ,										// bitwise representation of devices discovered, see #defines below CMND_ANNOUNCE
	.broadcast_iteration_count = 0, 									// counter
	.numerical_self_id = 0,												// decimal representation of SID
	.sleep_iteration = 0												// counter for sleep iterations, incremented by WDT_OVERFLOW_vect() ISR
};

volatile	uint8_t canintf; // interrupt flag register
volatile	uint8_t canstat;
volatile	uint8_t canctrl;
volatile	uint8_t eflg;
volatile	uint8_t rec;
volatile	uint8_t tec;

int main(void)
{
	#include "gpio_modes.h"												// GPIO state definitions

	PRR = _BV(PRUSART);													// turn off USART, we don't need it
	ACSR = _BV(ACD);													// turn off the analog comparator, we don't need it either

	cli();																// clear interrupts globally

	#if defined(MJ808_)													// OCR init for front light - have light off
	OCR_FRONT_LIGHT = 0x00;
	#endif
	#if defined(MJ818_)													// OCR init for rear lights - have lights off
	OCR_REAR_LIGHT = 0x00;												// rear light
	OCR_BRAKE_LIGHT = 0x00;												// brake light
	#endif

	#if ( defined(MJ808_) | defined(MJ828_) )							// timer/counter1 - 16bit (and timer/counter0 - 8bit) - pushbutton timing (charlieplex timing)
	/* timing of OCR1A in ms
		0xffff - 65.4ms
		0x6180 - 25ms
		0x2780 - 10ms
	*/
	OCR1A = 0x6180;														// 0x6180 - 25ms - counter increment up to this value
	TIFR |= _BV(OCF1A);													// clear interrupt flag
	TIMSK = _BV(OCIE1A);												// TCO compare match IRQ enable for OCIE1A
	TCCR1B = ( _BV(WGM12) |												// CTC mode w. TOP = OCR1A, TOV1 set to MAX
			   _BV(CS11)  );											// clkIO/8 (from prescaler), start timer

	#if defined(MJ828_)													// timer/counter0 - 8bit - charlie-plexing timer - 25ms
	/* timing of OCR0A in ms
		0xff - 32.5ms
		0x0f - 16.25ms
	*/
	OCR0A = 0x0F;														// 0x0f - 16.25ms, counter increment up to this value
	TCCR0A = _BV(WGM01);												// CTC mode w. TOP = OCR0A, TOV1 set to MAX
	TIMSK |= _BV(OCIE0A);												// additionally enable TCO compare match IRQ enable for OCIE0A
	TCCR0B = ( _BV(CS02) |
			   _BV(CS00) );												// clkIO/1024 (from prescaler), start timer
	#endif

	#endif

	#if defined(MJ818_)													// timer/counter1 - 16bit - brake light PWM
	TCCR1A = (_BV(COM1A1) |												// Clear OC1A/OC1B on Compare Match when up counting
			  _BV(WGM10));												// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10);													// clock prescaler: clk/1 (no pre-scaling)
	#endif

	#if ( defined(MJ808_) | defined(MJ818_) )							// timer/counter0 - 8bit - rear light or front light PWM
	TCCR0A = ( _BV(COM0A1)|												// Clear OC1A/OC1B on Compare Match when up counting
			   _BV(WGM00) );											// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock prescaler: clk/8
	#endif

	if(MCUSR & _BV(WDRF))												// power-up - if we got reset by the watchdog...
	{
		MCUSR &= ~_BV(WDRF);											// clear the reset flag
		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = 0x00;													// disable the thing completely
	}

	// setup of INT1  - handled via INT1_vect ISR
	MCUCR = _BV(ISC11);													// a falling edge generates an IRQ
	GIMSK = ( _BV(INT1)	|												// enable INT1
			  _BV(PCIE2));												// enable pin change IRQ for PCTIN17-11 (further specified in PCMSK2)

	#if defined(MJ808_)													// TODO - setup of pin change interrupts for pushbuttons
	PCMSK2 = _BV(PCINT15);												// enable pin change for sw @ pin D4
	#endif
	#if defined(MJ828_)													// TODO - setup of pin change interrupts for pushbuttons
	PCMSK2 = (_BV(PCINT11) |											// enable pin change for sw1 @ pin D0
			  _BV(PCINT12));											// enable pin change for sw2 @ pin D1
	#endif

	WDTCR |= (_BV(WDCE) | _BV(WDE));									// WDT change enable sequence
	WDTCR = ( _BV(WDIE) | _BV(WDP2)  );									// watchdog timer set to 0.25s

	sei();																// enable interrupts globally

	can_t CAN;															// declare CAN object - its address will be used soon in the constructor ...

	canbus_status.numerical_self_id = (uint8_t) ( (CAN_OUT.sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	#if defined(MJ808_)													// device init for MJ808
	mj808.led = &LED;													// pass reference to LED struct
	mj808.can = can_ctor(&CAN);											// pass CAN address into constructor, returns basically itself

	LED.led_count = 2;

	mj808.button[0].PIN = (uint8_t *) 0x30; 							// 0x020 offset plus address - PIND register
	mj808.button[0].pin_number = 4;										// sw2 is connected to pin D0
	#endif

	#if defined(MJ818_)													// device init for MJ818
	mj818.led = &LED;													// pass reference to LED struct
	mj818.can = can_ctor(&CAN);											// pass CAN address into constructor, returns basically itself

	LED.led_count = 2;
	#endif

	#if defined(MJ828_)													// device init for MJ828
	mj828.led = &LED;													// pass reference to LED struct
	mj828.can = can_ctor(&CAN);											// pass CAN address into constructor, returns basically itself

	LED.led_count = 7;
	LED.flag_any_glow = 1;
	LED.leds[green].on = 1;

	mj828.button[0].pin_number = 0;										// sw2 is connected to pin D0
	mj828.button[0].pin_number = 1;										// sw2 is connected to pin D1
	mj828.button[0].PIN = (uint8_t *) 0x30;								// 0x020 offset plus address - PIND register
	mj828.button[1].PIN = (uint8_t *) 0x30;								// ditto
	#endif



	// TODO - implement micro controller sleep cycles
	set_sleep_mode(SLEEP_MODE_IDLE);									// 11mA
	//set_sleep_mode(SLEEP_MODE_STANDBY);								// 10mA
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN);								// 10mA
	sleep_enable();
	sleep_cpu();

	#if defined(MJ808_) // crude power indicator - blink green LED once
	util_led(UTIL_LED_GREEN_BLINK_1X);
	#endif

	while (1)															// forever loop
	{
		asm("nop");														// on purpose kept as empty as possible !!
		// FIXME - refactor away

		#if defined(MJ808_)
		mj808.can->ReadBytes(CANINTF, &canintf, 1);						// download the interrupt flag register
		mj808.can->ReadBytes(CANSTAT, &canstat, 1);
		mj808.can->ReadBytes(CANSTAT, &canctrl, 1);
		mj808.can->ReadBytes(EFLG, &eflg, 1);
		mj808.can->ReadBytes(REC, &rec, 1);
		mj808.can->ReadBytes(TEC, &tec, 1);
		#endif

		#if defined(MJ818_)
		mj818.can->ReadBytes(CANINTF, &canintf, 1);						// download the interrupt flag register
		mj818.can->ReadBytes(CANSTAT, &canstat, 1);
		mj818.can->ReadBytes(CANSTAT, &canctrl, 1);
		mj818.can->ReadBytes(EFLG, &eflg, 1);
		mj818.can->ReadBytes(REC, &rec, 1);
		mj818.can->ReadBytes(TEC, &tec, 1);
		#endif

		#if defined(MJ828_)
		mj828.can->ReadBytes(CANINTF, &canintf, 1);						// download the interrupt flag register
		mj828.can->ReadBytes(CANSTAT, &canstat, 1);
		mj828.can->ReadBytes(CANSTAT, &canctrl, 1);
		mj828.can->ReadBytes(EFLG, &eflg, 1);
		mj828.can->ReadBytes(REC, &rec, 1);
		mj828.can->ReadBytes(TEC, &tec, 1);
		#endif
		if (MCUCR & _BV(SE))											// if sleep is enabled
			sleep_cpu();												// ...sleep
	}
}


ISR(INT1_vect)															// ISR for INT1 - triggered by CAN message reception of the MCP2515
{
	sleep_disable();													// wakey wakey

	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	void handle_message_error(volatile can_t *in_can)					// handles message error interrupts
	{
		in_can->BitModify(CANINTF, _BV(MERRF), 0x00);					// clear the flag
	};

	void helper_handle_rx(volatile can_t *in_can)						// handles incoming message interrupts
	{
		in_can->ReceiveMessage(&CAN_IN);								// load the CAN message into its structure & clear the RX int flag

		// update the CAN BUS status structure; if we get a message from SID foo, then _BV(foo) shall be marked as "on the bus"
		canbus_status.devices.uint16_val |= ( 1 << ( (CAN_IN.sidh >> 2) & 0x0F ) ); // shift as many bits as the originating SID is in decimal

		// command for device
		if (CAN_IN.COMMAND & CMND_DEVICE)								//  we received a command for some device...
		{
			#if defined(SENSOR)
			if ((CAN_IN.COMMAND & DEV_SENSOR) == DEV_SENSOR)			// ...a sensor
			{
				dev_sensor(&CAN_IN);									// deal with it
				return;
			}
			#endif

			#if defined(MJ808_) || defined(MJ818_)
			if (CAN_IN.COMMAND & ( CMND_DEVICE | DEV_LIGHT ) )			// ...a LED device
			{
				dev_light(&CAN_IN);										// deal with it
				return;
			}
			#endif

			#if defined(PWR_SRC)
			if ((CAN_IN.COMMAND & DEV_PWR_SRC) == DEV_PWR_SRC)			// ...a power source
			{
				dev_pwr_src(&CAN_IN);									// deal with it
				return;
			}
			#endif

			#if defined(LOGIC_UNIT)
			if ((CAN_IN.COMMAND & DEV_LU) == DEV_LU)					// ...a logic unit
			{
				dev_logic_unit(&CAN_IN);								// deal with it
				return;
			}
			#endif
		}

		#if defined(MJ808_)
		if ( (CAN_IN.COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)			// utility LED command
		{
			util_led(CAN_IN.COMMAND);									// blinky thingy
			return;
		}
		#endif

		#if defined(MJ828_)
		if ( (CAN_IN.COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)			// utility LED command
		{
			return;														// HACK - can be removed once CMND_UTIL_LED is of new command structure

			if (CAN_IN.ARGUMENT == 0)
			return;

			LED.flag_any_glow = (CAN_IN.ARGUMENT & ( LED_STATE_MASK | LED_BLINK_MASK) ); // figure out if anything shall glow at all

			uint8_t n = (uint8_t) ( (CAN_IN.COMMAND & CMND_UTIL_LED) & LEDS);			// translate numeric LED ID from command to LED on device
			LED.leds[n].on = (CAN_IN.ARGUMENT & LED_STATE_MASK);						// set the state command for that particular LED
			LED.leds[n].blink_count = (CAN_IN.ARGUMENT & LED_BLINK_MASK);				// set the blink command for that particular LED
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
		#if defined(MJ808_)												// read in of CAN registers for MJ808
		mj808.can->ReadBytes(TEC, &mj808.can->tec, 2);					// read in TEC and REC
		mj808.can->ReadBytes(CANINTF, &mj808.can->canintf, 3);			// read in CANINTF and EFLG
		mj808.can->ReadBytes(CANCTRL, &mj808.can->canctrl, 1);

		mj808.can->icod =  ((mj808.can->canstat & 0x0E) >> 1);			// right shift so that CANSTAT.U0 cant interfere

		switch (mj808.can->icod)										// handling of cases depending on ICOD value - sort of priority-style
		#endif

		#if defined(MJ818_)												// read in of CAN registers for MJ818
		mj818.can->ReadBytes(TEC, &mj818.can->tec, 2);					// read in TEC and REC
		mj818.can->ReadBytes(CANINTF, &mj818.can->canintf, 3);			// read in CANINTF and EFLG
		mj818.can->ReadBytes(CANCTRL, &mj818.can->canctrl, 1);

		mj818.can->icod =  ((mj818.can->canstat & 0x0E) >> 1);			// right shift so that CANSTAT.U0 cant interfere

		switch (mj818.can->icod)										// handling of cases depending on ICOD value - sort of priority-style
		#endif

		#if defined(MJ828_)												// read in of CAN registers for MJ828
		mcp2515_opcode_read_bytes(TEC, &mj828.can->tec, 2);				// read in TEC and REC
		mcp2515_opcode_read_bytes(CANINTF, &mj828.can->canintf, 3);		// read in CANINTF and EFLG
		mcp2515_opcode_read_bytes(CANCTRL, &mj828.can->canctrl, 1);

		mj828.can->icod =  ((mj828.can->canstat & 0x0E) >> 1);			// right shift so that CANSTAT.U0 cant interfere

		switch (mj828.can->icod)										// handling of cases depending on ICOD value - sort of priority-style
		#endif

		{																// while loops over ICOD bit values, each case handles an ICOD situation
			case 0:														// no interrupt
				break;

			case 1:														// error interrupt
				// FIXME - refactor away
				#if defined(MJ808_)
				helper_handle_error(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_error(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_error(mj828.can);
				#endif
				break;

			case 2:														// wake-up interrupt
				#if defined(MJ808_)
				helper_handle_wakeup(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_wakeup(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_wakeup(mj828.can);
				#endif
				break;

			case 3:														// TXB0 interrupt
				#if defined(MJ808_)
				helper_handle_tx(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_tx(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_tx(mj828.can);
				#endif
				break;

			case 4:														// TXB1 interrupt
				#if defined(MJ808_)
				helper_handle_tx(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_tx(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_tx(mj828.can);
				#endif
				break;

			case 5:														// TXB2 interrupt
				#if defined(MJ808_)
				helper_handle_tx(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_tx(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_tx(mj828.can);
				#endif
				break;

			case 6:														// RXB0 interrupt
				#if defined(MJ808_)
				helper_handle_rx(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_rx(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_rx(mj828.can);
				#endif
				break;

			case 7:														// RXB1 interrupt
				#if defined(MJ808_)
				helper_handle_rx(mj808.can);
				#endif
				#if defined(MJ818_)
				helper_handle_rx(mj818.can);
				#endif
				#if defined(MJ828_)
				helper_handle_rx(mj828.can);
				#endif
				break;
		};

	#if defined(MJ808_)
	} while (mj808.can->icod);
	#endif
	#if defined(MJ818_)
	} while (mj818.can->icod);
	#endif
	#if defined(MJ828_)
	} while (mj828.can->icod);
	#endif

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
	button_debounce(&mj808.button[0]);									// from here on the button is debounced and states can be consumed

	if (mj808.button[0].hold_error)
		util_led(UTIL_LED_RED_BLINK_6X);

	// FIXME - on really long button press (far beyond hold error) something writes crap into memory, i.e. the address of PIND in button struct gets overwritten, as does the adders of the led struct
	if (!flag_lamp_is_on && mj808.button[0].hold_temp)					// turn front light on
	{
		if (canbus_status.devices._MJ818)								// if rear light is present
		{
			CAN_OUT.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT);	// assemble appropriate command
			CAN_OUT.ARGUMENT = 0xFF;									// argument to turn on
			CAN_OUT.dlc = 2;
			mj808.can->SendMessage(&CAN_OUT);
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
		msg_button(&CAN_OUT, BUTTON0_ON);								// convey button press via CAN
		flag_lamp_is_on = 1;
	}

	if ((flag_lamp_is_on && !mj808.button[0].hold_temp) || mj808.button->hold_error)	// turn front light off
	{
		if (canbus_status.devices._MJ818)								// if rear light is present
		{
			CAN_OUT.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT);	// assemble appropriate command
			CAN_OUT.ARGUMENT = 0x00;									// argument to turn off
			CAN_OUT.dlc = 2;
			mj808.can->SendMessage(&CAN_OUT);
		}

		fade(0x00, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);				// turn off

		util_led(UTIL_LED_GREEN_OFF);									// power off green LED
		msg_button(&CAN_OUT, BUTTON0_OFF);								// convey button release via CAN
		flag_lamp_is_on = 0;
	}
	#endif

	#if defined(MJ828_)													// pushbutton code for mj828
	button_debounce(&mj828.button[0]);									// from here on the button is debounced and states can be consumed
	button_debounce(&mj828.button[1]);									// ditto

	mj828.led->leds[blue].on = mj828.button[0].toggle;
	mj828.led->leds[yellow].on = mj828.button[1].is_pressed;
	mj828.led->leds[red].blink_count = (mj828.button[0].hold_error || mj828.button[1].hold_error);
	mj828.led->leds[battery_led1].on = mj828.button[0].hold_temp;
	mj828.led->leds[battery_led2].on = mj828.button[1].hold_temp;
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

	WDTCR |= _BV(WDIE);													// setting the bit prevents a reset when the timer expires

	//if (gpio_tst(MCP2561_standby_pin))								// if in sleep...
	//{
		//gpio_clr(MCP2561_standby_pin);								// put pin to low -> ...wakeup of MCP2561
		//mcp2515_opcode_bit_modify(CANINTF, _BV(WAKIF), _BV(WAKIF));	// wakeup - put into normal mode
	//}

	if (canbus_status.status & 0x80)									// discovery mode, once on power up and 1s-periodic
		discovery_announce(&canbus_status, &CAN_OUT);
	else																// not discovery mode - every 1s
		discovery_behave(&canbus_status);								// behave according to what was announced

	++canbus_status.sleep_iteration;

	sleep_enable();														// back to sleep
}