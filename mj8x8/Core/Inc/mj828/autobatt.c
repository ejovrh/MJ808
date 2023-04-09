#include "main.h"

#if defined(MJ828_)	// if this particular device is active

#include "autobatt.h"
// TODO - implement functionality
typedef struct	// autobatt_t actual
{
	autobatt_t public;  // public struct
} __autobatt_t;

static __autobatt_t __AutoBatt __attribute__ ((section (".data")));  // preallocate __AutoBatt object in .data

//
static void _Do(void)
{
	;
}

static __autobatt_t __AutoBatt =  // instantiate autobatt_t actual and set function pointers
	{  //
	.public.Do = &_Do,  //	that one method that does its thing
	.public.AutoBattEnabled = 0,  //	set to off
	.public.AutoBattisOn = 0,  //
	};

autobatt_t* autobatt_ctor(void)  //
{
	return &__AutoBatt.public;  // set pointer to AutoBatt public part
}

#endif
