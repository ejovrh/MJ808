#ifndef MJ828_BUTTON_C_
#define MJ828_BUTTON_C_

#include "mj828.h"
#include "button_types_actual.c"										// various __*button_t types - for convenience in one place for all LED devices

extern void _debounce(__individual_button_t * const in_button, event_handler_t * const in_event);

// wrapper function for the debouncer
static void _debounce_wrapper()
{
	for (uint8_t i=0; i<__Button.__button_count; ++i)					// loop over all available buttons and debounce them
		_debounce(&__Button.__button[i], EventHandler);					// from here on the button is debounced and states can be consumed
};

static uint8_t ButtonLeftCaseTable[] =									// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x04,	// 0 - jump case 0x04 - momentary event
	0x00,	// 1 - not defined
	0x00,	// 2 - not defined
	0x01	// 3 - not defined
};

static uint8_t ButtonRightCaseTable[] =									// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x00,	// 0 - not defined
	0x02,	// 1 - jump case 0x02 - toggle event
	0x00,	// 2 - not defined
	0x01	// 3 - not defined
};

//FIXME - left button doesn't work anymore as previously
static __individual_button_t __button_actual[] =						// instantiate array of actual button struct's
{
	// index 0 - right button
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 0,												// sw1 is connected to pin D0
		.__ButtonCaseptr = ButtonRightCaseTable							// button press-to-case binding
	},
	// index 1 - left button
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 1,												// sw2 is connected to pin D1
		.__ButtonCaseptr = ButtonLeftCaseTable							// button press-to-case binding
	}
};

static individual_button_t * const __button_public[] =					// array of pointers to public members of button actual
{
	&__button_actual[Right].public,
	&__button_actual[Left].public
};

// implementation of virtual constructor for buttons
static button_t *_virtual_button_ctorMJ828(void)
{
	__Button.__button = __button_actual;								// assign pointer to button array
	__Button.__button_count = 2;										// how many buttons are on this device?

	__Button.public.button = *__button_public;							// assign pointer to array of pointers to public members of button actual
	__Button.public.deBounce = &_debounce_wrapper;						// tie in debounce function

	return &__Button.public;											// return address of public part; calling code accesses it via pointer
};

#endif /* MJ828_BUTTON_C_ */