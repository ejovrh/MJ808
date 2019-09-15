#ifndef MJ828_BUTTON_C_
#define MJ828_BUTTON_C_

#include "mj828.h"
#include "button_types_actual.c"										// various __*button_t types - for convenience in one place for all LED devices

extern void _debounce(individual_button_t *in_button, event_handler_t * const in_event);

static void _debounce_wrapper()
{
	for (uint8_t i=0; i<__Button.__button_count; ++i)					// loop over all available buttons and debounce them
		_debounce(&__Button.public.button[i], EventHandler);			// from here on the button is debounced and states can be consumed
};

// implementation of virtual constructor for buttons
static button_t *_virtual_button_ctorMJ828(void)
{
	static individual_button_t individual_button[2] __attribute__ ((section (".data")));	// define array of actual buttons and put into .data

	__Button.public.button = individual_button;							// assign pointer to button array
	__Button.__button_count = 2;										// how many buttons are on this device?
	__Button.public.button[Right]._PIN = (uint8_t *) 0x30;				// 0x020 offset plus address - PIND register
	__Button.public.button[Right]._pin_number = 0;						// sw2 is connected to pin D0
	__Button.public.button[Left]._PIN = (uint8_t *) 0x30;				// ditto
	__Button.public.button[Left]._pin_number = 1;						// sw2 is connected to pin D1

	static uint8_t ButtonLeftCaseTable[] =								// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{
		0x04,	// 0 - jump case 0x04 - momentary event
		0x00,	// 1 - not defined
		0x00,	// 2 - not defined
		0x01	// 3 - not defined
	};

	static uint8_t ButtonRightCaseTable[] =								// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{
		0x00,	// 0 - not defined
		0x02,	// 1 - jump case 0x02 - toggle event
		0x00,	// 2 - not defined
		0x01	// 3 - not defined
	};

	__Button.public.button[Left].ButtonCaseptr = ButtonLeftCaseTable;	// button press-to-case binding
	__Button.public.button[Right].ButtonCaseptr = ButtonRightCaseTable;	// button press-to-case binding

	__Button.public.deBounce = &_debounce_wrapper;						// tie in debounce function

	return &__Button.public;											// return address of public part; calling code accesses it via pointer
};

#endif /* MJ828_BUTTON_C_ */