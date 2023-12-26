#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "as5601.h"

typedef struct	// as5601c_t actual
{
	as5601_t public;  // public struct
} __as5601_t;

static __as5601_t __AS5601 __attribute__ ((section (".data")));  // preallocate __AS5601 object in .data

static uint8_t _CountRotation(void)
{
	return 0;
}

static __as5601_t __AS5601 =  // instantiate as5601c_t actual and set function pointers
	{  //
	.public.CountRotation = &_CountRotation,  // set function pointer
	};

as5601_t* as5601_ctor(void)  //
{
	return &__AS5601.public;  // set pointer to AS5601 public part
}

#endif
