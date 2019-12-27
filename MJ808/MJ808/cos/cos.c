#include "cos\cos.h"
#include "cos\tps630701\tps630701.h"
#include "cos\mcp73871\mcp73871.h"

#include <util/delay.h>
#include <avr/interrupt.h>

typedef struct															// cos_t actual
{
	cos_t public;														// public struct

	volatile uint16_t __ICR1;											// Input Capture Register, gets filled by ISR of ICR
} __cos_t;

static __cos_t __Device __attribute__ ((section (".data")));			// preallocate __Device object in .data

void _HeartbeatPeriodicCos(void)										// ran by every watchdog ISR, period should be 250ms
{
	__Device.public.ACfreq = (125000.0 / __Device.__ICR1);				// convert cycle count to frequency

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

	if (__Device.public.ACfreq < 5.0)									// slow speed - Delon voltage doubler rectifier
		__Device.public.Reg->SetRegulatorMode(__Device.public.Reg->RegulatorMode & Delon);

	if (__Device.public.ACfreq >= 5.0)									// higher speed - Graetz rectifier
		__Device.public.Reg->SetRegulatorMode(__Device.public.Reg->RegulatorMode & Graetz);

	if (__Device.public.ACfreq >= 18.0)									// even higher speed - Graetz & tuning caps, increase load
	{
		__Device.public.Reg->SetRegulatorMode(__Device.public.Reg->RegulatorMode & ( Graetz | Tuning) );
		__Device.public.LiIonCharger->SetResistor(128);
		*(__Device.public.BuckBoost->PWM) = 0xE0;
	}

	if (__Device.public.ACfreq >= 45.0)									// even more higher speed - Graetz and even higher load
	{
		__Device.public.Reg->SetRegulatorMode(__Device.public.Reg->RegulatorMode & Graetz);
		__Device.public.LiIonCharger->SetResistor(255);
		*(__Device.public.BuckBoost->PWM) = 0xE0;
	}

	// TODO - define message format
	// TODO - try to average over 250ms
	__Device.public.BuckBoost->GetValues(__Device.public.OpParamArray);	// download voltage/current measurement into array
	__Device.public.OpParamArray[MISC_STATUS_BITS] |= ( 0x1C & __Device.public.LiIonCharger->GetStatus());	// save charger/Powerpath controller operating mode its into MISC_STATUS_BITS
	__Device.public.OpParamArray[MISC_STATUS_BITS] |= ( 0xE0 & __Device.public.Reg->RegulatorMode);			// save rectifier operating mode bits into MISC_STATUS_BITS

	// TODO - implement array as argument to send message and not copy by value as SendMessage currently is (see _SendMessage() in message.c)
	MsgHandler->SendMessage(0x01, *__Device.public.OpParamArray, 7);	// send out operational parameters to the bus
	MsgHandler->SendMessage(0x01, __Device.public.ACfreq, 7);			// send out operational parameters to the bus
};

void _PopulatedBusOperationCOS(message_handler_t * const in_msg)		// received MsgHandler object and passes
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object

	//TODO - if clause is a dummy - implement proper command byte structure
	if (msg->COMMAND == (CMND_COS_SET_MODE | 0x0F))						// handle request for operating mode change
		__Device.public.Reg->SetRegulatorMode(0x07 & msg->COMMAND);		// bit val

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
	gpio_conf(COMPARATOR_REF_pin, INPUT, NOPULLUP);						// comparator pin as input and no pullups
	gpio_conf(COMPARATOR_IN_pin, INPUT, NOPULLUP);						// comparator pin as input and no pullups
	gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);						// MCP2561 standby - low (on), high (off)
	gpio_conf(TPS630701_PWM_pin, OUTPUT, HIGH);							// Buck-Boost converter PWM input - low (off), high (on)
	gpio_conf(MP3221_EN_pin, OUTPUT, LOW);								// 5V0 Boost converter enable pin - low (off), high (on)
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
	TIMSK = ( _BV(ICIE1) );												// Input Capture interrupt enable

	TCCR1B = ( _BV(ICNC1) |												// turn on comparator noise canceler
			   _BV(ICES1) |												// capture on rising edge
			   _BV(CS10) );												// clkIO/1 (from pre-scaler), start timer

	// timer/counter0 - 8bit - buck-boost PWM control
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

	// __Device function pointers
	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationCOS;	// implements device-specific operation depending on bus activity
	__Device.public.mj8x8->HeartbeatPeriodic = &_HeartbeatPeriodicCos;	// override mj8x8_t's own Heartbeat() - the purpose is to make use of the (periodic) watchdog timer
	__Device.public.Cos6V0OutputEnabled = &_Cos6V0OutputEnabled;		// enabled/disables the 5V0 Boost output controller

	// __Device core objects
	__Device.public.Reg = reg_ctor();									// initializes reg_t object
	__Device.public.BuckBoost = tps630701_ctor();						// initializes tps630701_t object
	__Device.public.LiIonCharger = mcp73871_ctor();						// initializes mcp73871_t object

	// Èos __Device operational initialization on MCU power on
	__Device.public.Reg->SetRegulatorMode(Delon);						// set regulator into Delon mode - we are very likely to start spinning slow
	*(__Device.public.BuckBoost->PWM) = 0xFF;							// put Buck-Boost into PWM/PFM (auto) mode
	__Device.public.LiIonCharger->SetResistor(128);						// set resistor value to something
};

#if defined(COS_)														// all devices have the object name "Device", hence the preprocessor macro
cos_t * const Device = &__Device.public;								// set pointer to MsgHandler public part
#endif

ISR(PCINT2_vect)														// pin change ISR (PD4) if port expander sees activity
{
	__Device.public.OpParamArray[MISC_STATUS_BITS] = ( MASK_MISC_STATUS_BITS_MCP73871 & __Device.public.LiIonCharger->GetStatus());
};

ISR(TIMER1_CAPT_vect)													// timer1 input capture interrupt for comparator output - once per AC period
{
	cli();																// disable interrupts

	__Device.__ICR1 = ICR1;												// length of one Dynamo AC period in n cycles, as determined by comparator and measured by input capture
	TCNT1 = 0;															// reset timer1 counter

	sei();																// enable interrupts
};