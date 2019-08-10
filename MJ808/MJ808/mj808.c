#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj808.h"
#include "led.h"
#include "gpio.h"

// TODO - optimize
void _fade(const uint8_t value, volatile uint8_t *ocr);

// TODO - optimize
void _wrapper_fade_mj808(const uint8_t value)
{
	_fade(value, &OCR_FRONT_LIGHT);
};

// concrete utility LED handling function
void _util_led_mj808(uint8_t in_val)
{
	uint8_t led = 0;													// holds the pin of the LED: D0 - green (default), D1 - red

	if (in_val & _BV(B3))												// determine B3 value: red or green (default)
	led = 1;															// red

	in_val &= 7;														// clear everything except B2:0, which is the blink count (1-6)

	if (in_val == 0x00)													// B3:B0 is 0 - turn off
	{
		PORTD |= (1<<led);												// clear bit
		return;
	}

	if (in_val == 0x07)													// B3:B0 is 7 - turn on
	{
		PORTD &= ~(1<<led);												// set bit
		return;
	}

	while (in_val--)													// blink loop
	{
		// TODO - util_led() - get rid of _delay_ms()
		_delay_ms(BLINK_DELAY);											// waste a few cycles (non-blocking)
		PORTD ^= (1<<led);												// toggle the led pin
		_delay_ms(BLINK_DELAY);											// waste a few cycles (non-blocking)
		PORTD ^= (1<<led);												// toggle the led pin
	}
};

// implementation of virtual constructor for buttons
void virtual_button_ctorMJ808(volatile button_t *self)
{
	self[0].PIN = (uint8_t *) 0x30; 									// 0x020 offset plus address - PIND register
	self[0].pin_number = 4;												// sw2 is connected to pin D0
};

// implementation of virtual constructor for LEDs
void virtual_led_ctorMJ808(volatile leds_t *self)
{
	static individual_led_t individual_led[2] __attribute__ ((section (".data")));		// define array of actual LEDs and put into .data
	self->led = individual_led;											// assign pointer to LED array

	self->led[Utility].Shine = &_util_led_mj808;
	self->led[Front].Shine = &_wrapper_fade_mj808;
};

// device default operation on empty bus
void EmptyBusOperationMJ808(void)
{
	;
};

// received MsgHandler object and passes
void PopulatedBusOperationMJ808(volatile void *in_msg, volatile void *self)
{
	message_handler_t *msg_ptr = (message_handler_t *) in_msg;			// pointer cast to avoid compiler warnings
	mj808_t *dev_ptr = (mj808_t *) self;								//	ditto

	volatile can_msg_t *msg = msg_ptr->ReceiveMessage(msg_ptr);			// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if ( (msg->COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)				// utility LED command
	{
		dev_ptr->led->led[Utility].Shine(msg->COMMAND);					// glowy thingy
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT) )		// front positional light - low beam
	{
		// TODO - access via object
		_wrapper_fade_mj808(msg->ARGUMENT);										// fade front light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) ) // front positional light - high beam
	{
		// TODO - implement timer based safeguard when OCR > OCR_MAX
		if (msg->ARGUMENT > OCR_MAX_FRONT_LIGHT)						// safeguard against too high a value (heating of MOSFet)
			OCR_FRONT_LIGHT = OCR_MAX_FRONT_LIGHT;
		else
			OCR_FRONT_LIGHT = msg->ARGUMENT;

		return;
	}
};

volatile mj808_t * mj808_ctor(volatile mj808_t *self, volatile mj8x8_t *base, volatile leds_t *led, volatile button_t *button, volatile message_handler_t *msg)
{
	// GPIO state definitions
	{
	// state initialization of device-specific pins
	gpio_conf(PWM_front_light_pin, OUTPUT, LOW);						// low (off), high (on)
	gpio_conf(RED_LED_pin, OUTPUT, HIGH);								// low (on), high (off)
	gpio_conf(GREEN_LED_pin, OUTPUT, HIGH);								// low (on), high (off)
	gpio_conf(PUSHBUTTON_pin, INPUT, LOW);								// SPST-NO - high on press, low on release
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
	// state initialization of device-specific pins
	}

	// hardware initialization
	{
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
	}

	self->mj8x8 = base;													// remember own object address
	self->led = led;													// remember the LED object address
	self->led->virtual_led_ctor = &virtual_led_ctorMJ808;
	self->button->virtual_button_ctor = &virtual_button_ctorMJ808;
	//self->button = &button;

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A);		// high byte
	msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);																		// low byte

	msg->bus->NumericalCAN_ID = (uint8_t) ( (msg->out->sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	self->mj8x8->EmptyBusOperation = &EmptyBusOperationMJ808;			// implement device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ808;	// implements device-specific operation depending on bus activity

	self->led->virtual_led_ctor(self->led);								// call virtual constructor
	self->button->virtual_button_ctor(self->button);					// call virtual constructor

	// TODO - access via object
	_util_led_mj808(UTIL_LED_GREEN_BLINK_1X);									// crude "I'm finished" indicator

	return self;
};

#if defined(MJ808_)
volatile mj808_t Device __attribute__ ((section (".data")));			// define Device object and put it into .data
#endif