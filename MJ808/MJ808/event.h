#ifndef TASK_H_
#define TASK_H_

#include <inttypes.h>

typedef struct event_handler_t											// struct describing the event handler functionality
{
	void (*fpointer)(const uint8_t val);								// function pointer to handler function; default is to point to _return() in event.c
	void (*Notify)(const uint8_t in_val);								// notifier - updates __self in event.c
	void (*UnSetEvent)(const uint8_t val);								// clears event identified by _BV(val) from execution
	void (*HandleEvent)(void);											// handler - called from main(), executes either _return() or whatever a device points the function pointer to
} event_handler_t;

void event_handler_ctor(volatile event_handler_t * const self);			// the event handler constructor

extern volatile event_handler_t EventHandler;							// declare task handler object

#endif /* TASK_H_ */