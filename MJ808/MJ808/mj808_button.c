#ifndef MJ808_BUTTON_C_
#define MJ808_BUTTON_C_

#include "mj808.h"
#include "button_types_actual.c"										// various __*button_t types - for convenience in one place for all LED devices

extern void _debounce(individual_button_t *in_button, event_handler_t * const in_event);

static void _debounce_wrapper()
{
	for (uint8_t i=0; i<__Button.__button_count; ++i)					// loop over all available buttons and debounce them
		_debounce(&__Button.public.button[i], EventHandler);			// from here on the button is debounced and states can be consumed
};

// implementation of virtual constructor for buttons
button_t *_virtual_button_ctorMJ808()
{
	static individual_button_t individual_button[1] __attribute__ ((section (".data")));		// define array of actual buttons and put into .data

	__Button.public.button = individual_button;							// assign pointer to button array
	__Button.__button_count = 1;										// how many buttons are on this device?
	__Button.public.button[Center]._PIN = (uint8_t *) 0x30; 			// 0x020 offset plus address - PIND register
	__Button.public.button[Center]._pin_number = 4;						// sw2 is connected to pin D0

	static uint8_t ButtonCenterCaseTable[] =							// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{
		0x00,	// 0 - not defined
		0x00,	// 1 - not defined
		0x02,	// 2 - jump case 0x02 - button Hold
		0x01,	// 3 - jump case 0x01 - error event
	};

	__Button.public.button[Center].ButtonCaseptr = ButtonCenterCaseTable;			// button press-to-case binding

	__Button.public.deBounce = &_debounce_wrapper;						// tie in debounce function

	return &__Button.public;											// return address of public part; calling code accesses it via pointer
};

#endif /* MJ808_BUTTON_C_ */