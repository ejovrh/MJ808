#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#include "mb85rc.h"

typedef struct	// mb85rc_t actual
{
	mb85rc_t public;  // public struct
} __mb85rc_t;

static __mb85rc_t __MB85RC __attribute__ ((section (".data")));  // preallocate __MB85RC object in .data

static uint8_t _Read(const uint8_t addr)
{
	return 0;
}

static void _Write(const uint8_t addr, const uint8_t data)
{

}

static __mb85rc_t __MB85RC =  // instantiate mb85rc_t actual and set function pointers
	{  //
	.public.Read = &_Read,  // set function pointer
	.public.Write = &_Write  // set function pointer
	};

mb85rc_t* mb85rc_ctor(void)  //
{
	return &__MB85RC.public;  // set pointer to MB85RC public part
}

#endif
