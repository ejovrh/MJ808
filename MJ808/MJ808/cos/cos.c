#include "cos\cos.h"
#include "cos\tps630701\tps630701.h"
#include "cos\mcp73871\mcp73871.h"

#include <util/delay.h>
#include <avr/interrupt.h>

typedef struct															// cos_t actual
{
	cos_t public;														// public struct

	volatile uint8_t __timer1_overflow;									// private timer1 overflow counter, gets incremented by TIMER1_OVF_vect ISR

	volatile union
	{
		volatile uint8_t icrl1_low_byte;
		volatile uint8_t icrl1_high_byte;
	} __ICR1;															// Input Capture Register, gets filled by ISR of ICR

} __cos_t;

static __cos_t __Device __attribute__ ((section (".data")));			// preallocate __Device object in .data

// executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
// cases in this switch-case statement must be unique for all events on this device
void _event_execution_function_cos(const uint8_t val)
{
	val;
	return;
};

// received MsgHandler object and passes
void _PopulatedBusOperationCOS(message_handler_t * const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object
	msg->dlc;
	return;
};

void cos_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_PWR_SRC | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A));	// call base class constructor & initialize own SID

	// GPIO state definitions
	{
	// state initialization of device-specific pins
	gpio_conf(COMPARATOR_REF_pin, INPUT, NOPULLUP);						// comparator pin as input and no pullups
	gpio_conf(COMPARATOR_IN_pin, INPUT, NOPULLUP);						// comparator pin as input and no pullups
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// MCP2561 standby - low (on), high (off)
	gpio_conf(TPS630702_PWM_pin, OUTPUT, HIGH);							// Buck-Boost converter PWM input - low (off), high (on)
	gpio_conf(MP3221_EN_pin, OUTPUT, LOW);								// 6V0 Boost converter enable pin - low (off), high (on)
	// state initialization of device-specific pins
	}

	// hardware initialization
	{
	cli();

	ACSR &= ~_BV(ACD);													// enable comparator
	ACSR |= (  _BV(ACIC) | 												// enable input capture function in timer1, needs ICIE1 in TIMSK
			(_BV(ACIS1) | _BV(ACIS0)) );								// rising edge

	// timer/counter1 - 16bit - frequency measurement by means of zero cross detection of dynamo AC voltage
	/*

	*/

	TIFR |= _BV(ICF1);													// clear interrupt flag
	TIMSK = ( _BV(ICIE1) | _BV(TOIE1) );								// Input Capture & Timer1 Overflow interrupt enable
	TCCR1B = ( _BV(WGM13) |												// TOP = ICR1, TOV1 set on BOTTOM
			   _BV(ICNC1) |												// turn on comparator noise canceler
			   _BV(ICES1) |												// capture on rising edge
			   _BV(CS11)  );											// clkIO/8 (from pre-scaler), start timer

	// timer/counter0 - 8bit - front light PWM
	OCR_BUCK_BOOST = 0xFF;												// TODO: full on for test purposes -- 0x6180 - 25ms - counter increment up to this value
	TCCR0A = ( _BV(COM0A1) |											// Clear OC1A/OC1B on Compare Match when up counting
			   _BV(WGM00) );											// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock pre-scaler: clk/8


	if (MCUSR & _BV(WDRF))												// power-up - if we got reset by the watchdog...
	{
		MCUSR &= ~_BV(WDRF);											// clear the reset flag
		WDTCR |= (_BV(WDCE) | _BV(WDE));								// WDT change enable sequence
		WDTCR = 0x00;													// disable the thing completely
	}

	GIMSK |= _BV(PCIE2);												// enable pin change interrupts on PCINT17..11 (INT_MCP23S08_pin - PD4)
	PCMSK2 = _BV(PCINT15);												// enable pin change for switch @ pin D4

	sei();
	}

	__Device.__timer1_overflow = 0;										// sets timer1 overflow counter to 0

	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationCOS;// implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function_cos;			// implements event hander for this device

}

#if defined(COS_)														// all devices have the object name "Device", hence the preprocessor macro
cos_t * const Device = &__Device.public ;								// set pointer to MsgHandler public part
#endif

ISR(PCINT2_vect)														// pin change ISR (PD4) if port expander sees activity
{
	;
}

ISR(TIMER1_CAPT_vect)													// timer1 input capture interrupt for comparator output
{
	cli();																// disable interrupts

	__Device.__ICR1.icrl1_low_byte = ICR1L;								// first read in Input Capture low byte
	__Device.__ICR1.icrl1_high_byte = ICR1H;							// then Input Capture high byte

	//TODO - determine wheel freq. based on ICRs
	__Device.public.WheelFreq = 123;
	sei();																// enable interrupts
}

ISR(TIMER1_OVF_vect)													// timer1 overflow vector
{
	++__Device.__timer1_overflow;										// increment
}