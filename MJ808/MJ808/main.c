#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "gpio.h"
#include "gpio_definitions.h"

#include "mcp2515.h"

#define FRONT_OCR1A 0x40 // count to hex value
#define LOW_SPEED_THRESHOLD_FREQ 20 // dynamo frequency below which we consider the bike to move too slow for power generation -> dim light

uint8_t can_msg[11]; // holds the received CAN message
volatile uint8_t can_msg_status; // holds message status metadata
volatile uint8_t dynamo_available = 0;	// boolean: 0 - dynamo absent, 1 - dynamo detected
volatile uint8_t dynamo_freq = 0; // dynamo frequency (is NOT wheel frequency)

int main(void)
{
	#include "gpio_modes.h"

	PRR = _BV(PRUSART);	// turn off USART, we don't need it

	mcp2515_init(); // initialize & configure the MCP2515

	//TODO - active CAN bus device discovery


	cli();	// clear interrupts globally

	// setup of INT1
	MCUCR = _BV(ISC10); // any logical change generates an IRQ
	GIMSK = _BV(INT1);	// enable INT1

	//TODO - check front light (OC1A) PWM with scope
	OCR1A = FRONT_OCR1A;	// count to 64
	TCCR1A = _BV(COM1A0); // toggle OC1A on compare match - aka. direct waveform generation on pin
	TCCR1B = (_BV(WGM12)|	// CTC mode
	_BV(CS11) | _BV(CS10)); // clock prescaler: clk/256

	sei();	// enable interrupts globally

	while (1) // forever loop
	{
		// keep as empty as possible !!
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
		//OCR1A /= 4;	// reduce intensity
	}
}