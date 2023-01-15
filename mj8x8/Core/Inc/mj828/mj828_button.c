#ifndef MJ828_BUTTON_C_
#define MJ828_BUTTON_C_

#include "main.h"
#if defined(MJ828_)	// if this particular device is active

#include "mj828\mj828.h"
#include "button\button_types_actual.c"	// various __*button_t types - for convenience in one place for all LED devices

extern void __HandleButton(__individual_button_t *const in_button, event_handler_t *const in_event);

// wrapper function for the debouncer
static void _ButtonHandler()
{
	for(uint8_t i = 0; i < __Button.__button_count; ++i)	// loop over all available buttons and debounce them
		__HandleButton(&__Button.__button[i], EventHandler);  // from here on the button is debounced and states can be consumed
}

static uint8_t PushButtonCaseTable[] =	// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
	0x08,  // 0 - button momentary - jump to case 0x08
	0x00,  // 1 - button toggle - not defined
	0x00,  // 2 - button hold - not defined
	0x01	// 3 - error event - jump to case 0x01
	};

static uint8_t LeverBrakeCaseTable[] =  // array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
	0x02,  // 0 - button momentary - jump to case 0x02 - braking action
	0x00,  // 1 - button toggle - not defined
	0x00,  // 2 - button hold - not defined
	0x01	// 3 - error event - jump to case 0x01
	};

static uint8_t LeverFrontCaseTable[] =  // array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
	0x04,  // 0 - button momentary - jump to case 0x04 - high beam
	0x00,  // 1 - button toggle - not defined
	0x00,  // 2 - button hold - not defined
	0x01	// 3 - error event - jump to case 0x01
	};

static __individual_button_t            __button_actual [] =  // instantiate array of 3 structs, one for each of the 3 device buttons
	{
	// index 0 - pushbutton
		{  //
		.__ButtonCaseptr = PushButtonCaseTable  // button press-to-case binding
		},//
	// index 1 - brake lever braking button
		{  //
		.__ButtonCaseptr = LeverBrakeCaseTable	// button press-to-case binding
		},//
	// index 2 - brake lever pushed forward button
		{  //
		.__ButtonCaseptr = LeverFrontCaseTable  // button press-to-case binding
		}//
	};

static individual_button_t *const __button_public[] =  // instantiate array of _public_ parts of the __button_actual array,
	{  //
	&__button_actual[PushButton].public,  //
	&__button_actual[LeverBrake].public,  //
	&__button_actual[LeverFront].public  //
	};

// implementation of virtual constructor for buttons
static button_t* _virtual_button_ctorMJ828(void)
{
	__Button.__button = __button_actual;	// assign pointer to button array
	__Button.__button_count = 3;	// how many buttons are on this device?

	__Button.public.button = *__button_public;	// assign pointer to array of pointers to public members of button actual
	__Button.public.Handler = &_ButtonHandler;  // tie in debounce function

	return &__Button.public;	// return address of public part; calling code accesses it via pointer
}

#endif // MJ828_

#endif /* MJ828_BUTTON_C_ */
