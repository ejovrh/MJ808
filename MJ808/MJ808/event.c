#include <avr/sfr_defs.h>
#include "event.h"

volatile event_handler_t *__self;

static void _Notify(uint8_t in_val)
{
	__self->index |= _BV(in_val);
};

static void _HandleEvent(void)
{
	(*__self->fpointer)(__self->index);
};

void event_handler_ctor(volatile event_handler_t * const self)
{
	__self = self;
	self->Notify = &_Notify;
	self->HandleEvent = &_HandleEvent;
};

volatile event_handler_t EventHandler __attribute__ ((section (".data")));		// define Task object and put it into .data