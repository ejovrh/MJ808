#include "cos\cos.h"

#include <util/delay.h>
#include <avr/interrupt.h>

typedef struct															// cos_t actual
{
	cos_t public;														// public struct
} __cos_t;

static __cos_t __Device __attribute__ ((section (".data")));			// preallocate __Device object in .data

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void _event_execution_function_cos(const uint8_t val)
{
	return;
};

// received MsgHandler object and passes
void _PopulatedBusOperationCOS(message_handler_t * const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object

	return;
};

void cos_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_LIGHT | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A));	// call base class constructor & initialize own SID

	// GPIO state definitions
	{
	// state initialization of device-specific pins
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// low (on), high (off)
	gpio_conf(TPS630702_PWM_pin, OUTPUT, HIGH);							// low (off), high (on)
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



	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationCOS;// implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function_cos;			// implements event hander for this device

}

#if defined(COS_)														// all devices have the object name "Device", hence the preprocessor macro
cos_t * const Device = &__Device.public ;								// set pointer to MsgHandler public part
#endif