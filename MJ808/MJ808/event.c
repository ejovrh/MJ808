#include <avr/sfr_defs.h>
#include "event.h"

static volatile event_handler_t *__self;
static uint8_t __walker = 1;												//
static uint8_t __index = 0;											// used to index 1-8 events
static void _return(void)
{
	return;
};

static void _Notify(uint8_t in_val)
{

	__self->index |= _BV(in_val);
};

static void _HandleEvent(void)
{
	__walker = (__walker << 1) | (__walker >> 7 );
	(*__self->fpointer)(__self->index);
};

void event_handler_ctor(volatile event_handler_t * const self)
{
	__self = self;
	self->Notify = &_Notify;											// notifies about an event by setting the index to a predetermined value (uint8_t array-based lookup table)
	self->HandleEvent = &_HandleEvent;									// handles event based on index
	self->fpointer = &_return;											// default -- not initialized -- action: do nothing
};

volatile event_handler_t EventHandler __attribute__ ((section (".data")));		// define Task object and put it into .data