#ifndef TASK_H_
#define TASK_H_

#include <inttypes.h>

#define ISR_PCINT2 2

typedef struct event_handler_t
{
	uint8_t index;
	//uint8_t event_table[8];

	void (*fpointer)(const uint8_t val);										// function pointer to handler function; default is to point to _return() in event.c
	void (*Notify)(const uint8_t in_val);								// notifier - updates __self in event.c
	void (*HandleEvent)(void);											// handler - called from main(), executes either _return() or whatever a device points the function pointer to
} event_handler_t;

void event_handler_ctor(volatile event_handler_t * const self);

extern volatile event_handler_t EventHandler;							// declare task handler object

#endif /* TASK_H_ */