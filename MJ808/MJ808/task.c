#include "task.h"

//static task_handler_t *_self;
static uint8_t __index = 0;

void _zero(void)
{
	
};

void _one(void)
{

};

void _two(void)
{

};

void _three(void)
{

}

void _four(void)
{

}

void _five(void)
{

}

static (* const _task_pointers[])(void) =
{
	&_zero,
	&_one,
	&_two,
	&_four,
	&_five
};

static void _foo_add(const uint8_t in_val)
{
	__index = in_val;
};

static void _foo_run(void)
{
	(*_task_pointers[__index])();
	__index = 0;
};

volatile task_handler_t *task_handler_ctor(volatile task_handler_t *self)					//
{
	//_self = self;

	self->Add = &_foo_add;
	self->Run = &_foo_run;

	return self;
};

volatile task_handler_t Task __attribute__ ((section (".data")));		// define Task object and put it into .data