#ifndef BUTTON_H_
#define BUTTON_H_

#include <inttypes.h>

#include "event.h"

#define BUTTON_MIN_PRESS_TIME 20										// number times 25ms duration: 500ms
#define BUTTON_MAX_PRESS_TIME 120										// number times 25ms duration: 3s

enum button_casetable_index												// represents index of FooButtonCaseTable[] (uniquely defined in device button constructor)
{
	Momentary,															// array index 0 -- momentary press of pushbutton
	Toggle,																// array index 1 -- momentary press of button toggles
	Hold,																// array index 2 -- longer press of button toggles
	ErrorHold															// array index 3 -- lean against tree and thereby depress button unintentionally
};

typedef struct 															// struct describing a generic pushbutton
{
	uint8_t *ButtonCaseptr;												// pointer to array in which button events are bound to switch-case statements in __mjxxx_button_execution_function
	uint8_t *_PIN;														// PIN register address of button pin
	uint8_t _pin_number;												// pin number (0, 1...6) to which the button is connected

	uint8_t Momentary :1;												// flag indicating if button is pressed right now
	uint8_t Toggle :1;													// flag indicating the toggle state
	uint8_t Hold :1;													// flag indicating a button press for a duration of BUTTON_MIN_PRESS_TIME (up to BUTTON_MAX_PRESS_TIME) seconds, followed by button release
	uint8_t ErrorHold :1;												// flag indicating constant button press (by error, object leaning on pushbutton, etc.)
	uint8_t _hold_counter;												// counter to count button press duration for hold_X states
	uint8_t _state : 2;													// something akin to a "counter" used for debouncing
	uint8_t _was_pressed :1;											// flag indicating if button was released after a stable state (used to remember previous state)
	uint8_t _is_at_default :1;											// 1 - default values, 0 otherwise
} individual_button_t;

typedef struct button_t
{
	individual_button_t *button;										// "virtual" pointer to array of buttons present on particular device
	volatile uint8_t button_count : 2;									// max. 4 buttons

	void (*deBounce)(volatile individual_button_t *in_button, volatile event_handler_t *in_event);			// button debouncer
	//void (*virtual_button_ctor)(volatile struct button_t *self);		// "virtual" pointer to array of button present on particular device
} button_t;

#endif /* BUTTON_H_ */