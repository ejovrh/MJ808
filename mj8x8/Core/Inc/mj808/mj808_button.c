#ifndef MJ808_BUTTON_C_
#define MJ808_BUTTON_C_

#if defined(MJ808_)	// if this particular device is active

#include "button\button_types_actual.c"	// various __*button_t types - for convenience in one place for all LED devices

extern void __HandleButton(__individual_button_t *in_button, event_handler_t *const in_event);

// wrapper function for the debouncer
static void _ButtonHandler()
{
	__HandleButton(&__Button.__button[PushButton], EventHandler);  // one button -> no loop: call directly
}

static uint8_t PushButtonCaseTable[] =  // array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
		0,  // 0 - button momentary
		3,  // 1 - button toggle
		2,  // 2 - button hold
		1,  // 3 - error event
		0,  // 4 - double press event
		0,  // 5 - future use
		0,  // 6 - future use
		0	// 7 - future use
	};

// marker function for button
static inline void _MarkPushButton(const uint8_t state)
{
	__Button.__button[PushButton].__state = state;	// mark whatever state the button is in
}

static __individual_button_t __button_actual [BUTTON_COUNT] =
	{0};  // initialize array of 3 structs, one for each of the 3 device buttons

// implementation of virtual constructor for buttons
button_t* _virtual_button_ctorMJ808()
{
	__button_actual[PushButton].__ButtonCaseptr = PushButtonCaseTable;	// set the casetable
	__button_actual[PushButton].public.Mark = &_MarkPushButton;  // set function pointer

	__Button.__button = __button_actual;  // assign pointer to button array

	__Button.public.Handler = &_ButtonHandler;	// set function pointer

	__Button.public.button[PushButton] = &__button_actual[PushButton].public;  // expose public part

	return &__Button.public;	// return address of public part; calling code accesses it via pointer
}

#endif // MJ808_

#endif /* MJ808_BUTTON_C_ */
