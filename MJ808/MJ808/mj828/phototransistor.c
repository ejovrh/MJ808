#include <inttypes.h>

#include "phototransistor.h"

// TODO - implement the whole of ph.trans. handling

typedef struct															// phototransistor_t actual
{
	phototransistor_t public;											// public struct

	uint8_t __active:1;													// flag - is the state of the phototransistor to be interpreted or not
} __phototransistor_t;

extern __phototransistor_t __Phototransistor;							// declare phototransistor_t actual

uint8_t _GetPinState(void)
{
	// TODO - implement read function
	return 0;
};

__phototransistor_t __Phototransistor =									// instantiate event_handler_t actual and set function pointers
{
	.public.GetPinState = &_GetPinState,								// returns pin state: high - daylight, low - darkness
	.__active = 0
} ;

phototransistor_t * const Phototransistor = &__Phototransistor.public ;	// set const pointer to EventHandler public part