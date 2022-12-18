#ifndef CORE_INC_EVENT_EVENT_H_
#define CORE_INC_EVENT_EVENT_H_

#include <inttypes.h>

typedef struct event_handler_t	// struct describing the event handler functionality
{
	void (*fpointer)(const uint8_t val);	// function pointer to handler function; default is to point to _return() in event.c
	void (*const Notify)(const uint8_t in_val);  // notifier - updates __self in event.c
	void (*const UnSetEvent)(const uint8_t val);	// clears event identified by _BV(val) from execution
	void (*const HandleEvent)(void);	// handler - called from main(), executes either _return() or whatever a device points the function pointer to
} event_handler_t;

void event_handler_ctor();	// the event handler constructor

extern event_handler_t *const EventHandler;  // declare pointer to public struct part
#endif /* CORE_INC_EVENT_EVENT_H_ */
