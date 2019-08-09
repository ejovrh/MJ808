#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "mj818.h"
#include "gpio.h"

// interprets LED commands for this device
void digestMJ818(volatile can_msg_t *in_msg)
{
	// command for device
	if (in_msg->COMMAND & CMND_DEVICE)									//  we received a command for some device...
	{
		if (in_msg->COMMAND & ( CMND_DEVICE | DEV_LIGHT ) )				// ...a LED device
		{
			dev_light(in_msg);											// deal with it
			return;
		}
	}
};

// implementation of virtual constructor for LEDs
void virtual_led_ctorMJ818(volatile leds_t *self)
{
	;
};

// defines device operation on empty bus
void EmptyBusOperationMJ818(void)
{
	if (OCR_REAR_LIGHT == 0x00)											// run once
		fade(0x10, &OCR_REAR_LIGHT, OCR_MAX_REAR_LIGHT);				// turn on rear light
};

// dispatches CAN messages to appropriate sub-component on device
void PopulatedBusOperationMJ818(volatile can_msg_t *in_msg, volatile void *self)
{
	mj818_t *ptr = (mj818_t *) self;									// pointer cast to avoid compiler warnings
	ptr->led->digest(in_msg);											// let the LED object deal wit it
};

volatile mj818_t * mj818_ctor(volatile mj818_t *self, volatile mj8x8_t *base, volatile leds_t *led, volatile message_handler_t *msg)
{
	// GPIO state definitions
	{
	// state initialization of device-specific pins
	gpio_conf(PWM_rear_light_pin, OUTPUT, LOW);							// low (off), high (on)
	gpio_conf(PWM_brake_light_pin, OUTPUT, LOW);						// low (off), high on
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
	// state initialization of device-specific pins
	}

	// hardware initialization
	{
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
	}

	self->mj8x8 = base;													// remember own object address
	self->led = led;													// remember the LED object address
	self->led->digest = &digestMJ818;
	self->led->virtual_led_ctor = &virtual_led_ctorMJ818;

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B);		// high byte
	msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);																		// low byte

	msg->bus->NumericalCAN_ID = (uint8_t) ( (msg->out->sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	self->mj8x8->EmptyBusOperation = &EmptyBusOperationMJ818;			// implement device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ818;	// implements device-specific operation depending on bus activity

	self->led->virtual_led_ctor(self->led);								// call virtual constructor

	return self;
};