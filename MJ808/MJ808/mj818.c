#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "mj818.h"
#include "gpio.h"

typedef struct															// mj818_t actual
{
	mj818_t public;														// public struct
	//	uint8_t foo_private;											// private - some data member
} __mj818_t;

static __mj818_t __Device __attribute__ ((section (".data")));			// instantiate mj818_t actual, as if it were initialized

extern void _fade(const uint8_t value, volatile uint8_t *ocr);

// TODO - optimize
static void _wrapper_fade_mj818_rear(uint8_t value)
{
// TODO - optimize
	_fade(value, &OCR_REAR_LIGHT);
};

static void _wrapper_fade_mj818_brake(uint8_t value)
{
	// TODO - optimize
	_fade(value, &OCR_BRAKE_LIGHT);
};

inline void __component_led_mj818_device_on(const uint8_t val)
{
	_fade(val, &OCR_BRAKE_LIGHT);
	_fade(val, &OCR_REAR_LIGHT);
};

inline void __component_led_mj818_device_off(void)
{
	_fade(0x00, &OCR_BRAKE_LIGHT);
	_fade(0x00, &OCR_REAR_LIGHT);
};

void _component_led_mj818(const uint8_t val)
{
	if (val)															// true - on, false - off
		__component_led_mj818_device_on(val);												// delegate indirectly to the leaves
	else
		__component_led_mj818_device_off();
};

composite_led_t *_virtual_led_ctorMJ818(composite_led_t *self)
{
	static primitive_led_t primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

	self->led = primitive_led;											// assign pointer to LED array

	self->Shine = &_component_led_mj818;								// component part ("interface")
	self->led[Rear].Shine = &_wrapper_fade_mj818_rear;					// LED-specific implementation
	self->led[Brake].Shine = &_wrapper_fade_mj818_brake;				// LED-specific implementation

	return self;
};

// defines device operation on empty bus
void _EmptyBusOperationMJ818(void)
{
	if (OCR_REAR_LIGHT == 0x00)											// run once
		__Device.public.led->Shine(0x10);								// operate on component part
};

// dispatches CAN messages to appropriate sub-component on device
void _PopulatedBusOperationMJ818(message_handler_t * const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | REAR_LIGHT) )		// rear positional light
	{
		__Device.public.led->led[Rear].Shine(msg->ARGUMENT);			// fade rear light to CAN msg. argument value
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

void mj818_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_B));	// call base class constructor & initialize own SID

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

	static composite_led_t LED __attribute__ ((section (".data")));		// define LED object and put it into .data

	__Device.public.led = _virtual_led_ctorMJ818(&LED);					// call virtual constructor & tie in object addresses

	__Device.public.mj8x8->EmptyBusOperation = &_EmptyBusOperationMJ818;			// override device-agnostic default operation with specifics
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ818;	// implements device-specific operation depending on bus activity
};

#if defined(MJ818_)														// all devices have the object name "Device", hence the preprocessor macro
mj818_t * const Device = &__Device.public ;								// set pointer to MsgHandler public part
#endif