#include "main.h"

#include "event.h"
#include "stm32f0xx_hal.h"

#if defined(MJ808_)	// if this particular device is active
extern mj808_t *const Device;
#endif

#if defined(MJ828_)	// if this particular device is active
extern mj828_t *const Device;
#endif

#if defined(MJ838_)	// if this particular device is active
extern mj838_t *const Device;
#endif

// (very) poor man's log base 2, kinda...
uint8_t _GetMSBSetBit(uint16_t in_val)  // returns the 1-indexed position of a set most significant bit
{
	if(in_val >= 32768)
		return 16;

	else if(in_val >= 16384)
		return 15;

	else if(in_val >= 8192)
		return 14;

	else if(in_val >= 4096)
		return 13;

	else if(in_val >= 2048)
		return 12;

	else if(in_val >= 1024)
		return 11;

	else if(in_val >= 512)
		return 10;

	else if(in_val >= 256)
		return 9;

	else if(in_val >= 128)
		return 8;

	else if(in_val >= 64)
		return 7;

	else if(in_val >= 32)
		return 6;

	else if(in_val >= 16)
		return 5;

	else if(in_val >= 8)
		return 4;

	else if(in_val >= 4)
		return 3;

	else if(in_val >= 2)
		return 2;

	else if(in_val == 1)
		return 1;

	else
		return 0;
}

typedef struct	// event_handler_t actual
{
	event_handler_t public;  // public struct
	uint16_t __walker;  // private - local variable used to walk over bit positions 1 to 16
	uint16_t __index;  // private - bit-wise flags for events (see _HandleEvent())
	uint8_t __bitpos;  // bit position index
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

// sets bit at bit_position ( 1 to 16) in word __index
static void _UnSetEvent(const uint16_t bit_position)
{
	__EventHandler.__index &= ~_BV(bit_position - 1);  // simply clears the bit at position bit_position

	if(__EventHandler.__index == 0)
		Device->StopTimer(&htim17);  // stop the timer
}

// sets bit at bit_position ( 1 to 16) in byte __index - _index will have values 0, 1, 2, 4, 8, 16...32768
static void _Notify(const uint16_t bit_position)
{
	__EventHandler.__index |= _BV(bit_position);  // simply sets the bit at position bit_position
	Device->StartTimer(&htim17);  // start the timer
}

// calls __mjxxx_event_execution_function and passes on argument into it
static void _HandleEvent(void)
{
	__EventHandler.__walker = (__EventHandler.__walker << 1) | (__EventHandler.__walker >> 15);  // the __walker shifts a "1" cyclically from right to left

	if(__EventHandler.__index & __EventHandler.__walker)	// if walker and index have the same bit set ...
		{
			__EventHandler.__bitpos = _GetMSBSetBit(__EventHandler.__walker);  // save the current bit position
			(*__EventHandler.public.fpointer)(__EventHandler.__bitpos);  //	... get the position of the set bit (n) and execute function pointer at index n
			_UnSetEvent(__EventHandler.__bitpos);  // mark the event as handled
		}

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
//	.public.UnSetEvent = &_UnSetEvent,	// un-does what Nofity() does
	.public.HandleEvent = &_HandleEvent  // handles event based on index
	};

void event_handler_ctor()
{
	__EventHandler.__walker = 1;	// set the walker to 1 in order to start
}

event_handler_t *const EventHandler = &__EventHandler.public;  // set const pointer to EventHandler public part
