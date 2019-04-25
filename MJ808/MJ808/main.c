#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define MJ808_ // what device to compile for?

#include "gpio.h"	// macros for pin definitions
#include "gpio_definitions.h" // pin layout

#include "mcp2515.h"	// CAN driver
#include "mj8x8.h"	// MJ-specific functions

// global variables
#if ( defined(MJ808_) | defined(MJ828_) ) // flags for front light
volatile uint8_t flag_lamp_is_on = 0; // flag - indicates if button turned the device on, used for pushbutton handling
volatile uint8_t counter_button_press_time = 0; // holds the counter value at button press, used for pushbutton debouncing
#endif

/*
 * self, template of an outgoing CAN message; SID intialized to this device
 * NOTE:
 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
 */
#if defined(MJ808_) // CAN_OUT SID for front light
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK) // low byte
};
#endif

#if defined(MJ818_) // CAN_OUT SID for rear light
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK) // low byte
};
#endif

#if defined(MJ828_) // CAN_OUT SID for dashboard
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK) // low byte
};
#endif

can_message_t CAN_IN; // structure holding an incoming CAN message

canbus canbus_status = // bit-wise info about CAN bus status
{
	.status = 0x80, // bitwise representation of CAN bus status, start with discovery mode on
	.devices.all = 0x0000 , // bitwise representation of devices discovered, see #defines below CMND_ANNOUNCE
	.broadcast_iteration_count = 0, // counter
	.numerical_self_id = 0, //
	.sleep_iteration = 0
};

int main(void)
{
	#include "gpio_modes.h" // GPIO state definitions

	PRR = _BV(PRUSART);		// turn off USART, we don't need it
	ACSR = _BV(ACD);		// turn off ADC, we don't need it either

	cli();	// clear interrupts globally

	#if defined(MJ808_) // OCR init for rear light - have light off
	OCR_FRONT_LIGHT = 0x00;
	#endif
	#if defined(MJ818_) // OCR init for rear light - have light off
	OCR_REAR_LIGHT = 0x00;
	OCR_BRAKE_LIGHT = 0x00;
	#endif

	#if ( defined(MJ808_) | defined(MJ828_) ) // OCR1A setup of timer-driven interrupt, mostly for SW button debouncing
	OCR1A = 0x6180;			// counter increment up to this value, at which CTC's MAX is reached - corresponds to 50ms w. prescaler at clkIO/8
	TIFR |= _BV(OCF1A);		// clear interrupt flag
	TIMSK = _BV(OCIE1A);	// TCO compare match IRQ enable
	TCCR1B = ( _BV(WGM12) |	// CTC mode w. TOP = OCR1A, TOV1 set to MAX
			   _BV(CS11)  ); // clkIO/8 (from prescaler), start timer
	#endif

	#if defined(MJ818_)	// timer/counter 1 - 16bit - PB3 - brake light PWM
	TCCR1A = (_BV(COM1A1) |	// Clear OC1A/OC1B on Compare Match when up counting
			  _BV(WGM10));	// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10);		// clock prescaler: clk/1 (no pre-scaling)
	#endif

	#if ( defined(MJ808_) | defined(MJ818_) ) // timer/counter 0 - 8bit - PB2 - rear light / front light PWM
	TCCR0A = ( _BV(COM0A1)|		// Clear OC1A/OC1B on Compare Match when up counting
			   _BV(WGM00) );	// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);			// clock prescaler: clk/8
	#endif

	if(MCUSR & _BV(WDRF)) // power-up - if we got reset by the watchdog...
	{
		MCUSR &= ~_BV(WDRF);				// clear the reset flag
		WDTCR |= (_BV(WDCE) | _BV(WDE));	// WDT change enable sequence
		WDTCR = 0x00;						// disable the thing completely
	}

	// setup of INT1  - handled via INT1_vect ISR
	MCUCR = _BV(ISC11); // a falling edge generates an IRQ
	GIMSK = _BV(INT1);	// enable INT1

	WDTCR |= (_BV(WDCE) | _BV(WDE));		// WDT change enable sequence
	WDTCR = ( _BV(WDIE) | _BV(WDP2)  );		// watchdog timer set to 0.25s

	sei();	// enable interrupts globally

	mcp2515_init(); // initialize & configure the MCP2515

	canbus_status.numerical_self_id = (uint8_t) ( (CAN_OUT.sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	// TODO - implement micro controller sleep cycles
	//set_sleep_mode(SLEEP_MODE_IDLE);
	//sleep_enable();
	//sleep_cpu();

	#if defined(MJ808_) // crude power indicator - blink green LED once
	util_led(UTIL_LED_GREEN_BLINK_1X);
	#endif

	while (1) // forever loop
	{
		// on purpose kept as empty as possible !!
		asm("nop");

		//if (MCUCR & _BV(SE)) // if sleep is enabled
			//sleep_cpu(); // ...sleep
	}
}


ISR(INT1_vect) // ISR for INT1 - triggered by CAN message reception of the MCP2515
{
	uint8_t canintf; // interrupt flag register

	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	mcp2515_opcode_read_bytes(CANINTF, &canintf, 1); // download the interrupt flag register

	// wake interrupt
	if (canintf & _BV(WAKIF))
	{
		mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x00); // put into normal mode
		mcp2515_opcode_bit_modify(CANINTF, _BV(WAKIF), 0x00); // simply clear the flag
		return;
	}

	// general error interrupt
	if (canintf & (_BV(ERRIF) )) //TODO - implement general error handling
	{
		uint8_t errflag;
		mcp2515_opcode_read_bytes(EFLG, &errflag, 1); // read EFLG register

		if (errflag & _BV(RX0OVR)) // RXB0 overflow - datasheet figure 4.3, p. 26
			mcp2515_opcode_bit_modify(EFLG, _BV(RX0OVR), 0x00); // clear the bit

		if (errflag & _BV(RX1OVR)) // RXB1 overflow - datasheet figure 4.3, p. 26
			mcp2515_opcode_bit_modify(EFLG, _BV(RX1OVR), 0x00); // clear the bit

		mcp2515_opcode_bit_modify(CANINTF, _BV(ERRIF), 0x00); // clear the flag
	}

	// message error interrupt
	if (canintf & (_BV(MERRF) )) //TODO - implement message error handling
	{
		#if defined(MJ808_)
		util_led(UTIL_LED_RED_BLINK_1X); // FIXME - implement bus error handling
		#endif
		mcp2515_opcode_bit_modify(CANINTF, _BV(MERRF), 0x00); // clear the flag
	}

	// general message handling
	if ( canintf & (_BV(RX1IF) | _BV(RX0IF)) ) // if we received a message
	{
		mcp2515_can_msg_receive(&CAN_IN); // load the CAN message into its structure

		// update the CAN BUS status structure; if we get a message from SID foo, then _BV(foo) shall be marked as "on the bus"
		canbus_status.devices.all |= ( 1 << ( (CAN_IN.sidh >> 2) & 0x0F ) ); // shift as many bits as the originating SID is in decimal

		// command for device
		if (CAN_IN.COMMAND & CMND_DEVICE) //  we received a command for some device...
		{
			#if defined(SENSOR)
			if ((CAN_IN.COMMAND & DEV_SENSOR) == DEV_SENSOR) // ...a sensor
			{
				dev_sensor(&CAN_IN); // deal with it
				return;
			}
			#endif

			#if defined(MJ808_) || defined(MJ818_)
			if (CAN_IN.COMMAND & ( CMND_DEVICE | DEV_LIGHT ) ) // ...a LED device
			{
				dev_light(&CAN_IN); // deal with it
				return;
			}
			#endif

			#if defined(PWR_SRC)
			if ((CAN_IN.COMMAND & DEV_PWR_SRC) == DEV_PWR_SRC) // ...a power source
			{
				dev_pwr_src(&CAN_IN); // deal with it
				return;
			}
			#endif

			#if defined(LOGIC_UNIT)
			if ((CAN_IN.COMMAND & DEV_LU) == DEV_LU) // ...a logic unit
			{
				dev_logic_unit(&CAN_IN); // deal with it
				return;
			}
			#endif
		}

		#if defined(MJ808_)
		if ( (CAN_IN.COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED) // utility LED command
		{
			util_led(CAN_IN.COMMAND); // blinky thingy
			return;
		}
		#endif
	}
}

#if ( defined(MJ808_) | defined(MJ828_) ) // ISR for timer 0 A compare match - button handling
ISR(TIMER1_COMPA_vect)
{
	// code to be executed every 32.5ms

	// pushbutton debounce in software. yuk! implemented via counters in a timer interrupt
	#if defined(MJ808_)
	if (gpio_tst(PUSHBUTTON_pin)) // if button is pressed
	#endif
	#if defined(MJ828_)
	if (!gpio_tst(PUSHBUTTON1_pin)) // if button is pressed
	#endif
	{
		counter_button_press_time++; // start the counter

		if (flag_lamp_is_on && counter_button_press_time > 50) // longer press - turn off
		{
			#if defined(MJ808_) // button handling if we are a dumb front light
			if (canbus_status.devices._MJ818) // only the rear and front lights are on the bus
			{
				CAN_OUT.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT); // assemble appropriate command
				CAN_OUT.ARGUMENT = 0x00; // argument to turn off
				CAN_OUT.dlc = 2;
				mcp2515_can_msg_send(&CAN_OUT);
				fade(0x00, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);
			}

			if (canbus_status.devices.all == 0x00) // only the front light is on the bus
			{
				fade(0x00, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT); // turn off
			}
			util_led(UTIL_LED_GREEN_OFF); // power off green LED
			#endif

			msg_button(&CAN_OUT, BUTTON0_OFF); // convey button release via CAN
			counter_button_press_time = 0; // reset the counter
			flag_lamp_is_on = 0;
		}

		if (!flag_lamp_is_on && counter_button_press_time > 20) // shorter press - turn on
		{
			#if defined(MJ808_) // button handling if we are a dumb front light
			if (canbus_status.devices._MJ818) // only the rear and front lights are on the bus
			{
				CAN_OUT.COMMAND = (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT); // assemble appropriate command
				CAN_OUT.ARGUMENT = 0xFF; // argument to turn on
				CAN_OUT.dlc = 2;
				mcp2515_can_msg_send(&CAN_OUT);
				fade(0x40, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);
			}

			if (canbus_status.devices.all == 0x00) // only the front light is on the bus
			{
				fade(0x40, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT); // turn on
			}
			util_led(UTIL_LED_GREEN_ON); // power on green LED
			#endif

			msg_button(&CAN_OUT, BUTTON0_ON); // convey button press via CAN
			counter_button_press_time = 0; // reset the counter
			flag_lamp_is_on = 1;
		}
	}
	else // not long enough press - bounce
	{
		counter_button_press_time = 0; // reset the counter
	}
}
#endif

ISR(WDT_OVERFLOW_vect, ISR_NOBLOCK) // TODO - state machine - active CAN bus device discovery & default operation on empty bus
{
	// TODO - implement sleep cycles for processor and CAN bus hardware
	//sleep_disable(); // wakey wakey
	WDTCR |= _BV(WDIE); // setting the bit prevents a reset when the timer expires

	//if (gpio_tst(MCP2561_standby_pin)) // if in sleep...
	//{
		//gpio_clr(MCP2561_standby_pin); // put pin to low -> ...wakeup of MCP2561
		//mcp2515_opcode_bit_modify(CANINTF, _BV(WAKIF), _BV(WAKIF)); // wakeup - put into normal mode
	//}

	if (canbus_status.status & 0x80) // discovery mode, once on power up and 2s-periodic
		discovery_announce(&canbus_status, &CAN_OUT);
	else // not discovery mode - every 2s
		discovery_behave(&canbus_status); // behave according to what was announced

	++canbus_status.sleep_iteration;
	//sleep_enable(); // back to sleep
}