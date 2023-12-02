#ifndef CORE_INC_EVENT_EVENT_H_
#define CORE_INC_EVENT_EVENT_H_

#include <inttypes.h>

#define DONOTHING 0	// human-readable event indices
#define EVENT00 0	// used in e.g. button binding mj828 pushbutton state to event via PushButtonCaseTable[]
#define EVENT01 1	// also used to convey e.g. mj828 autobatt states to events
#define EVENT02 2
#define EVENT03 3
#define EVENT04 4
#define EVENT05 5
#define EVENT06 6
#define EVENT07 7
#define EVENT08 8
#define EVENT09 9
#define EVENT10 10
#define EVENT11 11
#define EVENT12 12
#define EVENT13 13
#define EVENT14 14
#define EVENT15 15

typedef struct event_handler_t	// struct describing the event handler functionality
{
	void (*fpointer)(const uint8_t val);	// function pointer to handler function; default is to point to _return() in event.c
	void (*const Notify)(const uint16_t in_val);  // notifier - updates __self in event.c
//	void (*const UnSetEvent)(const uint16_t val);  // clears event identified by _BV(val) from execution
	void (*const HandleEvent)(void);	// handler - called from main(), executes either _return() or whatever a device points the function pointer to
} event_handler_t;

void event_handler_ctor();	// the event handler constructor

extern event_handler_t *const EventHandler;  // declare pointer to public struct part
#endif /* CORE_INC_EVENT_EVENT_H_ */
