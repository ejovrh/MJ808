#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>

#include "main.h"

#if defined(MJ808_)														// mj808 header include
#include "mj808.h"
#endif
#if defined(MJ818_)														// mj818 header include
#include "mj818.h"
#endif
#if defined(MJ828_)														// mj828 header include
#include "mj828.h"
#endif


int main(void)
{
	event_handler_ctor();												// call event handler constructor; the Device constructor further down has the chance to override EventHandler.fpointer and implement its own handler

	#if defined(MJ808_)													// MJ808 - call derived class constructor and tie in base class
	mj808_ctor();
	#endif
	#if defined(MJ818_)													// MJ818 - call derived class constructor and tie in base class
	mj818_ctor();
	#endif
	#if defined(MJ828_)													// MJ828 - call derived class constructor and tie in base class
	mj828_ctor();
	#endif

	message_handler_ctor(Device->mj8x8->can);							// call message handler constructor

	// TODO - implement micro controller sleep cycles
	set_sleep_mode(SLEEP_MODE_IDLE);									// 11mA
	//set_sleep_mode(SLEEP_MODE_STANDBY);								// 10mA
	//set_sleep_mode(SLEEP_MODE_PWR_DOWN);								// 10mA
	sleep_enable();
	sleep_cpu();

	while (1)															// forever loop
	{
			EventHandler->HandleEvent();								// execute the event handling function with argument taken from case table array

		if (MCUCR & _BV(SE))											// if sleep is enabled
			sleep_cpu();												// ...sleep
	}
}


ISR(INT1_vect)															// ISR for INT1 - triggered by CAN message reception of the MCP2515
{
	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters

	inline void handle_message_error(can_t * const in_can)				// handles message error interrupts
	{
		in_can->BitModify(CANINTF, _BV(MERRF), 0x00);					// clear the flag
	};

	inline void helper_handle_rx(void)									// handles incoming message interrupts
	{
		Device->mj8x8->PopulatedBusOperation(MsgHandler);				// let the particular device deal with the message
	};

	void helper_handle_error(can_t * const in_can)						// handles RXBn overflow interrupts
	{
		in_can->BitModify(CANINTF, _BV(ERRIF), 0x00);					// clear the error interrupt flag

/*
		if (in_can->eflg & _BV(TXBO))									// TODO - handle bus off situation
		{
			;
		}

		if (in_can->eflg & _BV(TXEP))									// handle TX error-passive situation
		{
			Device.mj8x8->can->Sleep(in_can, 1);						// put to sleep
		}

		if (in_can->eflg & _BV(RXEP))									// TODO - handle RX error-passive situation
		{
			;
		}

		if (in_can->eflg & _BV(TXWAR))									// TODO - handle TX waring situation
		{
			// TODO - log it
			;
		}

		if (in_can->eflg & _BV(RXWAR))									// TODO - handle RX warning situation
		{
			// TODO - log it
			;
		}
*/

		if (in_can->eflg & _BV(RX0OVR))									// RXB0 overflow - datasheet figure 4.3, p. 26
		{
			// FIXME - check for correct RX buffer clearing
			helper_handle_rx();											// handle the message
			in_can->BitModify(EFLG, _BV(RX0OVR), 0x00);					// clear the overflow bit
			return;
		}

		if (in_can->eflg & _BV(RX1OVR))									// RXB1 overflow - datasheet figure 4.3, p. 26
		{
			helper_handle_rx();											// handle the message
			in_can->BitModify(EFLG, _BV(RX1OVR), 0x00);					// clear the overflow bit
			return;
		}
	};

	void helper_handle_wakeup(can_t *const in_can)						// handles wakeup interrupts
	{
		// functionally, this function is similar to can_sleep(), but still different in one aspect:
			// can_sleep(foo_can, 0) wakes up by triggering a wake up interrupt, which helper_handle_wakeup() handles
			// helper_handle_wakeup() can be triggered by any external event while can_sleep(foo_can, 0) is called from within this program

		in_can->ChangeOpMode(REQOP_CONFIG);								// put into config mode -> clears all error counters; other settings appear to remain consistent
		in_can->ChangeOpMode(REQOP_NORMAL);								// put back into normal mode

		in_can->BitModify(CANINTF, _BV(WAKIF), _BV(WAKIF));				// clear the wakeup flag
		in_can->BitModify(CANINTF, 0xFF, 0x00);							// clear the wakeup flag
		gpio_conf(MCP2561_standby_pin, OUTPUT, LOW);					// wake up MCP2561

		in_can->in_sleep = 0;
	};

	inline void helper_handle_tx(can_t *const in_can)
	{
		in_can->BitModify(CANINTF, 0x1C, 0x00);
	};

	sleep_disable();													// wakey wakey

//#define BRANCHTABLE_ICOD

	can_t *can = Device->mj8x8->can;									// get pointer to CAN instance

#if defined(BRANCHTABLE_ICOD)
	static const uint16_t (*fptr)(can_t *in_can);						// declare pointer for function pointers in branchtable_led[]
	void (* const branchtable_icod[])(can_t *in_can) PROGMEM =			// array of function pointers for basic LED handling in PROGMEM
	{
		&DoNothing,														// icod value 0
		&helper_handle_error,											// icod value 1
		&helper_handle_wakeup,											// icod value 2
		&helper_handle_tx,												// icod value 3
		&helper_handle_tx,												// icod value 4
		&helper_handle_tx,												// icod value 5
		&helper_handle_rx,												// icod value 6
		&helper_handle_rx												// icod value 7
	};
#endif


	do		// ICOD loop handler - runs while ICOD != 0
	{
		can->ReadBytes(TEC, &can->tec, 2);								// read in TEC and REC
		can->ReadBytes(CANINTF, &can->canintf, 3);						// read in CANINTF and EFLG
		can->ReadBytes(CANCTRL, &can->canctrl, 1);

		can->icod =  ((can->canstat & 0x0E) >> 1);						// right shift so that CANSTAT.U0 cant interfere

#if defined(BRANCHTABLE_ICOD)
		fptr = pgm_read_ptr(&branchtable_icod[can->icod]);				// get appropriate function pointer from PROGMEM
		(fptr)(can);
#endif
#if !defined(BRANCHTABLE_ICOD)

		// TODO - implement something like a vpointer lookup table or branch table instead of this shit:
		switch (can->icod)												// handling of cases depending on ICOD value - sort of priority-style

		{																// while loops over ICOD bit values, each case handles an ICOD situation
			case 0:														// no interrupt
				break;

			case 1:														// error interrupt
				helper_handle_error(can);
				break;

			case 2:														// wake-up interrupt
				helper_handle_wakeup(can);
				break;

			case 3:														// TXB0 interrupt
				helper_handle_tx(can);
				break;

			case 4:														// TXB1 interrupt
				helper_handle_tx(can);
				break;

			case 5:														// TXB2 interrupt
				helper_handle_tx(can);
				break;

			case 6:														// RXB0 interrupt
				helper_handle_rx();
				break;

			case 7:														// RXB1 interrupt
				helper_handle_rx();
				break;
		};
#endif

	} while (can->icod);

	sleep_enable();														// back to sleep
}

// port change interrupts - not used yet
/*
ISR(PCINT2_vect)														// pin-change ISR for pushbuttons
{
	sleep_disable();													// wakey wakey

	;

	sleep_enable();														// back to sleep
}
*/

#if ( defined(MJ808_) | defined(MJ828_) )								// ISR for timers 1 A compare match - button handling
ISR(TIMER1_COMPA_vect)													// timer/counter 1 - button debounce - 25ms
{
	// code to be executed every 25ms
	sleep_disable();													// wakey wakey

	for (uint8_t i=0; i<Device->button->button_count; ++i)				// loop over all available buttons and debounce them
		Device->button->deBounce(&Device->button->button[i], EventHandler);		// from here on the button is debounced and states can be consumed

	sleep_enable();														// back to sleep
}

#if defined(MJ828_)														// ISR for timer0 - 16.25ms - charlieplexing timer
ISR(TIMER0_COMPA_vect)													// timer/counter0 - 16.25ms - charlieplexed blinking
{
		 charlieplexing_handler(Device->led->flags);					// handles LEDs according to CAN message (of type CMND_UTIL_LED)
}
#endif

#endif