#ifndef CORE_INC_BUTTON_BUTTON_TYPES_ACTUAL_C_
#define CORE_INC_BUTTON_BUTTON_TYPES_ACTUAL_C_

#include "main.h"

#if defined(USE_BUTTON)
#include "button.h"

typedef struct	// individual_button_t actual
{
	individual_button_t public;  // public struct

	uint16_t __state :1;  // counter used for button press timing

	uint8_t *__ButtonCaseptr;  // pointer to array in which button events are bound to switch-case statements in __mjxxx_button_execution_function
	uint16_t __hold_counter :15;  // counter to count button press duration for hold_X states
} __individual_button_t;

typedef struct	// button_t actual
{
	button_t public;	// public struct
	__individual_button_t *__button;	//
} __button_t;

// preallocate __Button object in .data
static __button_t __Button __attribute__ ((section (".data")));  // @suppress("Unused variable declaration in file scope")

#endif
#endif /* CORE_INC_BUTTON_BUTTON_TYPES_ACTUAL_C_ */
