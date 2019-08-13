#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj828.h"
#include "gpio.h"

static void _glow(uint8_t led, uint8_t state);

void mj828_led_gpio_init(void)
{
	gpio_conf(LED_CP1_pin, INPUT, LOW);									// Charlie-plexed pin1
	gpio_conf(LED_CP2_pin, INPUT, LOW);									// Charlie-plexed pin2
	gpio_conf(LED_CP3_pin, INPUT, LOW);									// Charlie-plexed pin3
	gpio_conf(LED_CP4_pin, INPUT, LOW);									// Charlie-plexed pin4
};

static void _LED_red(const uint8_t state)								// red LED on/off
{
	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);								// pin b1 - anode

	if (state)															// on
	gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// pin b2 - cathode
	else																// off
	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// pin b2 - cathode
};

static void _LED_green(const uint8_t state)								// green LED on/off
{
	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);								// pin b2 - anode

	if (state)															// on
	gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// pin b1 - cathode
	else																// off
	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// pin b1 - cathode
}

static void _LED_blue1(const uint8_t state)								// blue1 LED on/off
{
	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);								// pin b1 - anode

	if (state)															// on
	gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// pin b0 - cathode
	else																// off
	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// pin b0 - cathode
}

static void _LED_yellow(const uint8_t state)							// yellow LED on/off
{
	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);								// pin b2 - anode

	if (state)															// on
	gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// pin b1 - cathode
	else																// off
	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// pin b1 - cathode
}

static void _LED_blue2(const uint8_t state)								// blue2 LED on/off
{
	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);								// pin d6 - anode

	if (state)															// on
	gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// pin b0 - cathode
	else																// off
	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// pin b0 - cathode
}

static void _LED_blue3(const uint8_t state)								// blue3 LED on/off
{
	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);								// pin b0 - anode

	if (state)															// on
	gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// pin d6 - cathode
	else																// off
	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// pin d6 - cathode
}

static void _LED_blue4(const uint8_t state)								// blue4 LED on/off
{
	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);								// pin b2 - anode

	if (state)															// on
	gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// pin d6 - cathode
	else																// off
	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// pin d6 - cathode
}

static void _LED_blue5(const uint8_t state)								// blue5 LED on/off
{
	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);								// pin d6 - anode

	if (state)															// on
	gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// pin b2 - cathode
	else																// off
	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// pin b2 - cathode
}

// private function, used only by the charlieplexing_handler() function
static void _glow(uint8_t led, uint8_t state)
{
	if (!state)															// if we get 0x00 (off argument) - do nothing and get out
	return;

	static void (* const branchtable_led[])(const uint8_t in_val) =		// array of function pointers for basic LED handling
	{
		&_LED_red,
		&_LED_green,
		&_LED_blue1,
		&_LED_yellow,
		&_LED_blue2,
		&_LED_blue3,
		&_LED_blue4,
		&_LED_blue5
	};

	mj828_led_gpio_init();												// set LED pins to initial state

	// TODO - implement blinking
	//static uint8_t counter;
	//counter++;

	(*branchtable_led[led])(state);										// execute LED code depending on supplied LED number
};

void charlieplexing_handler(volatile leds_t *in_led)
{
	static uint8_t i = 0;												// iterator to loop over all LEDs on device

	_glow(i, in_led->led[i].Flag_On);									// tell LED number "i" what to do

	// !!!!
	(i == 7) ? i = 0 : ++i;												// count up to led_count and then start from zero
};

// implementation of virtual constructor for buttons
void virtual_button_ctorMJ828(volatile button_t *self)
{
	static individual_button_t individual_button[2] __attribute__ ((section (".data")));		// define array of actual buttons and put into .data
	self->button = individual_button;									// assign pointer to button array

	self->button[0].pin_number = 0;										// sw2 is connected to pin D0
	self->button[1].pin_number = 1;										// sw2 is connected to pin D1
	self->button[0].PIN = (uint8_t *) 0x30;								// 0x020 offset plus address - PIND register
	self->button[1].PIN = (uint8_t *) 0x30;								// ditto
};

// implementation of virtual constructor for LEDs
void virtual_led_ctorMJ828(volatile leds_t *self)
{
	static individual_led_t individual_led[8] __attribute__ ((section (".data")));		// define array of actual LEDs and put into .data
	self->led = individual_led;											// assign pointer to LED array

	// FIXME - if below flag is 0, it doesnt work properly
	self->flag_any_glow = 1;
	// FIXME - if below flag is 0, it doesnt work properly: at least one LED has to be on for the thing to work
	// also: if any other than Green is on, it doesnt shine properly
	self->led[Green].Flag_On = 1;
};

// defines device operation on empty bus
void EmptyBusOperationMj828(void)
{
	;
};

// dispatches CAN messages to appropriate sub-component on device
void PopulatedBusOperationMJ828(volatile void *in_msg, volatile void *self)
{
	message_handler_t *msg_ptr = (message_handler_t *) in_msg;			// pointer cast to avoid compiler warnings
	mj828_t *dev_ptr = (mj828_t *) self;								//	ditto

	volatile can_msg_t *msg = msg_ptr->ReceiveMessage(msg_ptr);			// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if ( (msg->COMMAND & MASK_COMMAND) == CMND_DASHBOARD )				// dashboard command
	{
		dev_ptr->led->led[ ((msg->COMMAND & 0x0E) >> 1) ].Flag_On = (msg->COMMAND & 0x01);	// flag LED at appropriate index as whatever the command says
		return;
	}
};

volatile mj828_t * mj828_ctor(volatile mj828_t *self, volatile mj8x8_t *base, volatile leds_t *led, volatile button_t *button, volatile message_handler_t *msg)
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

	self->mj8x8 = base;													// remember own object address
	self->led = led;													// remember the LED object address
	self->button = button;												// remember the button object address

	self->led->virtual_led_ctor = &virtual_led_ctorMJ828;
	self->button->virtual_button_ctor = &virtual_button_ctorMJ828;

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D);		// high byte
	msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);																	// low byte

	msg->bus->NumericalCAN_ID = (uint8_t) ( (msg->out->sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	self->mj8x8->EmptyBusOperation = &EmptyBusOperationMj828;			// implements device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ828;	// implements device-specific operation depending on bus activity

	self->led->virtual_led_ctor(self->led);								// call virtual constructor
	self->button->virtual_button_ctor(self->button);					// call virtual constructor

	return self;
};

#if defined(MJ828_)
volatile mj828_t Device __attribute__ ((section (".data")));			// define Device object and put it into .data
#endif