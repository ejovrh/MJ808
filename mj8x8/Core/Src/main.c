#include "main.h"	// device core config
#include "try/try.h"	// top-level object for bus-wide device handling
#if USE_SPI && USE_LOGGER
#include "logger/logger.h"  // logging functionality
#endif

int main(void)
{
	__disable_irq();	// disable interrupts until end of all initialisations

#if USE_EVENTHANDLER
	event_handler_ctor();  // call event handler constructor; the Device constructor further down has the chance to override EventHandler.fpointer and implement its own handler
#endif

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
	mj838_ctor();
#if USE_SPI && USE_LOGGER
	extern const char LogFileName[11];	// log file name for logger
	logger_ctor(LogFileName, (void*) Device->SDCard);  // pass SD card object as void*
#endif
#endif
#if defined(MJ848_)	// LU - call derived class constructor and tie in base class
	mj848_ctor();
#endif
#if defined(MJ514_)	// Rohloff e14 shifter unit - call derived class constructor and tie in base class
	mj514_ctor();
#endif
#if defined(MJ515_)	// Rohloff grip shifter electrical unit - call derived class constructor and tie in base class
	mj515_ctor();
#endif

	message_handler_ctor(Device->mj8x8->can);  // call message handler constructor

	try_ctor();  // call top-level object constructor

	__enable_irq();  // enable interrupts

//	while(1)  // THE loop...
//		{
//			;
//		}
}
