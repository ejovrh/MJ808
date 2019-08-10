#ifndef BUTTON_H_
#define BUTTON_H_

#include <inttypes.h>

#define BUTTON_MIN_PRESS_TIME 20										// number times 25ms duration: 500ms
#define BUTTON_MAX_PRESS_TIME 120										// number times 25ms duration: 3s

typedef struct button_t													// struct describing a generic pushbutton
{
	uint8_t state : 2;													// something akin to a "counter" used for debouncing
	uint8_t *PIN;														// PIN register address of button pin
	uint8_t pin_number;													// pin number (0, 1...6) to which the button is connected
	uint8_t is_pressed :1;												// flag indicating if button is pressed right now
	uint8_t was_pressed :1;												// flag indicating if button was released after a stable state (used to remember previous state)
	uint8_t toggle :1;													// flag indicating the toggle state
	uint8_t hold_temp :1;												// flag indicating a button press for a duration of BUTTON_MIN_PRESS_TIME (up to BUTTON_MAX_PRESS_TIME) seconds, followed by button release
	uint8_t hold_error :1;												// flag indicating constant button press (by error, object leaning on pushbutton, etc.)
	uint8_t hold_counter;												// counter to count button press duration for hold_X states
	uint8_t is_at_default :1;											// 1 - default values, 0 otherwise

	void (*virtual_button_ctor)(volatile struct button_t *self);		// virtual constructor
} button_t;

void button_debounce(volatile button_t *in_button);						// marks a button as pressed if it was pressed for the duration of 2X ISR iterations

extern volatile button_t BUTTON;										// declare button object

#endif /* BUTTON_H_ */