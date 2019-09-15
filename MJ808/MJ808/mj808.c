#include "mj808.h"
#include "mj808_led.c"													// concrete device-specific LED functions
#include "mj808_button.c"												// concrete device-specific button functions

typedef struct															// mj808_t actual
{
	mj808_t public;														// public struct
} __mj808_t;

static __mj808_t __Device __attribute__ ((section (".data")));			// preallocate __Device object in .data

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void _event_execution_function_mj808(const uint8_t val)
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

		// next cases: 0x08, 0x16, etc.

		default:	// 0x00
			EventHandler->UnSetEvent(val);
			return;
	}
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
		// CHECKME - does it work?
		__Device.public.led->led[Front].Shine(msg->ARGUMENT);
		// TODO - access via object
//		_wrapper_fade_mj808(msg->ARGUMENT);								// fade front light to CAN msg. argument value
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
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A));	// call base class constructor & initialize own SID

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

	__Device.public.led = _virtual_led_ctorMJ808();						// call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ808();				// call virtual constructor & tie in object addresses

	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ808;// implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function_mj808;			// implements event hander for this device

	// TODO - access via object
	_util_led_mj808(UTIL_LED_GREEN_BLINK_1X);							// crude "I'm finished" indicator
};

#if defined(MJ808_)														// all devices have the object name "Device", hence the preprocessor macro
mj808_t * const Device = &__Device.public ;								// set pointer to MsgHandler public part
#endif