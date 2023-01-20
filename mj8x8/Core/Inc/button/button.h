#ifndef CORE_INC_BUTTON_BUTTON_H_
#define CORE_INC_BUTTON_BUTTON_H_

#include "event\event.h"

#define BUTTON_COUNT 3	// how many buttons are there
#define BUTTON_MIN_PRESS_TIME 3	// (numerical value x 25ms) = duration: 75ms
#define BUTTON_MAX_PRESS_TIME 63	// (numerical value x 25ms) = duration: 1.575s

enum button_casetable_index  // represents index of FooButtonCaseTable[] (uniquely defined in device button constructor)
{
	  CaseMomentary,	// array index 0 -- momentary press of pushbutton
	  CaseToggle,  // array index 1 -- momentary press of button toggles
	  CaseHold,  // array index 2 -- longer press of button toggles
	  CaseErrorHold,  // array index 3 -- lean against tree and thereby depress button unintentionally
	  CaseDoublePress,  // array index 4 - double press of a button
	  Casefoo,	// array index 5 - reserved for future use
	  Casebar,	// array index 6 - reserved for future use
	  Casebaz  // array index 7 - reserved for future use
};

typedef struct	// struct describing a generic pushbutton
{  //	flags can be consumed by the public
	volatile uint8_t Momentary :1;  // only while button being pressed hold pressed state, otherwise fall back to stable released state
	volatile uint8_t Toggle :1;  // button press & release, thereby move from one stable state into another stable state
	volatile uint8_t Hold :1;  // button press for a duration of BUTTON_MIN_PRESS_TIME (up to BUTTON_MAX_PRESS_TIME), thereby transition into a different stable state
	volatile uint8_t ErrorHold :1;  // button press of longer than BUTTON_MAX_PRESS_TIME transitions back to original stable state
	volatile uint8_t DoublePress :1;	// double button press, thereby transition from one stable state into another
	volatile uint8_t foo :1;	//	placeholder for future use
	volatile uint8_t bar :1;	//	placeholder for future use
	volatile uint8_t baz :1;	//	placeholder for future use
	void (*Mark)(const uint8_t state);	// marker for EXTI events (both edge transitions)
} individual_button_t;

typedef struct button_t
{
	void (*Handler)(void);  // button handler
	individual_button_t *button[BUTTON_COUNT];  // "virtual" pointer to array of buttons present on particular device

//button_t *(*virtual_button_ctor)(struct button_t * const self);	// "virtual" pointer to array of button present on particular device
} button_t;

#endif /* CORE_INC_BUTTON_BUTTON_H_ */
