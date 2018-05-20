#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mcp2515.h"

	/*
		timer/counter 0 and timer/counter 1 both operate in 8bit mode
		hex val. - duty cycle - current front light
		0x00 - 0% (off)				-	20 mA
		0x10 - 6.26%					-	20 mA
		0x20 - 12.5%					-	30 mA
		0x40 - 25.05%					-	60 mA
		0x80 - 50.1%					-	100 mA
		0xA0 - 62.6%					-	120 mA
		0xC0 - 75.15%					-	150 mA
		0xE0 - 87.84%					-	170 mA
		0xF0 - 94.12%					-	180 mA
		0xFF - 100% (max)			-	200 mA
*/

// 0x80 MAX. SAFE COUNT WITH REV2 BOARD
#define FRONT_OCR1A 0x80 // count to hex value
#define LOW_SPEED_THRESHOLD_FREQ 20 // dynamo frequency below which we consider the bike to move too slow for power generation -> dim light

uint8_t can_msg[11]; // holds the received CAN message
volatile uint8_t can_msg_status; // holds message status metadata
volatile uint8_t dynamo_available = 0;	// boolean: 0 - dynamo absent, 1 - dynamo detected
volatile uint8_t dynamo_freq = 0; // dynamo frequency (is NOT wheel frequency)

int main(void)
{
	#include "gpio_modes.h"

	PRR = _BV(PRUSART);	// turn off USART, we don't need it

//	mcp2515_init(); // initialize & configure the MCP2515

	//TODO - active CAN bus device discovery


	cli();	// clear interrupts globally


	// setup of INT1 and PCINT1(pin B1) - handled via PCINT0_vect ISR
	MCUCR = _BV(ISC10); // any logical change generates an IRQ
	GIMSK = _BV(INT1) | _BV(PCIE2);	// enable INT1 and PCIE2
	PCMSK2 = _BV(PCINT15); // turn on PCINT1 (pin D4)
	//PCMSK = _BV(PCINT15)

	// setup of interrupt-driven timer - fires every ~65ms
	OCR0A = 0xFC;
	TCCR0A = _BV(WGM01); // CTC mode
	TIFR |= _BV(OCF0A); // clear interrupt flag
	TIMSK = _BV(OCIE0A); // TCO compare match IRQ enable
	TCCR0B = ( _BV(CS02) | _BV(CS00) ); // clkIO/1024 (from prescaler)

	//// setup of front light PWM
	OCR1A = FRONT_OCR1A;	// count to this hex value
	TCCR1A = (_BV(COM1A1) | // clear OC1A on compare match, set OC1A at TOP
						_BV(WGM10)); // phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10); // clock prescaler: clk/8

	sei();	// enable interrupts globally

	while (1) // forever loop
	{
		// keep as empty as possible !!
		;
	}
}

// ISR for INT1
ISR(INT1_vect)
{
	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	mcp2515_can_msg_receive(can_msg); // load the can message
	can_msg_status = mcp2515_read_rx_status(); // load can message metadata

	//TODO - write code that deals with the incoming INT1

	if (dynamo_available) // if we have a dynamo
	{
		; // TODO - implement dynamo available
	}

	if (dynamo_freq < LOW_SPEED_THRESHOLD_FREQ) //TODO - calculate min. speed for light to get brighter
	{
		OCR1A /= 4;	// reduce intensity
	}
}


// ISR for PCINT2 - pushbutton
ISR(PCINT2_vect)
{
	//TODO - write code which handles the button

		static char toggle = 0;

		// toggle the LED on each interrupt
		if (toggle)
		{
			toggle = 0;
			gpio_conf(RED_LED_pin, OUTPUT, LOW);

		}
		else
		{
			toggle = 1;
			gpio_conf(RED_LED_pin, OUTPUT, HIGH);
		}
}

// interrupt service routine (ISR) for timer 0 A compare match
ISR(TIMER0_COMPA_vect)
{
	// dummy code to be executed every 65ms

	static char toggle = 0;

	// toggle the LED on each interrupt
	if (toggle)
	{
		toggle = 0;
		gpio_conf(GREEN_LED_pin, OUTPUT, LOW);
	}
	else
	{
		toggle = 1;
		gpio_conf(GREEN_LED_pin, OUTPUT, HIGH);
	}
}