#include <avr/sfr_defs.h>
#include "event.h"

static volatile event_handler_t *__self;								// ... a true private data member of this "class"
static uint8_t __walker = 1;											// local variable used to walk over bit positions 1 to 8

/* theory of operation
 *
 *	components: subject, event handler, object
 *		the subject uses Notify() to notify the event handler about an event - e.g. a button press
 *		via Notify() the event handler sets a bit in __index
 *
 *		on every iteration of the loop in main() the event handler function HandleEvent() is called
 *			upon every call __walker iterates over __index by shifting "1" left and ANDing both variables
 *			once the position of 1 in both variables match, HandleEvent() executes an event handling function and passes __index into it
 *
 *		the event handling function determines via switch-case what to do to an object (e.g. light a LED)
 *
 *		each subject has its event ( e.g. button hold, toggle, etc.) uniquely identified via an enum
 *		the value of said enum corresponds to a case table array in which the enum value is the array index
 *		the value at the array index gets passed into the event handling function as an argument
 *		the argument gets evaluated via a switch-case
 *
 *		values for switch-case must be unique
 *		each subject ought to have its own case table
 *		all values in all case table arrays must be unique
 */

// TODO - make global because similar functionality is needed elsewhere, too
static void _return(void)												// do-noting function
{
	return;
};

// sets bit at bit_position ( 1 to 8) in byte EventHandler.index
static void _Notify(const uint8_t bit_position)
{

	__self->index |= bit_position;										// simply sets the bit at position bit_position
};

// calls __mjxxx_event_execution_function and passes on argument into it
static void _HandleEvent(void)
{
	__walker = (__walker << 1) | (__walker >> 7 );						// the __walker shifts a "1" cyclically from right to left
	(*__self->fpointer)(__self->index & __walker);						//	and ANDs it with __index, thereby passing the result as an argument to __mjxxx_event_execution_function
};

void event_handler_ctor(volatile event_handler_t * const self)
{
	__self = self;														// private - pointer to self, utilized in functions above

	self->Notify = &_Notify;											// notifies about an event by setting the index to a predetermined value (uint8_t array-based lookup table)
	self->HandleEvent = &_HandleEvent;									// handles event based on index
	self->fpointer = &_return;											// default -- if not initialized: do nothing
};

volatile event_handler_t EventHandler __attribute__ ((section (".data")));		// define Task object and put it into .data