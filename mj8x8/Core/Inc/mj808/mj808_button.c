#ifndef MJ808_BUTTON_C_
#define MJ808_BUTTON_C_

#include "main.h"
#if defined(MJ808_)	// if this particular device is active

#include "mj808\mj808.h"
#include "button\button_types_actual.c"	// various __*button_t types - for convenience in one place for all LED devices

extern void __HandleButton(__individual_button_t *in_button, event_handler_t *const in_event);

// wrapper function for the debouncer
static void _ButtonHandler()
{
	__HandleButton(&__Button.__button[Pushbutton], EventHandler);	// one button -> no loop: call directly
}

static uint8_t __ButtonCenterCaseTable[] =	// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
	0x00,  // 0 - button momentary - not defined
	0x04,  // 1 - button toggle - jump to case 0x04
	0x02,  // 2 - button hold - jump to case 0x02
	0x01,  // 3 - error event - jump to case 0x01
	};

static __individual_button_t   __button_actual [] =  // instantiate array of actual button struct
	{
	// index 0 - that one button...
		{//
		.__ButtonCaseptr = __ButtonCenterCaseTable	// button press-to-case binding
		}//
	};

// implementation of virtual constructor for buttons
static button_t* _virtual_button_ctorMJ808()
{
	__Button.__button = __button_actual;	// assign pointer to button array
	__Button.__button_count = 1;	// how many buttons are on this device?

	__Button.public.button = &__button_actual[Pushbutton].public;  // pointer to public members of button actual
	__Button.public.Handler = &_ButtonHandler;  // tie in debounce function

	return &__Button.public;	// return address of public part; calling code accesses it via pointer
}

#endif // MJ808_

#endif /* MJ808_BUTTON_C_ */
