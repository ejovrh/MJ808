#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "mj828.h"
#include "gpio.h"

extern void _fade(const uint8_t value, volatile uint8_t *ocr);
extern void _debounce(volatile individual_button_t *in_button, volatile event_handler_t *in_event);

void __mj828_led_gpio_init(void)
{
	gpio_conf(LED_CP1_pin, INPUT, LOW);									// Charlie-plexed pin1
	gpio_conf(LED_CP2_pin, INPUT, LOW);									// Charlie-plexed pin2
	gpio_conf(LED_CP3_pin, INPUT, LOW);									// Charlie-plexed pin3
	gpio_conf(LED_CP4_pin, INPUT, LOW);									// Charlie-plexed pin4
};

static void __LED_red(const uint8_t state)								// red LED on/off
{
	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);								// pin b1 - anode

	if (state)															// on
		gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// pin b2 - cathode
	else																// off
		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// pin b2 - cathode
};

static void __LED_green(const uint8_t state)							// green LED on/off
{
	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);								// pin b2 - anode

	if (state)															// on
		gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// pin b1 - cathode
	else																// off
		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// pin b1 - cathode
}

static void __LED_blue1(const uint8_t state)							// blue1 LED on/off
{
	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);								// pin b1 - anode

	if (state)															// on
		gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// pin b0 - cathode
	else																// off
		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// pin b0 - cathode
}

static void __LED_yellow(const uint8_t state)							// yellow LED on/off
{
	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);								// pin b2 - anode

	if (state)															// on
		gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// pin b1 - cathode
	else																// off
		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// pin b1 - cathode
}

static void __LED_blue2(const uint8_t state)							// blue2 LED on/off
{
	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);								// pin d6 - anode

	if (state)															// on
		gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// pin b0 - cathode
	else																// off
		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// pin b0 - cathode
}

static void __LED_blue3(const uint8_t state)							// blue3 LED on/off
{
	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);								// pin b0 - anode

	if (state)															// on
		gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// pin d6 - cathode
	else																// off
		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// pin d6 - cathode
}

static void __LED_blue4(const uint8_t state)							// blue4 LED on/off
{
	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);								// pin b2 - anode

	if (state)															// on
		gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// pin d6 - cathode
	else																// off
		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// pin d6 - cathode
}

static void __LED_blue5(const uint8_t state)							// blue5 LED on/off
{
	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);								// pin d6 - anode

	if (state)															// on
		gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// pin b2 - cathode
	else																// off
		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// pin b2 - cathode
}

// private function, used only by the charlieplexing_handler() function
static void __glow(uint8_t led, uint8_t state)
{
	if (!state)															// if we get 0x00 (off argument) - do nothing and get out
		return;

	static uint8_t (*fptr)(const uint8_t in_val);						// declare pointer for function pointers in branchtable_led[]
	static void (* const branchtable_led[])(const uint8_t in_val) PROGMEM =		// array of function pointers for basic LED handling in PROGMEM
	{
		&__LED_red,														// index 0
		&__LED_green,													// index 1
		&__LED_blue1,													//	and so on...
		&__LED_yellow,
		&__LED_blue2,
		&__LED_blue3,
		&__LED_blue4,
		&__LED_blue5
	};

	__mj828_led_gpio_init();											// set LED pins to initial state

	// TODO - implement blinking
	//static uint8_t counter;
	//counter++;

	fptr = pgm_read_ptr(&branchtable_led[led]);							// get appropriate function pointer from PROGMEM
	fptr(state);														// execute with arguments given
};

void charlieplexing_handler(volatile leds_t *in_led)
{
	static uint8_t i = 0;												// iterator to loop over all LEDs on device

	__glow(i, (in_led->flags->All & _BV(i)) );							// pass glow the LED number and the appropriate bit in the flag struct

	// !!!!
	(i == 7) ? i = 0 : ++i;												// count up to led_count and then start from zero
};

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void __mj828_event_execution_function(uint8_t val)
{
	switch (val)														// based on array value at position #foo of array e.g. FooButtonCaseTable[]
	{
		case 0x01:														// error button press
			// TODO - implement device function on button error press
			EventHandler.index &= ~val;
		break;

		case 0x04:
			if (Device.button->button[Left].Momentary)
			{
				Device.led->flags->All |= _BV(Blue);
				MsgHandler.SendMessage(&MsgHandler, (CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) , 0xF8, 2);
			}
			else
			{
				Device.led->flags->All &= ~_BV(Blue);
				MsgHandler.SendMessage(&MsgHandler, (CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) , 0x00, 2);
				EventHandler.index &= ~val;
			}

		break;

		case 0x02:
			if (!Device.button->button[Right].Toggle)
			{
				Device.led->flags->All &= ~_BV(Red);
			}
			else
			{
				Device.led->flags->All |= _BV(Red);
			}
			EventHandler.index &= ~val;
		break;

		default:														// no value passed
			EventHandler.index &= ~val;									// do nothing
		break;
	}
};

// implementation of virtual constructor for buttons
volatile button_t *_virtual_button_ctorMJ828(volatile button_t *self, volatile event_handler_t *event)
{
	static individual_button_t individual_button[2] __attribute__ ((section (".data")));		// define array of actual buttons and put into .data

	self->button = individual_button;									// assign pointer to button array
	self->button_count = 2;												// how many buttons are on this device?
	self->button[0]._PIN = (uint8_t *) 0x30;							// 0x020 offset plus address - PIND register
	self->button[0]._pin_number = 0;									// sw2 is connected to pin D0
	self->button[1]._PIN = (uint8_t *) 0x30;							// ditto
	self->button[1]._pin_number = 1;									// sw2 is connected to pin D1

	static uint8_t LeftButtonCaseTable[] =								// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{
		0x04,	// 0 - jump case 0x04 - momentary event
		0x00,	// 1 - not defined
		0x00,	// 2 - not defined
		0x01	// 3 - not defined
	};

	static uint8_t RightButtonCaseTable[] =								// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{
		0x00,	// 0 - not defined
		0x02,	// 1 - jump case 0x02 - toggle event
		0x00,	// 2 - not defined
		0x01	// 3 - not defined
	};

	self->button[Left].ButtonCaseptr = LeftButtonCaseTable;				// button press-to-case binding
	self->button[Right].ButtonCaseptr = RightButtonCaseTable;			// button press-to-case binding

	self->deBounce = &_debounce;									// tie in debounce function
	event->fpointer = &__mj828_event_execution_function;				// button execution override from default to device-specific

	return self;
};

// implementation of virtual constructor for LEDs
volatile leds_t *_virtual_led_ctorMJ828(volatile leds_t *self)
{
	static individual_led_t individual_led[8] __attribute__ ((section (".data")));		// define array of actual LEDs and put into .data
	self->led = individual_led;											// assign pointer to LED array
	self->flags = &LEDFlags;											// tie in LEDFlags struct into led struct

	// FIXME - if below flag is 0, it doesnt work properly: at least one LED has to be on for the thing to work
	// also: if any other than Green is on, it doesnt shine properly
	self->flags->All = _BV(Green);										// mark green LED as on

	return self;
};

// defines device operation on empty bus
void _EmptyBusOperationMj828(void)
{
	;
};

// dispatches CAN messages to appropriate sub-component on device
void _PopulatedBusOperationMJ828(volatile message_handler_t *in_msg, volatile void *self)
{
	mj828_t *dev_ptr = (mj828_t *) self;								// pointer cast to concrete device

	volatile can_msg_t *msg = in_msg->ReceiveMessage(in_msg);			// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if ( (msg->COMMAND & MASK_COMMAND) == CMND_DASHBOARD )				// dashboard command
	{
		if ((msg->COMMAND & 0x01))										// flag LED at appropriate index as whatever the command says
			dev_ptr->led->flags->All |= _BV( ((msg->COMMAND & 0x0E) >> 1) );			// set bit
		else
			dev_ptr->led->flags->All &= ~_BV( ((msg->COMMAND & 0x0E) >> 1) );			// clear bit

		return;
	}
};

void mj828_ctor(volatile mj828_t * const self, volatile leds_t *led, volatile button_t *button)
{
	// GPIO state definitions
	{
	// state initialization of device-specific pins

	gpio_conf(PUSHBUTTON1_pin, INPUT, LOW);								// SPST-NO - high on press, low on release
	gpio_conf(PUSHBUTTON2_pin, INPUT, LOW);								// SPST-NO - high on press, low on release

	gpio_conf(LED_CP1_pin, INPUT, LOW);									// Charlie-plexed pin1
	gpio_conf(LED_CP2_pin, INPUT, LOW);									// Charlie-plexed pin2
	gpio_conf(LED_CP3_pin, INPUT, LOW);									// Charlie-plexed pin3
	gpio_conf(LED_CP4_pin, INPUT, LOW);									// Charlie-plexed pin4

	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
	// state initialization of device-specific pins
	}

	// hardware initialization
	{
	cli();

	// timer/counter1 - 16bit (and timer/counter0 - 8bit) - pushbutton timing (charlieplexed timing)
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

	// timer/counter0 - 8bit - charlieplexing timer - 25ms
	/* timing of OCR0A in ms
		0xff - 32.5ms
		0x0f - 16.25ms
	*/
	OCR0A = 0x0F;														// 0x0f - 16.25ms, counter increment up to this value
	TCCR0A = _BV(WGM01);												// CTC mode w. TOP = OCR0A, TOV1 set to MAX
	TIMSK |= _BV(OCIE0A);												// additionally enable TCO compare match IRQ enable for OCIE0A
	TCCR0B = ( _BV(CS02) |
			   _BV(CS00) );												// clkIO/1024 (from pre-scaler), start timer

	// TODO - setup of pin change interrupts for pushbuttons
	PCMSK2 = (_BV(PCINT11) |											// enable pin change for sw1 @ pin D0
			  _BV(PCINT12));											// enable pin change for sw2 @ pin D1

	sei();
	}

	self->mj8x8 = mj8x8_ctor(&MJ8x8, &CAN, &MCU);						// call base class constructor & tie in object addresses

	self->led = _virtual_led_ctorMJ828(led);							// call virtual constructor & tie in object addresses
	self->button = _virtual_button_ctorMJ828(button, &EventHandler);	// call virtual constructor & tie in object addresses

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	self->mj8x8->can->own_sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D);	// high byte
	self->mj8x8->can->own_sidl = ( RCPT_DEV_BLANK | BLANK);																// low byte

	self->mj8x8->EmptyBusOperation = &_EmptyBusOperationMj828;			// implements device-specific default operation
	self->mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ828;	// implements device-specific operation depending on bus activity
};

#if defined(MJ828_)														// all devices have the object name "Device", hence the preprocessor macro
volatile mj828_t Device __attribute__ ((section (".data")));			// define Device object and put it into .data
#endif