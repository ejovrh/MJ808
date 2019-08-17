#include "task.h"

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

static (* const _branchtable_eventhandler[])(void) =
{
	&_zero,
	&_one,
	&_two,
	&_four,
	&_five
};

static void _Notify(const uint8_t in_val)
{
	__index |= _BV(in_val);
};

static void _Add(const uint8_t in_val)
{
	//__index |= _BV(in_val);
};

static void _HandleEvent(void)
{
	(*_branchtable_eventhandler[__index])();
};

void virtual_event_handler_ctor(volatile event_handler_t *self, volatile void *device)					//
{
	//_self = self;
	self->Add = &_Add;
	self->Notify = &_Notify;
	self->HandleEvent = &_HandleEvent;

	return self;
};

volatile event_handler_t EventHandler __attribute__ ((section (".data")));		// define Task object and put it into .data