#include <avr/interrupt.h>
#include <avr/sleep.h>

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

// FIXME - get rid of this here
	void helper_handle_rx(void)											// handles incoming message interrupts
	{
		Device->mj8x8->PopulatedBusOperation(MsgHandler);				// let the particular device deal with the message
//		EventHandler->Notify(0x08);
	};


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