#ifndef MJ808_BUTTON_C_
#define MJ808_BUTTON_C_

#include "mj808\mj808.h"
#include "button\button_types_actual.c"										// various __*button_t types - for convenience in one place for all LED devices

extern void _debounce(__individual_button_t *in_button, event_handler_t * const in_event);

// wrapper function for the debouncer
static void _debounce_wrapper()
{
	 _debounce(&__Button.__button[Center], EventHandler);				// one button -> no loop: call directly
};

static uint8_t __ButtonCenterCaseTable[] =								// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x00,	// 0 - not defined
	0x00,	// 1 - not defined
	0x02,	// 2 - jump case 0x02 - button Hold
	0x01,	// 3 - jump case 0x01 - error event
};

static __individual_button_t __button_actual[] =						// instantiate array of actual button struct
{
	// index 0 - that one button...
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 4,												// sw1 is connected to pin D4
		.__ButtonCaseptr = __ButtonCenterCaseTable						// button press-to-case binding
	}
};

// implementation of virtual constructor for buttons
static button_t *_virtual_button_ctorMJ808()
{
	__Button.__button = __button_actual;								// assign pointer to button array
	__Button.__button_count = 1;										// how many buttons are on this device?

	__Button.public.button = &__button_actual[Center].public;			// pointer to public members of button actual
	__Button.public.deBounce = &_debounce_wrapper;						// tie in debounce function

	return &__Button.public;											// return address of public part; calling code accesses it via pointer
};

#endif /* MJ808_BUTTON_C_ */