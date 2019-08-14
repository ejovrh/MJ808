#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "mj818.h"
#include "gpio.h"

void _fade(const uint8_t value, volatile uint8_t *ocr);

// TODO - optimize
static void _wrapper_fade_mj818(uint8_t value)
{
// TODO - optimize
	_fade(value, &OCR_REAR_LIGHT);
};

// implementation of virtual constructor for LEDs
void virtual_led_ctorMJ818(volatile leds_t *self)
{
	static individual_led_t individual_led[2] __attribute__ ((section (".data")));		// define array of actual LEDs and put into .data
	self->led = individual_led;											// assign pointer to LED array

	self->led[Rear].Shine = &_wrapper_fade_mj818;						// LED-specific implementation
};

// defines device operation on empty bus
void EmptyBusOperationMJ818(void)
{
	if (OCR_REAR_LIGHT == 0x00)											// run once
	{
		_fade(0x10, &OCR_REAR_LIGHT);									// turn on rear light
		_fade(0x05, &OCR_BRAKE_LIGHT);									// turn on rear light
	}
};

// dispatches CAN messages to appropriate sub-component on device
void PopulatedBusOperationMJ818(volatile void *in_msg, volatile void *self)
{
	message_handler_t *msg_ptr = (message_handler_t *) in_msg;			// pointer cast to avoid compiler warnings
	mj818_t *dev_ptr = (mj818_t *) self;								//	ditto

	volatile can_msg_t *msg = msg_ptr->ReceiveMessage(msg_ptr);			// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | REAR_LIGHT) )		// rear positional light
	{
		dev_ptr->led->led[Rear].Shine(msg->ARGUMENT);					// fade rear light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT) )		// brake light
	{
		if (msg->ARGUMENT > OCR_MAX_BRAKE_LIGHT)
			OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
		else
			OCR_BRAKE_LIGHT = msg->ARGUMENT;
	}
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
			  _BV(WGM10));												// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR1B = _BV(CS10);													// clock pre-scaler: clk/1 (no pre-scaling)

	// timer/counter0 - 8bit - rear light PWM
	TCCR0A = ( _BV(COM0A1)|												// Clear OC1A/OC1B on Compare Match when up counting
			   _BV(WGM00) );											// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock pre-scaler: clk/8

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

#if defined(MJ818_)
volatile mj818_t Device __attribute__ ((section (".data")));			// define Device object and put it into .data
#endif