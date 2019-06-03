#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#include "mj828.h"
#include "gpio.h"

void EmptyBusOperationMj828(void)										// device default operation on empty bus
{
	;
};

void PopulatedBusOperationMJ828(can_msg_t *in_msg)						// device operation on populated (not empty) bus
{
	;
};

volatile mj828_t * mj828_ctor(volatile mj828_t *self, volatile mj8x8_t *base, volatile message_handler_t *msg)
{
// state initialization of device-specific pins

	gpio_conf(PUSHBUTTON1_pin, INPUT, LOW);								// SPST-NO - high on press, low on release
	gpio_conf(PUSHBUTTON2_pin, INPUT, LOW);								// SPST-NO - high on press, low on release

	gpio_conf(LED_CP1_pin, INPUT, LOW);									// Charlie-plexed pin1
	gpio_conf(LED_CP2_pin, INPUT, LOW);									// Charlie-plexed pin2
	gpio_conf(LED_CP3_pin, INPUT, LOW);									// Charlie-plexed pin3
	gpio_conf(LED_CP4_pin, OUTPUT, LOW);								// Charlie-plexed pin4
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
// state initialization of device-specific pins

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
			   _BV(CS11)  );											// clkIO/8 (from prescaler), start timer

	// timer/counter0 - 8bit - charlieplexing timer - 25ms
	/* timing of OCR0A in ms
		0xff - 32.5ms
		0x0f - 16.25ms
	*/
	OCR0A = 0x0F;														// 0x0f - 16.25ms, counter increment up to this value
	TCCR0A = _BV(WGM01);												// CTC mode w. TOP = OCR0A, TOV1 set to MAX
	TIMSK |= _BV(OCIE0A);												// additionally enable TCO compare match IRQ enable for OCIE0A
	TCCR0B = ( _BV(CS02) |
			   _BV(CS00) );												// clkIO/1024 (from prescaler), start timer

	// TODO - setup of pin change interrupts for pushbuttons
	PCMSK2 = (_BV(PCINT11) |											// enable pin change for sw1 @ pin D0
	_BV(PCINT12));														// enable pin change for sw2 @ pin D1

	sei();


	self->mj8x8 = base;

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LU | RCPT_DEV_CLASS_BLANK | SENDER_DEV_D);
	msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);

	self->mj8x8->bus->NumericalCAN_ID = (uint8_t) ( (msg->out->sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	self->mj8x8->EmptyBusOperation = &EmptyBusOperationMj828;			// implements device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ828;	// implements device-specific operation depending on bus activity

	return self;
};

// private function, used only by the charlieplexing_handler() function
static void glow(uint8_t led, uint8_t state, uint8_t blink)
{
	// FIXME - blinking is passed by value, hence never decremented where it should be: in the struct
	//	task: put code in glow() into the charlieplex-handler

	// set LED pins to initial state
	gpio_conf(LED_CP1_pin, INPUT, LOW);									// Charlie-plexed pin1
	gpio_conf(LED_CP2_pin, INPUT, LOW);									// Charlie-plexed pin2
	gpio_conf(LED_CP3_pin, INPUT, LOW);									// Charlie-plexed pin3
	gpio_conf(LED_CP4_pin, INPUT, LOW);									// Charlie-plexed pin4

	static uint8_t counter;
	counter++;

	if (! (state || blink) )											// if we get 0x00 (off argument) - do nothing and get out
	return;



	switch (led)
	{
		case 0x00:														//red led
		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// b1 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// b2 - cathode
		else
		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// b2 - cathode
		break;

		case 0x01:														// green led
		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// b2 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// b1 - cathode
		else
		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// b1 - cathode
		break;

		case 0x02:														// blue1 led
		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// b1 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// b0 - cathode
		else
		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// b0 - cathode
		break;

		case 0x03:														// yellow led
		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// b2 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// b1 - cathode
		else
		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// b1 - cathode
		break;

		case 0x04:														// blue2 LED (battery indicator 1)
		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// d6 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// b0 - cathode
		else
		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// b0 - cathode
		break;

		case 0x05:														// blue3 LED (battery indicator 2)
		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// b0 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// d6 - cathode
		else
		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// d6 - cathode
		break;

		case 0x06:														// blue4 LED (battery indicator 3)
		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// b2 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// d6 - cathode
		else
		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// d6 - cathode
		break;

		case 0x07:														// blue5 LED (battery indicator 4)
		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// d6 - anode

		if ( (state) || (blink && counter <= 128) )						// on
		gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// b2 - cathode
		else
		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// b2 - cathode
		break;
	}

};

void charlieplexing_handler(volatile leds_t *in_led)
{
	static uint8_t i = 0;												// iterator to loop over all LEDs on device

	glow(i, in_led->leds[i].on, in_led->leds[i].blink_count);			// e.g. command = 0x00 (red), arg = 0x01 (on)

	// !!!!
	(i == in_led->led_count) ? i = 0 : ++i;								// count up to led_count and then start from zero
};