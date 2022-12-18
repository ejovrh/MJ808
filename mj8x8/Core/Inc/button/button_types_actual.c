#ifndef MJ808_BUTTON_TYPES_ACTUAL_C_
#define MJ808_BUTTON_TYPES_ACTUAL_C_

#include "button.h"

typedef struct															// individual_button_t actual
{
	individual_button_t public;											// public struct

	uint8_t __state :2;				// something akin to a "counter" used for debouncing
	uint8_t __was_pressed :1;	// flag indicating if button was released after a stable state (used to remember previous state)
	uint8_t __is_at_default :1;									// 1 - default values, 0 otherwise

	uint8_t *__ButtonCaseptr;	// pointer to array in which button events are bound to switch-case statements in __mjxxx_button_execution_function
	volatile uint8_t *__PIN;								// PIN register address of button pin
	uint8_t __pin_number;	// pin number (0, 1...6) to which the button is connected
	uint8_t __hold_counter;	// counter to count button press duration for hold_X states
} __individual_button_t;

typedef struct															// button_t actual
{
	button_t public;													// public struct
	__individual_button_t *__button;									//
	uint8_t __button_count :2;					// private - button count, max. 4 buttons
} __button_t;

static __button_t  __Button  __attribute__ ((section (".data")));	// preallocate __Button object in .data

#endif /* MJ808_BUTTON_TYPES_ACTUAL_C_ */
