#ifndef TASK_H_
#define TASK_H_

#include <inttypes.h>

#define ISR_PCINT2 2

typedef struct task_handler_t
{
	void (*Add)(const uint8_t in_val);
	void (*Run)(void);
} task_handler_t;

volatile task_handler_t *task_handler_ctor(volatile struct task_handler_t *self);// "virtual" pointer to array of button present on particular device

extern volatile task_handler_t Task;									// declare task handler object

#endif /* TASK_H_ */