#ifndef TASK_H_
#define TASK_H_

#include <inttypes.h>

#define ISR_PCINT2 2

typedef struct event_handler_t
{
	void (*fpointer)(uint8_t state);
	void (*Add)(uint8_t val);
	void (*Notify)(const uint8_t in_val);
	void (*HandleEvent)(void);

	//void (*event_handler_ctor)(volatile struct event_handler_t *self, volatile void *device);
} event_handler_t;

//void virtual_event_handler_ctor(volatile struct event_handler_t *self, volatile void *device);// "virtual" pointer to array of button present on particular device

extern volatile event_handler_t EventHandler;							// declare task handler object

#endif /* TASK_H_ */