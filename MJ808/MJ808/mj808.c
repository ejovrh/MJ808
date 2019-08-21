#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <util/delay.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj808.h"
#include "led.h"
#include "gpio.h"

// TODO - optimize
extern void _fade(const uint8_t value, volatile uint8_t *ocr);
extern void _debounce(volatile individual_button_t *in_button, volatile event_handler_t *in_event);

// TODO - optimize
static void _wrapper_fade_mj808(const uint8_t value)
{
	_fade(value, &OCR_FRONT_LIGHT);
};

// TODO - optimize & should be static and the caller in question should use an object
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

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void __mj808_event_execution_function(const uint8_t val)
{
	switch (val)
	{
		case 0x01:	// button error: - do the error thing
			// TODO - implement device function on button error press
			EventHandler.index &= ~val;
			return;

		case 0x02:	// button hold
			if (Device.button->button[Center].Hold)
			{
				Device.led->led[Utility].Shine(UTIL_LED_GREEN_ON);		// green LED on
				Device.led->led[Front].Shine(0x20);						// front light on - low key; gets overwritten by LU command, since it comes in a bit later

				 //send the messages out, UDP-style. no need to check if the device is actually online
				MsgHandler.SendMessage(&MsgHandler, MSG_BUTTON_EVENT_BUTTON0_ON, 0x00, 1);					// convey button press via CAN and the logic unit will do its own thing
				MsgHandler.SendMessage(&MsgHandler, (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0xFF, 2);		// turn on rear light
				MsgHandler.SendMessage(&MsgHandler, DASHBOARD_LED_YELLOW_ON, 0x00, 1);						// turn on yellow LED
			}
			else
			{
				Device.led->led[Utility].Shine(UTIL_LED_GREEN_OFF);		// green LED off
				Device.led->led[Front].Shine(0x00);						// front light off

				// send the messages out, UDP-style. no need to check if the device is actually online
				MsgHandler.SendMessage(&MsgHandler, MSG_BUTTON_EVENT_BUTTON0_OFF, 0x00, 1);					// convey button press via CAN and the logic unit will tell me what to do
				MsgHandler.SendMessage(&MsgHandler, (CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0x00, 2);		// turn off rear light
				MsgHandler.SendMessage(&MsgHandler, DASHBOARD_LED_YELLOW_OFF, 0x00, 1);						// turn off yellow LED
			}
			EventHandler.index &= ~val;
		break;

		default:	// 0x00
			EventHandler.index &= ~val;
			return;
	}
};

volatile button_t *_virtual_button_ctorMJ808(volatile button_t * const self, volatile event_handler_t * const event)
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

	self->deBounce = &_debounce;									// tie in debounce function
	event->fpointer = &__mj808_event_execution_function;				// button execution override from default to device-specific

	return self;
};

// implementation of virtual constructor for LEDs
volatile leds_t *_virtual_led_ctorMJ808(volatile leds_t *self)
{
	static individual_led_t individual_led[2] __attribute__ ((section (".data")));		// define array of actual LEDs and put into .data
	self->led = individual_led;											// assign pointer to LED array
	self->flags = &LEDFlags;											// tie in LEDFlags struct into led struct

	self->led[Utility].Shine = &_util_led_mj808;						// LED-specific implementation
	self->led[Front].Shine = &_wrapper_fade_mj808;						// LED-specific implementation

	return self;
};

// device default operation on empty bus
void _EmptyBusOperationMJ808(void)
{
	// empty - our button will tell us when to act
	return;
};

// received MsgHandler object and passes
void _PopulatedBusOperationMJ808(volatile message_handler_t *in_msg, volatile void *self)
{
	mj808_t *dev_ptr = (mj808_t *) self;								// pointer cast to concrete device

	volatile can_msg_t *msg = in_msg->ReceiveMessage(in_msg);			// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if ( (msg->COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)				// utility LED command
	{
		dev_ptr->led->led[Utility].Shine(msg->COMMAND);					// glowy thingy
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

void mj808_ctor(volatile mj808_t * const self, volatile leds_t *led, volatile button_t *button)
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
	PCMSK2 = _BV(PCINT15);												// enable pin change for switch @ pin D4

	sei();
	}

	self->mj8x8 = mj8x8_ctor(&MJ8x8, &CAN, &MCU);						// call base class constructor & tie in object addresses

	self->led = _virtual_led_ctorMJ808(led);								// call virtual constructor & tie in object addresses

	self->button = _virtual_button_ctorMJ808(button, &EventHandler);					// call virtual constructor & tie in object addresses

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	self->mj8x8->can->own_sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A);	// high byte
	self->mj8x8->can->own_sidl = ( RCPT_DEV_BLANK | BLANK);																	// low byte

	self->mj8x8->EmptyBusOperation = &_EmptyBusOperationMJ808;			// implement device-specific default operation
	self->mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ808;	// implements device-specific operation depending on bus activity

	// TODO - access via object
	_util_led_mj808(UTIL_LED_GREEN_BLINK_1X);							// crude "I'm finished" indicator
};

#if defined(MJ808_)														// all devices have the object name "Device", hence the preprocessor macro
volatile mj808_t Device __attribute__ ((section (".data")));			// define Device object and put it into .data
#endif