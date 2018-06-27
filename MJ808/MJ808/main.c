#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define MJ808_ // what device to compile for?

#include "gpio.h"
#include "gpio_definitions.h"

#include "mcp2515.h"
#include "mj8x8.h"

// global variables
#if defined(MJ808_)
volatile uint8_t flag_lamp_is_on = 0; // flag - indicates if button turned the device on, used for pushbutton handling
volatile uint8_t counter_button_press_time = 0; // holds the counter value at button press, used for pushbutton debouncing
#endif

// variables populated in INT1_vect() ISR by means of CAN interrupts


#if defined(MJ808_)
/*
 * self, template of an outgoing CAN message; SID intialized to this device
 * NOTE:
 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
 */

can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK) // low byte
};
#endif

#if defined(MJ818_)
can_message_t CAN_OUT =
{
	.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B), // high byte
	.sidl = ( RCPT_DEV_BLANK | BLANK) // low byte
};
#endif

can_message_t CAN_IN; // structure holding an incoming CAN message

int main(void)
{
	#include "gpio_modes.h"

	PRR = _BV(PRUSART);	// turn off USART, we don't need it

	cli();	// clear interrupts globally

	// setup of INT1  - handled via INT1_vect ISR
	MCUCR = _BV(ISC11); // a falling edge generates an IRQ
	GIMSK = _BV(INT1);	// enable INT1

	#if defined(MJ808_)
	// setup of interrupt-driven timer
	OCR0A = 0xFC; // fires every ~32.5ms
	TCCR0A = _BV(WGM01); // CTC mode
	TIFR |= _BV(OCF0A); // clear interrupt flag
	TIMSK = _BV(OCIE0A); // TCO compare match IRQ enable
	TCCR0B = ( _BV(CS02) | _BV(CS00) ); // clkIO/1024 (from prescaler)

	// setup of front light PWM
	//TODO - test with 16bit value
	OCR1A = 0x00;	// hex value PWM counter - start with light turned off by default
	TCCR1A = (_BV(COM1A1) | // Clear OC1A/OC1B on Compare Match when up counting
	_BV(WGM10)); // phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10); // clock prescaler: clk/8
	#endif

	#if defined(MJ818_)
	// setup of rear light PWM
	OCR0A = 0x00;	// hex value PWM counter - start with light turned off by default
	TCCR0A = ( _BV(COM0A1)| // Clear OC1A/OC1B on Compare Match when up counting
	_BV(WGM00) );	// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);		// clock prescaler: clk/8

	// setup of brake light PWM
	OCR1A = 0x00;	// hex value PWM counter - start with light turned off by default
	TCCR1A = (_BV(COM1A1) | // Clear OC1A/OC1B on Compare Match when up counting
	_BV(WGM10)); // phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10); // clock prescaler: clk/8
	#endif

	sei();	// enable interrupts globally

	mcp2515_init(); // initialize & configure the MCP2515
	//TODO - state machine - active CAN bus device discovery & default operation on empty bus

#if defined(MJ808_)
	util_led(UTIL_LED_RED_BLINK_1X); // startup indicator - blinks green 2 times
	util_led(UTIL_LED_GREEN_BLINK_1X); // startup indicator - blinks green 2 times
	util_led(UTIL_LED_RED_BLINK_1X); // startup indicator - blinks green 2 times
#endif

	while (1) // forever loop
	{
		// on purpose kept as empty as possible !!
		//asm("nop");
	}
}

// ISR for INT1 - triggered by CAN message reception of the MCP2515
ISR(INT1_vect)
{
	uint8_t canintf; // interrupt flag register

	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	mcp2515_opcode_read_bytes(CANINTF, &canintf, 1); // download the interrupt flag register

	if (canintf & _BV(WAKIF)) // if we detect a wake interrupt
	{
		//TODO: device handling immediately after message wakes up controller
		mcp2515_opcode_bit_modify(CANCTRL, 0xE0, 0x00); // put into normal mode
		mcp2515_opcode_bit_modify(CANINTF, _BV(WAKIF), 0x00); // simply clear the flag
		return;
	}

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

	if (canintf & (_BV(MERRF) )) //TODO - implement message error handling
	{
		mcp2515_opcode_bit_modify(CANINTF, _BV(MERRF), 0x00); // clear the flag
	}

	if ( canintf & (_BV(RX1IF) | _BV(RX0IF)) ) // if we received a message
	{
		mcp2515_can_msg_receive(&CAN_IN); // load the CAN message into its structure

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

#if defined(MJ808_)
// ISR for timer 0 A compare match
ISR(TIMER0_COMPA_vect)
{
	// code to be executed every 32.5ms

	// pushbutton debounce in software. yuk! implemented via counters in a timer interrupt
	if (gpio_tst(PUSHBUTTON_pin)) // if button is pressed
	{
		counter_button_press_time++; // start the counter

		if (flag_lamp_is_on && counter_button_press_time > 50) // longer press - turn off
		{
			util_led(UTIL_LED_GREEN_OFF); // power off green LED
			msg_button(&CAN_OUT, BUTTON0_OFF); // convey button release via CAN
			counter_button_press_time = 0; // reset the counter
			flag_lamp_is_on = 0;
		}

		if (!flag_lamp_is_on && counter_button_press_time > 20) // shorter press - turn on
		{
			//gpio_conf(GREEN_LED_pin, OUTPUT, LOW); // power on green LED
			util_led(UTIL_LED_GREEN_ON); // power on green LED
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