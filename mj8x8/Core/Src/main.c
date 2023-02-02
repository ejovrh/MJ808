#include "main.h"	// device core config

int main(void)
{
	__disable_irq();	// disable interrupts - the actual device constructor will enable them...

	event_handler_ctor();  // call event handler constructor; the Device constructor further down has the chance to override EventHandler.fpointer and implement its own handler

#if defined(MJ808_)	// MJ808 - call derived class constructor and tie in base class
	mj808_ctor();
#endif
#if defined(MJ818_)	// MJ818 - call derived class constructor and tie in base class
	mj818_ctor();
#endif
#if defined(MJ828_)	// MJ828 - call derived class constructor and tie in base class
	mj828_ctor();
#endif
#if defined(MJ838_)	// Čos - call derived class constructor and tie in base class
	cos_ctor();
#endif
#if defined(MJ848_)	// LU - call derived class constructor and tie in base class
	lu_ctor();
#endif

	message_handler_ctor(Device->mj8x8->can);  // call message handler constructor

	__enable_irq();  // enable interrupts

//	while(1)  // THE loop...
//		{
//			;
//		}
}
