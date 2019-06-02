#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj818.h"
#include "mj8x8.h"
//#include "mcp2515.h" // TODO - should not be here
#include "gpio.h"

void EmptyBusOperationMJ818(void)										// device default operation on empty bus
{
	if (OCR_REAR_LIGHT == 0x00)											// run once
		fade(0x10, &OCR_REAR_LIGHT, OCR_MAX_REAR_LIGHT);				// turn on rear light
};

void PopulatedBusOperationMJ818(can_message_t *in_msg)					// device operation on populated (not empty) bus
{
	;
};

volatile mj818_t *mj818_ctor(volatile mj818_t *self, volatile mj8x8_t *base)
{
// state initialization of device-specific pins
	gpio_conf(PWM_rear_light_pin, OUTPUT, LOW);							// low (off), high (on)
	gpio_conf(PWM_brake_light_pin, OUTPUT, LOW);						// low (off), high on
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
// state initialization of device-specific pins

	cli();

	// OCR init for rear lights - have lights off
	OCR_REAR_LIGHT = 0x00;												// rear light
	OCR_BRAKE_LIGHT = 0x00;												// brake light

	// timer/counter1 - 16bit - brake light PWM
	TCCR1A = (_BV(COM1A1) |												// Clear OC1A/OC1B on Compare Match when up counting
	_BV(WGM10));														// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10);													// clock prescaler: clk/1 (no pre-scaling)

	// timer/counter0 - 8bit - rear light PWM
	TCCR0A = ( _BV(COM0A1)|												// Clear OC1A/OC1B on Compare Match when up counting
	_BV(WGM00) );														// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock prescaler: clk/8

	if(MCUSR & _BV(WDRF))												// power-up - if we got reset by the watchdog...
	{
		MCUSR &= ~_BV(WDRF);											// clear the reset flag
		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = 0x00;													// disable the thing completely
	}

	sei();


	self->mj8x8 = base;

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	can_msg_outgoing.sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B);
	can_msg_outgoing.sidl = ( RCPT_DEV_BLANK | BLANK);

	self->mj8x8->EmptyBusOperation = &EmptyBusOperationMJ818;			// implement device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ818;	// implements device-specific operation depending on bus activity

	self->mj8x8->bus->NumericalCAN_ID = (uint8_t) ( (can_msg_outgoing.sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	return self;
}