#ifndef MJ828_BUTTON_C_
#define MJ828_BUTTON_C_

#if defined(MJ828_)	// if this particular device is active

#include "button\button_types_actual.c"	// various __*button_t types - for convenience in one place for all LED devices

extern void __HandleButton(__individual_button_t *const in_button, event_handler_t *const in_event);

// wrapper function for the debouncer
static void _ButtonHandler()
{
	for(uint8_t i = 0; i < BUTTON_COUNT; ++i)  // loop over all available buttons and debounce them
		__HandleButton(&__Button.__button[i], EventHandler);  // from here on the button is debounced and states can be consumed
}

static uint8_t PushButtonCaseTable[] =	// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
			EVENT04,  // 0 - button momentary
			EVENT06,  // 1 - button toggle
			EVENT05,  // 2 - button hold
			EVENT01,  // 3 - error event
			DONOTHING,  // 4 - double press event
			DONOTHING,  // 5 - future use
			DONOTHING,  // 6 - future use
			DONOTHING	// 7 - future use
	};

static uint8_t LeverBrakeCaseTable[] =  // array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
			EVENT02,  // 0 - button momentary - braking action
			DONOTHING,  // 1 - button toggle
			DONOTHING,  // 2 - button hold
			EVENT01,  // 3 - error event
			DONOTHING,  // 4 - double press event
			DONOTHING,  // 5 - future use
			DONOTHING,  // 6 - future use
			DONOTHING	// 7 - future use
	};

static uint8_t LeverFrontCaseTable[] =  // array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
	{  //
			EVENT03,  // 0 - button momentary - high beam
			DONOTHING,  // 1 - button toggle
			DONOTHING,  // 2 - button hold
			EVENT01,  // 3 - error event
			DONOTHING,  // 4 - double press event
			DONOTHING,  // 5 - future use
			DONOTHING,  // 6 - future use
			DONOTHING	// 7 - future use
	};

// marker function for button
static inline void _MarkPushButton(const uint8_t state)
{
	__Button.__button[PushButton].__state = state;	// mark whatever state the button is in
}

// marker function for button
static inline void _MarkLeverFront(const uint8_t state)
{
	__Button.__button[LeverFront].__state = state;  // see above
}

// marker function for button
static inline void _MarkLeverBrake(const uint8_t state)
{
	__Button.__button[LeverBrake].__state = state;
}

static __individual_button_t __button_actual [BUTTON_COUNT] =
	{0};  // initialize array of 3 structs, one for each of the 3 device buttons

// implementation of virtual constructor for buttons
button_t* _virtual_button_ctor(void)
{
	__button_actual[PushButton].__ButtonCaseptr = PushButtonCaseTable;	// set the casetable
	__button_actual[LeverFront].__ButtonCaseptr = LeverFrontCaseTable;	// ditto
	__button_actual[LeverBrake].__ButtonCaseptr = LeverBrakeCaseTable;

	__button_actual[PushButton].public.Mark = &_MarkPushButton;  // set function pointer
	__button_actual[LeverFront].public.Mark = &_MarkLeverFront;  // ditto
	__button_actual[LeverBrake].public.Mark = &_MarkLeverBrake;

	__Button.__button = __button_actual;  // assign pointer to button array

	__Button.public.Handler = &_ButtonHandler;	// set function pointer
	__Button.public.button[PushButton] = &__button_actual[PushButton].public;  // expose public part
	__Button.public.button[LeverFront] = &__button_actual[LeverFront].public;  // ditto
	__Button.public.button[LeverBrake] = &__button_actual[LeverBrake].public;

	return &__Button.public;	// return address of public part; calling code accesses it via pointer
}

#endif // MJ828_

#endif /* MJ828_BUTTON_C_ */
