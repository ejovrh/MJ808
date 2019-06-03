#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj808.h"
#include "gpio.h"

void EmptyBusOperationMJ808(void)										// device default operation on empty bus
{
	;
};

void PopulatedBusOperationMJ808(can_msg_t *in_msg)						// device operation on populated (not empty) bus
{
	;
};

volatile mj808_t * mj808_ctor(volatile mj808_t *self, volatile mj8x8_t *base, volatile message_handler_t *msg)
{
// state initialization of device-specific pins
	gpio_conf(PWM_front_light_pin, OUTPUT, LOW);						// low (off), high (on)
	gpio_conf(RED_LED_pin, OUTPUT, HIGH);								// low (on), high (off)
	gpio_conf(GREEN_LED_pin, OUTPUT, HIGH);								// low (on), high (off)
	gpio_conf(PUSHBUTTON_pin, INPUT, LOW);								// SPST-NO - high on press, low on release
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
// state initialization of device-specific pins

	cli();

	OCR_FRONT_LIGHT = 0x00;												// OCR init for front light - have light off

	// timer/counter1 - 16bit (and timer/counter0 - 8bit) - pushbutton timing
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

	// timer/counter0 - 8bit - front light PWM
	TCCR0A = ( _BV(COM0A1)|												// Clear OC1A/OC1B on Compare Match when up counting
	_BV(WGM00) );														// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock prescaler: clk/8


	if(MCUSR & _BV(WDRF))												// power-up - if we got reset by the watchdog...
	{
		MCUSR &= ~_BV(WDRF);											// clear the reset flag
		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = 0x00;													// disable the thing completely
	}

	// TODO - setup of pin change interrupts for pushbuttons
	PCMSK2 = _BV(PCINT15);												// enable pin change for sw @ pin D4

	sei();


	self->mj8x8 = base;

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A);
	msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);

	self->mj8x8->bus->NumericalCAN_ID = (uint8_t) ( (msg->out->sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	// EmptyBusOperation() is so far not needed
	//self->mj8x8->EmptyBusOperation = &EmptyBusOperationMJ808;			// implement device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ808;	// implements device-specific operation depending on bus activity

	util_led(UTIL_LED_GREEN_BLINK_1X);									// crude "im finished" indicator

	return self;
};