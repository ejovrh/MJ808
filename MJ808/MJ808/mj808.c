#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj808.h"
#include "led.h"
#include "gpio.h"

typedef struct															// mj808_t actual
{
	mj808_t public;														// public struct
//	uint8_t foo_private;												// private - some data member
} __mj808_t;

static __mj808_t __Device __attribute__ ((section (".data")));			// instantiate mj808_t actual, as if it were initialized

// TODO - optimize
extern void _fade(const uint8_t value, volatile uint8_t *ocr);
extern void _debounce(individual_button_t *in_button, event_handler_t * const in_event);
extern void DoNothing(void);

// TODO - optimize
static void _wrapper_fade_mj808(const uint8_t value)
{
	_fade(value, &OCR_FRONT_LIGHT);
};

// TODO - optimize & should be static and the caller in question should use an object
// concrete utility LED handling function
void _util_led_mj808( uint8_t in_val)
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

void __device_on()
{
	Device->led->led[Utility].Shine(UTIL_LED_GREEN_ON);					// green LED on
	Device->led->led[Front].Shine(0x20);								// front light on - low key; gets overwritten by LU command, since it comes in a bit later

	 //send the messages out, UDP-style. no need to check if the device is actually online
	MsgHandler->SendMessage(MSG_BUTTON_EVENT_BUTTON0_ON, 0x00, 1);					// convey button press via CAN and the logic unit will do its own thing
	MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0xFF, 2);		// turn on rear light
	MsgHandler->SendMessage(DASHBOARD_LED_YELLOW_ON, 0x00, 1);						// turn on yellow LED
}

void __device_off()
{
	Device->led->led[Utility].Shine(UTIL_LED_GREEN_OFF);				// green LED off
	Device->led->led[Front].Shine(0x00);								// front light off

	// send the messages out, UDP-style. no need to check if the device is actually online
	MsgHandler->SendMessage(MSG_BUTTON_EVENT_BUTTON0_OFF, 0x00, 1);					// convey button press via CAN and the logic unit will tell me what to do
	MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0x00, 2);		// turn off rear light
	MsgHandler->SendMessage(DASHBOARD_LED_YELLOW_OFF, 0x00, 1);						// turn off yellow LED
}

// delegates operations from LED component downwards to LED leaves
void _component_led(const uint8_t val)
{
	if(val)																// true - on, false - off
		__device_on();													// delegate indirectly to the leaves
	else
		__device_off();
};

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void __mj808_event_execution_function(const uint8_t val)
{
	switch (val)
	{
		case 0x01:	// button error: - do the error thing
			Device->led->Shine(0);
			EventHandler->UnSetEvent(val);
			return;

		case 0x02:	// button hold
			Device->led->Shine(Device->button->button[Center].Hold);
			EventHandler->UnSetEvent(val);
		break;

		default:	// 0x00
			EventHandler->UnSetEvent(val);
			return;
	}
};

button_t *_virtual_button_ctorMJ808(button_t * const self, event_handler_t * const event)
{
	static individual_button_t individual_button[1] __attribute__ ((section (".data")));		// define array of actual buttons and put into .data

	self->button = individual_button;									// assign pointer to button array
	self->button_count = 1;												// how many buttons are on this device?
	self->button[Center]._PIN = (uint8_t *) 0x30; 						// 0x020 offset plus address - PIND register
	self->button[Center]._pin_number = 4;								// sw2 is connected to pin D0

	static uint8_t CenterButtonCaseTable[] =							// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{
		0x00,	// 0 - not defined
		0x00,	// 1 - not defined
		0x02,	// 2 - jump case 0x02 - button Hold
		0x01,	// 3 - jump case 0x01 - error event
	};

	self->button[Center].ButtonCaseptr = CenterButtonCaseTable;			// button press-to-case binding

	self->deBounce = &_debounce;										// tie in debounce function
	event->fpointer = &__mj808_event_execution_function;				// button execution override from default to device-specific

	return self;
};

// implementation of virtual constructor for LEDs
composite_led_t *_virtual_led_ctorMJ808(composite_led_t * const self)
{
	static ledflags_t Flags __attribute__ ((section (".data")));		// define LEDFlags object and put it into .data
	static primitive_led_t primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

	self->led = primitive_led;											// assign pointer to LED array
	self->flags = &Flags;												// tie in LEDFlags struct into led struct

	self->Shine = &_component_led;										// component part ("interface")
	self->led[Utility].Shine = &_util_led_mj808;						// LED-specific implementation
	self->led[Front].Shine = &_wrapper_fade_mj808;						// LED-specific implementation

	return self;
};

// received MsgHandler object and passes
void _PopulatedBusOperationMJ808(message_handler_t * const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if ( (msg->COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)				// utility LED command
	{
		__Device.public.led->led[Utility].Shine(msg->COMMAND);			// glowy thingy
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT) )		// front positional light - low beam
	{
		// TODO - access via object
		_wrapper_fade_mj808(msg->ARGUMENT);								// fade front light to CAN msg. argument value
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

void mj808_ctor()
{
	__Device.public.mj8x8 = mj8x8_ctor();								// call base class constructor & tie in object addresses

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
			   _BV(CS11)  );											// clkIO/8 (from pre-scaler), start timer

	// timer/counter0 - 8bit - front light PWM
	TCCR0A = ( _BV(COM0A1) |											// Clear OC1A/OC1B on Compare Match when up counting
			   _BV(WGM00) );											// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock pre-scaler: clk/8


	if(MCUSR & _BV(WDRF))												// power-up - if we got reset by the watchdog...
	{
		MCUSR &= ~_BV(WDRF);											// clear the reset flag
		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = 0x00;													// disable the thing completely
	}

	// TODO - setup of pin change interrupts for pushbuttons
	//PCMSK2 = _BV(PCINT15);												// enable pin change for switch @ pin D4

	sei();
	}

	static composite_led_t LED __attribute__ ((section (".data")));		// define LED object and put it into .data
	static button_t Button __attribute__ ((section (".data")));			// define BUTTON object and put it into .data

	__Device.public.led = _virtual_led_ctorMJ808(&LED);							// call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ808(&Button, EventHandler);	// call virtual constructor & tie in object addresses

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */

	__Device.public.mj8x8->can->own_sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A);	// high byte
	__Device.public.mj8x8->can->own_sidl = ( RCPT_DEV_BLANK | BLANK);																// low byte

	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ808;// implements device-specific operation depending on bus activity

	// TODO - access via object
	_util_led_mj808(UTIL_LED_GREEN_BLINK_1X);							// crude "I'm finished" indicator
};

#if defined(MJ808_)														// all devices have the object name "Device", hence the preprocessor macro
mj808_t * const Device = &__Device.public ;								// set pointer to MsgHandler public part
#endif