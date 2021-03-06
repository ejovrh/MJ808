#include "button_types_actual.c"
#include "button.h"

// button debouncer for devices with buttons
void _debounce(__individual_button_t * const in_button, event_handler_t * const in_event)
{
	inline void local_advance_counter(void)								// local helper function which advances the debounce "timer"
	{
		++in_button->__hold_counter;									// start to count (used to determine long button press; not used for debouncing)


		++in_button->__state;											// debouncing by means of bit shifting - redux

																		// debouncing by means of bit shifting
		//in_button._state ^ 0x03;										// XOR what we currently have in state
		//in_button._state <<= 1;										// left shift so that if we have 0x01 this becomes 0x02
		//in_button._state |= 0x01;										// OR what we have with 0x01 so that 0x02 becomes 0x03
	}

	/*	rationale of debouncing:
	 *		- the ISR fires every 25ms, which means the sample rate of a button press is once per 25ms
	 *		- if the button is held for only one iteration (bounce state) we have 0x01 and on the next iteration state is reset to 0x00 [1st 25ms]
	 *		- if the button is held for two iterations, on the first iteration state is 0x01 and on the second 0x03 [2nd 25ms]
	 *		- after 2 iterations 50ms have passed -> we call that a stable state
	 *
	 *		detecting the stable state boils down to:
	 *			- counting the duration of the stable state in 25ms intervals
	 *			- comparing a bit-shifted number (count of 25ms intervals) to another value (BUTTON_MIN_PRESS_TIME or BUTTON_MAX_PRESS_TIME)
	 *
	 *		the debouncer does only that - it debounces
	 *
	 *		the state marker marks the button state for external code to make sense of it
	 *		valid states:
	 *			- "Momentary" - key is pressed (and held for a short time), while button is pressed - one state, when it is released back to the original state
	 *			- "Toggle" - toggled on/off state, once per key press the state changes and remains changed until next press
	 *			- "Hold" - held for e.g 1s to turn something on/off, akin to Momentary but a longer press is needed to change from one state to another
	 *			- "hold_error" - held constantly (e.g. by error) - after a timeout revert to original state
	 */
																		// CHECKME - random spikes (not true button press events) every 25ms might be an issue
	#if defined(MJ828_)													// inverted
	if (	!( *(in_button->__PIN) & (1<<in_button->__pin_number))	)	// if in the given PIN register the given button is pressed
	#endif
	#if defined(MJ808_)													// non-inverted
	if (	( *(in_button->__PIN) & (1<<in_button->__pin_number))	)	// if in the given PIN register the given button is pressed
	#endif
	{												// button is pressed
		local_advance_counter();										// debouncing happens here

		if (in_button->__hold_counter >= BUTTON_MAX_PRESS_TIME)			// too long button press -> error state
		{																// turn everything off
			in_button->__state = 0;										// reset state
			in_button->public.Momentary = 0;							// mark as currently not pressed
			in_button->__was_pressed = 1;								// mark button as "was pressed" - this is the previous state in the next iteration

			// order is important
			in_button->public.ErrorHold = 1;							// mark as error state
		    in_event->Notify(in_button->__ButtonCaseptr[CaseErrorHold]);// notify event handler of button press

			in_button->public.Toggle = 0;								// toggled due to error state -> reset to default value
			in_button->public.Hold = 0;									// mark as hold_temp off
			return;														// get out
		}
	}
#if ( defined(MJ808_) | defined(MJ828_) )
	else											// button is released
#endif
	{
		if (in_button->__is_at_default)									// if we are in zero state
			return;														// no need to do anything

																		// button is released
																		// button is not pressed - reset everything to default values
		in_button->__state = 0;											// reset state
		in_button->public.Momentary = 0;								// mark as currently not pressed

		in_button->__was_pressed = 1;									// mark button as "was pressed" - this is the previous state in the next iteration
		in_button->public.ErrorHold = 0;								// release error flag, since button is now released
		in_button->__hold_counter = 0;									// set counter back to 0
		in_button->public.Momentary = 0;								// mark as currently not pressed
		in_button->__is_at_default = 1;
		return;															// finish
	}

	// state markers
	if (in_button->__state == 0x03 && !in_button->public.ErrorHold)		// if we have a non-error steady state
	{
		if (in_button->__hold_counter >= BUTTON_MIN_PRESS_TIME)			// pressed for a valid amount of time
		{
			if (!in_button->__was_pressed)								// previous state (prevent flapping on/off)
			{
				// order is important
				in_button->public.Hold = !in_button->public.Hold;		// set "hold_temp" state
				in_event->Notify(in_button->__ButtonCaseptr[CaseHold]);	// notify event handler of button press
			}

			in_button->__was_pressed = 1;								// mark the button as being pressed
			return;
		}

		if (in_button->__was_pressed)									// previous state (prevent flapping on/off)
		{
			// order is important
			in_button->public.Toggle = !in_button->public.Toggle;		// set "toggle" state
			in_event->Notify(in_button->__ButtonCaseptr[CaseToggle]);	// notify event handler of button press
		}

		// order is important
		if (!in_button->public.Momentary)
		{
			in_button->public.Momentary = 1;							// set "is_pressed" state
			in_event->Notify(in_button->__ButtonCaseptr[CaseMomentary]);// notify event handler of button press
		}

		in_button->__was_pressed = 0;									// mark the button as being pressed
		in_button->__is_at_default = 0;
	}
};