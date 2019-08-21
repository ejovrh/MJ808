#include <avr/sfr_defs.h>
#include "event.h"

static volatile event_handler_t *__self;								// ... a true private data member of this "class"
static uint8_t __walker = 1;											// local variable used to walk over bit positions 1 to 8

// TODO - make global because similar functionality is needed elsewhere, too

static void _return(void)												// do-noting function
{
	return;
};

// sets bit at bit_position ( 1 to 8) in byte EventHandler.index
static void _Notify(const uint8_t bit_position)
{

	__self->index |= bit_position;										// simply sets the bit at position bit_position
};

static void _HandleEvent(void)
{
	__walker = (__walker << 1) | (__walker >> 7 );
	(*__self->fpointer)(__self->index & __walker);
};

void event_handler_ctor(volatile event_handler_t * const self)
{
	__self = self;
	//static uint8_t event_table[8];

	self->Notify = &_Notify;											// notifies about an event by setting the index to a predetermined value (uint8_t array-based lookup table)
	self->HandleEvent = &_HandleEvent;									// handles event based on index
	self->fpointer = &_return;											// default -- not initialized -- action: do nothing
};

volatile event_handler_t EventHandler __attribute__ ((section (".data")));		// define Task object and put it into .data