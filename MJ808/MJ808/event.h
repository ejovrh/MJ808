#ifndef TASK_H_
#define TASK_H_

#include <inttypes.h>

#define ISR_PCINT2 2

typedef struct event_handler_t
{
	uint8_t index;

	void (*fpointer)(uint8_t val);
	void (*Notify)(uint8_t in_val);
	void (*HandleEvent)(void);
} event_handler_t;

void event_handler_ctor(volatile event_handler_t *self);

extern volatile event_handler_t EventHandler;							// declare task handler object

#endif /* TASK_H_ */