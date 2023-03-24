#include "main.h"

#include "event.h"
#include "stm32f0xx_hal.h"

#if defined(MJ828_)	// if this particular device is active
extern mj828_t *const Device;
#endif

#if defined(MJ808_)	// if this particular device is active
extern mj808_t *const Device;
#endif

typedef struct	// event_handler_t actual
{
	event_handler_t public;  // public struct
	uint8_t __walker;  // private - local variable used to walk over bit positions 1 to 8
	uint8_t __index;	// private - bit-wise flags for events (see _HandleEvent())
} __event_handler_t;

extern TIM_HandleTypeDef htim17;  // Timer17 object - event handling - 10ms
extern __event_handler_t __EventHandler;  // declare event_handler_t actual

/* theory of operation
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

// a function that does nothing
static inline void _DoNothing(const uint8_t foo)
{
	return;
}

// sets bit at bit_position ( 1 to 8) in byte __index
static void _UnSetEvent(const uint8_t val)
{
	__EventHandler.__index &= ~val;  // simply clears the bit at position bit_position

	if(__EventHandler.__index == 0)
		Device->StopTimer(&htim17);  // stop the timer
}

// sets bit at bit_position ( 1 to 8) in byte __index - _index will have values 0, 1, 2, 4, 8, 16...128
static void _Notify(const uint8_t bit_position)
{
	__EventHandler.__index |= bit_position;  // simply sets the bit at position bit_position
	Device->StartTimer(&htim17);  // start the timer
}

// calls __mjxxx_event_execution_function and passes on argument into it
static void _HandleEvent(void)
{
	__EventHandler.__walker = (__EventHandler.__walker << 1) | (__EventHandler.__walker >> 7);  // the __walker shifts a "1" cyclically from right to left
	(*__EventHandler.public.fpointer)(__EventHandler.__index & __EventHandler.__walker);	//	and ANDs it with __index, thereby passing the result as an argument to __mjxxx_event_execution_function

	if(__EventHandler.__index == 0)
		{
			HAL_TIM_Base_Stop_IT(&htim17);  // stop the timer
			__HAL_RCC_TIM17_CLK_DISABLE();  // stop the clock
		}
}

__event_handler_t __EventHandler =  // instantiate event_handler_t actual and set function pointers
	{  //
	.public.fpointer = &_DoNothing,  // default -- if not initialize: do nothing
	.public.Notify = &_Notify,	// notifies about an event by setting the index to a predetermined value (uint8_t array-based lookup table)
	.public.UnSetEvent = &_UnSetEvent,	// un-does what Nofity() does
	.public.HandleEvent = &_HandleEvent  // handles event based on index
	};

void event_handler_ctor()
{
	__EventHandler.__walker = 1;	// set the walker to 1 in order to start
}

event_handler_t *const EventHandler = &__EventHandler.public;  // set const pointer to EventHandler public part
