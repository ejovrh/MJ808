#include "cos\cos.h"
#include "rect\rect.h"													// rectifier object
#include "cos\tps630701\tps630701.h"									// Buck-Boost regulator object
#include "cos\mcp73871\mcp73871.h"										// Li-Ion Charger&Powerpath controller object

#include <util/delay.h>
#include <avr/interrupt.h>

/* functional tests:

verified_TODO: verify PWM output on TPS630701_PWM_pin for range of 0x00 - 0xff
	- PWM signal can be generated

verified_TODO: TIMER1_OVF_vect triggers successfully on timer1 overflow

TODO: verify effect on PWM on TPC63701 for constant load (resistor) and varying load (potentiometer)

verified_TODO: verify _HeartbeatPeriodicCos(void) operation every 250ms
	- executes approximately every 250ms

verified_TODO: verify AD5160 resistor operation (SerRestitor() args: 0, 128, 256, etc., derive formula and place in comments
	- SPI SetResistor() command seems to work; resistance changes are visible in the expected range

TODO: verify LMP92064SD U/I readout
	- check reported voltage and current readings against external measurement
	- voltage looks reasonable but current readout are still erratic

TODO: verify comparator interrupt timing
FIXME: ground noise floor triggers too many interrupts; solve by raising the reference pin via resistor network
done_TODO: check if DIDR enable/disable will help - doesnt help

TODO: verify MP3221 on/off operation via _Cos6V0OutputEnabled()
	works in principle, just needs liion battery attached

TODO: verify MCP23S08 GPIO states, interrupts, etc

TODO: verify mosfet AC switches (Delon, tuning caps, etc)

TODO: verify pin change interrupt on PD4

FIXME: fix bootstrap problem with Graetz mosfet gate pullup

TODO: verify MCP73871 operation for charge and discharge
	- is suspicious: 500mA discharge over a 20Ohm resistor,
	- powerpath is not working (20mA in, 500mA out)
	- Vout without battery seems limited in current (25mA or so)

*/

typedef struct															// cos_t actual
{
	cos_t public;														// public struct

	volatile uint16_t __ICR1;											// Input Capture Register, gets filled by ISR of ICR
} __cos_t;

static __cos_t __Device __attribute__ ((section (".data")));			// preallocate __Device object in .data

void _HeartbeatPeriodicCos(void)										// ran by every watchdog ISR, period should be 250ms
{
	/* on this device the 250ms periodic call is responsible for monitoring the dynamo-generated AC voltage and for adjusting Èos operational parameters.
	*  operational parameters are:
	*	- wheel speed (expressed via __Device.public.OpParamArray[ACFREQ])
	*		this parameter is the basis for everything: Vout with/without no load depends on it, Pmax depends on it
	*	- rectifier operational mode: Delon voltage doubler on/off, Graetz rectifier on/ff, Tuning capacitors on/off
	*	- TPS630701 Buck-Boost PS/SYNC signal on TPS630701_PWM_pin
	*	- Vout/Iout, measured on the Buck-Boost output, before the load
	*	- charge current regulation of the MCP73871 LiIon Charger/Powerpath controller
	*	- status pins of the MCP73871 LiIon Charger/Powerpath controller
	*	- MP3221 6V0 output for outside system load
	*
	*	operational parameters are stored in the uint8_t OpParamArray[OP_PARAM_ARRAY_SIZE] array - see cos.h
	*/

	/*
	//__Device.public.ACfreq = (125000 / __Device.__ICR1);				// convert cycle count to frequency

	if (__Device.public.ACfreq < 5)										// slow speed - Delon voltage doubler rectifier
		__Device.public.Rect->SetRectifierMode(__Device.public.Rect->RectifierMode & _BV(_delon));

	if (__Device.public.ACfreq >= 5)									// higher speed - Graetz rectifier
		__Device.public.Rect->SetRectifierMode(__Device.public.Rect->RectifierMode & _BV(_graetz));

	if (__Device.public.ACfreq >= 18)									// even higher speed - Graetz & tuning caps, increase load
	{
		__Device.public.Rect->SetRectifierMode(__Device.public.Rect->RectifierMode & ( _BV(_graetz) | _BV(_4700uF) ) );
		__Device.public.LiIonCharger->SetResistor(128);
		*(__Device.public.BuckBoost->PWM) = 0xE0;
	}

	if (__Device.public.ACfreq >= 45)									// even more higher speed - Graetz and even higher load
	{
		__Device.public.Rect->SetRectifierMode(__Device.public.Rect->RectifierMode & _BV(_graetz));
		__Device.public.LiIonCharger->SetResistor(255);
		*(__Device.public.BuckBoost->PWM) = 0xE0;
	}

	// TODO - define message format
	// TODO - try to average over 250ms
	__Device.public.OpParamArray[MISC_STATUS_BITS] |= ( 0x1C & __Device.public.LiIonCharger->GetStatus());	// save charger/Powerpath controller operating mode its into MISC_STATUS_BITS
	__Device.public.OpParamArray[MISC_STATUS_BITS] |= ( 0xE0 & __Device.public.Rect->RectifierMode);			// save rectifier operating mode bits into MISC_STATUS_BITS

	// TODO - implement array as argument to send message and not copy by value as SendMessage currently is (see _SendMessage() in message.c)
	MsgHandler->SendMessage(0x01, *__Device.public.OpParamArray, 7);	// send out operational parameters to the bus
	MsgHandler->SendMessage(0x01, __Device.public.ACfreq, 7);			// send out operational parameters to the bus
	*/
	
	__Device.public.BuckBoost->GetValues();								// download voltage/current measurement into array
		
	//__Device.public.LiIonCharger->GetMCP23S08();				// temporary for functional verification
	//__Device.public.LiIonCharger->SetMCP23S08(0x05 ,0x00);
	//__Device.public.LiIonCharger->GetMCP23S08();				// temporary for functional verification

};

void _PopulatedBusOperationCOS(message_handler_t * const in_msg)		// received MsgHandler object and passes
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object

	//TODO - if clause is a dummy - implement proper command byte structure
	if (msg->COMMAND == (CMND_COS_SET_MODE | 0x0F))						// handle request for operating mode change
		__Device.public.Rect->SetRectifierMode(0x07 & msg->COMMAND);	// bit val

	//TODO - if clause is a dummy - implement proper command byte structure
	if (msg->COMMAND == (CMND_COS_SET_MODE | 0x07))						// handle request for operating mode change
		*(__Device.public.BuckBoost->PWM) = (msg->COMMAND | 0xf00);		// full byte val

	//TODO - if clause is a dummy - implement proper command byte structure
	if (msg->COMMAND == (CMND_COS_SET_MODE | 0x07))						// handle request for operating mode change
		__Device.public.LiIonCharger->SetResistor(msg->COMMAND | 0xf00);// full byte val
};

void _Cos6V0OutputEnabled(const uint8_t in_val)							// enable/disable the Èos 5V0 output boost converter, 0 - off, 1 - on
{
	if (in_val)															// if non-zero
		gpio_set(MP3221_EN_pin);										// turn on
	else
		gpio_clr(MP3221_EN_pin);										// turn off
};

void cos_ctor()															// constructor for concrete class
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor((PRIORITY_LOW | UNICAST | SENDER_DEV_CLASS_PWR_SRC | RCPT_DEV_CLASS_BLANK | SENDER_DEV_A));	// call base class constructor & initialize own SID

	// GPIO state definitions
	{
	// state initialization of device-specific pins
	gpio_conf(COMPARATOR_REF_pin, INPUT, HIGH);							// FIXME: should be NOPULLUP; resistor divider on ref. should raise the reference voltage above noise level -- comparator pin as input and no pullups
	gpio_conf(COMPARATOR_IN_pin, INPUT, NOPULLUP);						// comparator pin as input and no pullups
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// MCP2561 standby - low (on), high (off)
	gpio_conf(TPS630701_PWM_pin, OUTPUT, HIGH);							// Buck-Boost converter PWM input - low (off), high (on)
	gpio_conf(MP3221_EN_pin, OUTPUT, LOW);								// 5V0 Boost converter enable pin - low (off), high (on)
	gpio_conf(SPI_SS_AD5160_pin, OUTPUT, HIGH);							// SPI Slave Select known init state
	gpio_conf(SPI_SS_LMP92064SD_pin, OUTPUT, HIGH);						// SPI Slave Select known init state
	gpio_conf(SPI_SS_MCP2515_pin, OUTPUT, HIGH);						// SPI Slave Select known init state
	gpio_conf(SPI_SS_MCP23S08_pin, OUTPUT, HIGH);						// SPI Slave Select known init state

	//gpio_conf(INT_MCP23S08_pin, INPUT, HIGH);							// MCP23S08 interrupt pin; port expander has interrupt pin active low (IOCON register, datasheet p. 15)
	gpio_conf(INT_MCP23S08_pin, OUTPUT, LOW);							// temporary pin for verification of stuff

	// state initialization of device-specific pins
	}

	// hardware initialization
	{
	cli();

	DIDR = (_BV(AIN1D) | _BV(AIN0D) );									// disable digital input buffer on comparator pins
	ACSR &= ~_BV(ACD);													// clear bit - enable comparator
	ACSR |= ( _BV(ACIC) |												// enable input capture function in timer1, needs ICIE1 in TIMSK
			(_BV(ACIS1) | _BV(ACIS0)) );								// rising edge

	// timer/counter1 - 16bit - frequency measurement by means of zero cross detection of dynamo AC voltage
	/*

	*/
	TIMSK = ( _BV(ICIE1) | _BV(TOV1) );									// Input Capture interrupt enable

	TCCR1B = ( _BV(ICNC1) |												// turn on comparator noise canceler
			   _BV(ICES1) |												// capture on rising edge
			   _BV(CS11) |
			   _BV(CS10) );												// clkIO/64 (from pre-scaler), start timer

	// timer/counter0 - 8bit - buck-boost PWM control
	TCCR0A = ( _BV(COM0A1) |											// Clear OC1A/OC1B on Compare Match when up counting
			   _BV(WGM00) );											// phase correct 8bit PWM, TOP=0x00FF, update of OCR at TOP, TOV flag set on BOTTOM
	TCCR0B = _BV(CS01);													// clock pre-scaler: clk/8

	TIFR = 0x00;														// unset all pending interrupts

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

	// __Device function pointers
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationCOS;	// implements device-specific operation depending on bus activity
	__Device.public.mj8x8->HeartbeatPeriodic = &_HeartbeatPeriodicCos;	// override mj8x8_t's own Heartbeat() - the purpose is to make use of the (periodic) watchdog timer
	__Device.public.Cos6V0OutputEnabled = &_Cos6V0OutputEnabled;		// enabled/disables the 5V0 Boost output controller

	// __Device core objects
	__Device.public.Rect = RECT;										// get address of rect_t object
	__Device.public.BuckBoost = TPS630701;								// get address of tps630701_t object
	__Device.public.LiIonCharger = MCP73871;							// get address of mcp73871_t object

	// Èos __Device operational initialization on MCU power on
	// NOTE: most default states are implemented in hardware by means of pulldown/pullup resistors
	//__Device.public.Rect->SetRectifierMode(_delon);						// set regulator manually into Delon mode - we are very likely to start spinning slow
	*(__Device.public.BuckBoost->PWM) = 0xFF;							// put Buck-Boost into PWM/PFM (auto) mode
	__Device.public.LiIonCharger->SetResistor(128);						// set resistor value to something


	__Device.public.Cos6V0OutputEnabled(0);

};

#if defined(COS_)														// all devices have the object name "Device", hence the preprocessor macro
cos_t * const Device = &__Device.public;								// set pointer to MsgHandler public part
#endif

ISR(PCINT2_vect)														// pin change ISR (PD4) if port expander sees activity
{
	//__Device.public.OpParamArray[MISC_STATUS_BITS] = ( MASK_MISC_STATUS_BITS_MCP73871 & __Device.public.LiIonCharger->GetMCP23S08());
};

ISR(TIMER1_CAPT_vect)													// timer1 input capture interrupt for comparator output - once per AC period
{
	cli();																// disable interrupts

	gpio_toggle(INT_MCP23S08_pin);

	TCNT1 = 0;															// reset timer1 counter

	__Device.__ICR1 = ICR1;												// length of one Dynamo AC period in n cycles, as determined by comparator and measured by input capture

	sei();																// enable interrupts
};

ISR(TIMER1_OVF_vect)													// timer1 overflow
{
	cli();																// disable interrupts

	sei();																// enable interrupts
};