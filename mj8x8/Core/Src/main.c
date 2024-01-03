#include "main.h"	// device core config
#include "try\try.h"	// top-level object for bus-wide device handling

int main(void)
{
	__disable_irq();	// disable interrupts until end of all initialisations

#if USE_EVENTHANDLER
	event_handler_ctor();  // call event handler constructor; the Device constructor further down has the chance to override EventHandler.fpointer and implement its own handler
#endif

#if USE_I2C
	i2c_ctor(I2C_SDA_Pin, I2C_SCL_Pin, I2C_SDA_GPIO_Port);	// call I2C constructor
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
#endif
#if defined(MJ848_)	// LU - call derived class constructor and tie in base class
	mj848_ctor();
#endif
#if defined(MJ514_)	// electronic shifter - call derived class constructor and tie in base class
	mj514_ctor();
#endif

	message_handler_ctor(Device->mj8x8->can);  // call message handler constructor

	try_ctor();  // call top-level object constructor

	__enable_irq();  // enable interrupts

//	while(1)  // THE loop...
//		{
//			;
//		}
}
