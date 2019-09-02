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


	// assumption: an incoming message is of interest for this unit
	//	'being of interest' is defined in the filters




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
