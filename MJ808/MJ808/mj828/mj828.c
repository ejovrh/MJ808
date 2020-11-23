#include <avr/interrupt.h>

#include "mj828\mj828.h"

#include "mj828\mj828_led.c"											// concrete device-specific LED functions
#include "mj828\mj828_button.c"											// concrete device-specific button functions

typedef struct															// mj828_t actual
{
	mj828_t public;														// public struct
} __mj828_t;

static __mj828_t __Device __attribute__ ((section (".data")));			// preallocate __Device object in .data

void _event_execution_function_mj828(uint8_t val)
{
	switch (val)														// based on array value at position #foo of array e.g. FooButtonCaseTable[]
	{
		case 0x01:														// error button press
			// TODO - implement device function on button error press
			EventHandler->UnSetEvent(val);
		break;

		case 0x02:														// pushbutton hold
			Device->led->Shine(Green);

			if (Device->button->button[Pushbutton].HoldToggle)
			{
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) , 0x20, 2);
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0xFF, 2);		// turn on rear light
			}
			else
			{
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) , 0x00, 2);
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0x00, 2);		// turn off rear light
			}
			EventHandler->UnSetEvent(val);
		break;

		//case 0x04:														// TODO - high beam momentary
			//if (Device->button->button[HighBeam].Momentary)
			//{
				//// FIXME - on button hold, multiple events are triggered and flapping occurs
				////Device->led->Shine(Blue);
				////MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) , 0xF8, 2);	// high beam on
//
			//}
			//else
			//{
				////MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | FRONT_LIGHT_HIGH) , 0x00, 2);	// high beam off
//
			//}
			//EventHandler->UnSetEvent(val);
		//break;

		case 0x08:														// TODO - brake light momentary
				Device->led->Shine(Red);

			if (Device->button->button[BrakeLight].Momentary)
			{
				//MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT) , 0x00, 2);
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | UTIL_LED_RED_ON) , 0x00, 1);
			}
			else
			{
				//MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | BRAKE_LIGHT) , 0x80, 2);
				MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | UTIL_LED_RED_OFF) , 0x00, 1);
			}

			// next case
			EventHandler->UnSetEvent(val);
		break;

		case 0x16:														// phototransistor action
			//Device->led->Shine(Yellow);
			
			if (Device->button->button[PhotoTransisitor].Momentary)
			{
				Device->led->Shine(Green);
			}
			else
			{
				Device->led->Shine(Yellow);
			}



			EventHandler->UnSetEvent(val);
		break;

		//case 0x32:														//next free case
			//// next case
			//EventHandler->UnSetEvent(val);
		//break;


		default:														// no value passed
			EventHandler->UnSetEvent(val);								// do nothing
		break;
	}
};

// toggles a bit in the LED flags variable; charlieplexer in tun makes it shine
void _PopulatedBusOperationMJ828(message_handler_t * const in_msg)
{
	volatile can_msg_t *msg = in_msg->ReceiveMessage();					// CAN message object

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if ( (msg->COMMAND & MASK_COMMAND) == CMND_DASHBOARD )				// dashboard command
	{
		__Device.public.led->Shine(((msg->COMMAND & 0x0E) >> 1));		// flag LED at appropriate index as whatever the command says
		return;
	}

	// FIXME - implement proper command nibble parsing; this here is buggy as hell (parsing for set bits is shitty at best)
	if (msg->COMMAND & MSG_BUTTON_EVENT_BUTTON0_OFF)					// if either button on or button off command
	{
		Device->button->button[Pushbutton].HoldToggle = !Device->button->button[Pushbutton].HoldToggle;					// toggle pushbutton state
		Device->led->Shine(Green);										// toggle green dashboard LED
		return;
	}
};

void mj828_ctor()
{
	// only SIDH is supplied since with the addressing scheme SIDL is always 0
	__Device.public.mj8x8 = mj8x8_ctor(MJ828_CAN_ID);					// call base class constructor & initialize own SID

	// GPIO state definitions
	{
	// state initialization of device-specific pins

	gpio_conf(PUSHBUTTON_pin, INPUT, LOW);								// SPST-NO - high on press, low on release
	// TODO - verify phototransistor operation
	gpio_conf(PHOTOTRANSISTOR_COLLECTOR_pin, INPUT, LOW);				// high on darkness, low on light
	// TODO - verify brake lever operation
	gpio_conf(HIGH_IN_pin, INPUT, HIGH);								// brake lever high beam - high - off (low B), low - on (high B)
	gpio_conf(BREAK_IN_pin, INPUT, HIGH);								// brake lever brake light - high - off (low B), low - on (high B)


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
//	PCMSK2 = (_BV(PCINT11) |											// enable pin change for sw1 @ pin D0
//			  _BV(PCINT12));											// enable pin change for sw2 @ pin D1

	sei();
	}

	__Device.public.led = _virtual_led_ctorMJ828();						// call virtual constructor & tie in object addresses
	__Device.public.button = _virtual_button_ctorMJ828();				// call virtual constructor & tie in object addresses

	__Device.public.mj8x8->PopulatedBusOperation = &_PopulatedBusOperationMJ828;	// implements device-specific operation depending on bus activity

	EventHandler->fpointer = &_event_execution_function_mj828;			// implements event hander for this device

	//__Device.public.led->Shine(Yellow);								// crude power indicator
};

#if defined(MJ828_)														// all devices have the object name "Device", hence the preprocessor macro
mj828_t * const Device = &__Device.public ;								// set pointer to MsgHandler public part
#endif