#include <avr/sfr_defs.h>
#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "mj808.h"
#include "gpio.h"

// interprets CMND_UTIL_LED command - utility LED (red, green, on, off, blink)
void util_led(uint8_t in_val)
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
		_delay_ms(BLINK_DELAY);											// waste a few cycles (non-blocking)
		PORTD ^= (1<<led);												// toggle the led pin
		_delay_ms(BLINK_DELAY);											// waste a few cycles (non-blocking)
		PORTD ^= (1<<led);												// toggle the led pin
	}
};

// private function - fades *ocr to value (or ocr_max) - up to OCR_MAX or down to 0x00
void fade(uint8_t value, volatile uint8_t *ocr, uint8_t ocr_max)
{
	cli();																// if without cli(), *ocr gets corrupted; im suspecting an ISR while ocr is incrementin/decrementing
	//	hence an atomic fade()

	if (value > *ocr)													// we need to get brighter
	{
		while (++*ocr < value)											// loop until we match the OCR with the requested value & increment the OCR
		{
			//if (*ocr >= ocr_max)										// safeguard against too high a CAN command argument (OCR_MAX is a function of schematic & PCB design)
			//{
			//#if defined(MJ808_)
			//util_led(UTIL_LED_RED_BLINK_2X);						// CHECKME: something drives this OCR dangerously up
			//#endif
			//break;
			//}

			_delay_ms(5);												// delay it a bit for visual stimulus ...
		}

		sei();															// enable interrupts
		return;
	}

	if (value < *ocr)													// we need to get dimmer & decrement the OCR
	{
		while (--*ocr > value)											// loop until we match the OCR with the requested value
		{
			#if defined(MJ808_)
			_delay_ms(2.5);												// delay it a bit for visual stimulus ...
			#endif
			#if defined(MJ818_)
			_delay_ms(1);												// delay it a bit for visual stimulus ...
			#endif
		}

		sei();															// enable interrupts
		return;
	}
};

// interprets CMND_DEVICE-DEV_LIGHT command - positional light control
void dev_light(volatile can_msg_t *msg)
{
	#if defined (MJ808_)
	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT) )		// front positional light - low beam
	{
		fade(msg->ARGUMENT, &OCR_FRONT_LIGHT, OCR_MAX_FRONT_LIGHT);		// fade front light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) ) // front positional light - high beam
	{
		if (msg->ARGUMENT > OCR_MAX_FRONT_LIGHT)
		{
			OCR_FRONT_LIGHT = OCR_MAX_FRONT_LIGHT;
			util_led(UTIL_LED_RED_BLINK_2X);							// CHECKME: something drives this OCR dangerously up
		}
		else
		OCR_FRONT_LIGHT = msg->ARGUMENT;

		return;
	}
	#endif

	#if defined (MJ818_)
	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | REAR_LIGHT) )		// rear positional light
	{
		fade(msg->ARGUMENT, &OCR_REAR_LIGHT, OCR_MAX_REAR_LIGHT);		// fade rear light to CAN msg. argument value
		return;
	}

	if (msg->COMMAND == ( CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT) )		// brake light
	{
		if (msg->ARGUMENT > OCR_MAX_BRAKE_LIGHT)
		OCR_BRAKE_LIGHT = OCR_MAX_BRAKE_LIGHT;
		else
		OCR_BRAKE_LIGHT = msg->ARGUMENT;
	}
	#endif
};

void EmptyBusOperationMJ808(void)										// device default operation on empty bus
{
	;
};

void PopulatedBusOperationMJ808(volatile can_msg_t *in_msg)						// device operation on populated (not empty) bus
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

	if ( (in_msg->COMMAND & CMND_UTIL_LED) == CMND_UTIL_LED)			// utility LED command
	{
		util_led(in_msg->COMMAND);										// blinky thingy
		return;
	}
};

volatile mj808_t * mj808_ctor(volatile mj808_t *self, volatile mj8x8_t *base, volatile message_handler_t *msg)
{
// state initialization of device-specific pins
	gpio_conf(PWM_front_light_pin, OUTPUT, LOW);						// low (off), high (on)
	gpio_conf(RED_LED_pin, OUTPUT, HIGH);								// low (on), high (off)
	gpio_conf(GREEN_LED_pin, OUTPUT, HIGH);								// low (on), high (off)
	gpio_conf(PUSHBUTTON_pin, INPUT, LOW);								// SPST-NO - high on press, low on release
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
// state initialization of device-specific pins

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

	self->mj8x8 = base;													// remember own object address

	/*
	 * self, template of an outgoing CAN message; SID intialized to this device
	 * NOTE:
	 *	the MCP2515 uses 2 left-aligned registers to hold filters and SIDs
	 *	for clarity see the datasheet and a description of any RX0 or TX or filter register
	 */
	msg->out->sidh = (PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A);		// high byte
	msg->out->sidl = ( RCPT_DEV_BLANK | BLANK);																		// low byte

	self->mj8x8->bus->NumericalCAN_ID = (uint8_t) ( (msg->out->sidh >>2 ) & 0x0F ) ; // populate the status structure with own ID

	// EmptyBusOperation() is so far not needed
	//self->mj8x8->EmptyBusOperation = &EmptyBusOperationMJ808;			// implement device-specific default operation
	self->mj8x8->PopulatedBusOperation = &PopulatedBusOperationMJ808;	// implements device-specific operation depending on bus activity

	util_led(UTIL_LED_GREEN_BLINK_1X);									// crude "im finished" indicator

	return self;
};