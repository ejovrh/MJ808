#include "button_types_actual.c"
#include "button.h"

// button debouncer for devices with buttons
void __HandleButton(__individual_button_t *const in_button, event_handler_t *const in_event)
{
	// TODO - write button handler rationale

	if(in_button->__state)  // 1 - pressed down
		{
			++in_button->__hold_counter;	// button is pressed. start to count (used to determine long button press)

			// BUTTON PRESSED DOWN (meta-stable)
			if(!in_button->public.Momentary)
				{
					in_button->public.Momentary = 1;
					in_event->Notify(in_button->__ButtonCaseptr[CaseMomentary]);	// notify event handler of button press
				}
		}
	else
		{
			// BUTTON PRESS TOGGLE (stable states)
			if(in_button->__hold_counter > 4 && in_button->__hold_counter < 20)
				{
					in_button->public.Toggle = !in_button->public.Toggle;
					in_event->Notify(in_button->__ButtonCaseptr[CaseToggle]);  // notify event handler of button press
					in_button->__hold_counter = 0;
					return;
				}

			// BUTTON PRESS HOLD (stable states)
			if(in_button->__hold_counter > 30)  // more than 750ms
				{
					in_button->public.Hold = !in_button->public.Hold;
					in_event->Notify(in_button->__ButtonCaseptr[CaseHold]);  // notify event handler of button press
					in_button->__hold_counter = 0;
					return;
				}

			// BUTTON RELEASED (Stable)
			if(in_button->public.Momentary)
				{
					in_button->public.Momentary = 0;
					in_event->Notify(in_button->__ButtonCaseptr[CaseMomentary]);	// notify event handler of button press
				}
		}
}
