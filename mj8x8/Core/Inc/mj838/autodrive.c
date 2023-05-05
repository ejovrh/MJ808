#include "main.h"

#if defined(MJ838_)	// if this particular device is active

#include "autodrive.h"

typedef struct	// autobatt_t actual
{
	autodrive_t public;  // public struct
} __autodrive_t;

static __autodrive_t __AutoDrive __attribute__ ((section (".data")));  // preallocate __AutoDrive object in .data

// AutoDrive functionality based on detected speed
static void _Do(void)
{
	;
}

static __autodrive_t __AutoDrive =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do  // set function pointer
	};

autodrive_t* autodrive_ctor(void)  //
{
	return &__AutoDrive.public;  // set pointer to AutoBatt public part
}

#endif
