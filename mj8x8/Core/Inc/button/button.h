#ifndef CORE_INC_BUTTON_BUTTON_H_
#define CORE_INC_BUTTON_BUTTON_H_

#include <inttypes.h>

#include "event\event.h"

#define BUTTON_MIN_PRESS_TIME 20	// number times 25ms duration: 500ms
#define BUTTON_MAX_PRESS_TIME 120	// number times 25ms duration: 3s

enum button_casetable_index  // represents index of FooButtonCaseTable[] (uniquely defined in device button constructor)
{
	  CaseMomentary,	// array index 0 -- momentary press of pushbutton
	  CaseToggle,  // array index 1 -- momentary press of button toggles
	  CaseHold,  // array index 2 -- longer press of button toggles
	  CaseErrorHold  // array index 3 -- lean against tree and thereby depress button unintentionally
};

typedef struct	// struct describing a generic pushbutton
{  //	flags can be consumed by the public
	volatile uint8_t Momentary :1;  // flag indicating if button is pressed right now
	volatile uint8_t Toggle :1;  // flag indicating the toggle state
	volatile uint8_t Hold :1;  // flag indicating a button press for a duration of BUTTON_MIN_PRESS_TIME (up to BUTTON_MAX_PRESS_TIME) seconds, followed by button release
	volatile uint8_t ErrorHold :1;  // flag indicating constant button press (by error, object leaning on pushbutton, etc.)
} individual_button_t;

typedef struct button_t
{
	void (*Handler)(void);  // button handler
	individual_button_t *button;	// "virtual" pointer to array of buttons present on particular device

//button_t *(*virtual_button_ctor)(struct button_t * const self);	// "virtual" pointer to array of button present on particular device
} button_t;

#endif /* CORE_INC_BUTTON_BUTTON_H_ */
