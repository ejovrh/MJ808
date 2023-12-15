#ifndef MJ838_MOTION_C_
#define MJ838_MOTION_C_

#if defined(MJ838_)	// if this particular device is active

#include "motion/motion_actual.c"

static __motion_t __Motion;  // forward declaration of object

// starts the peripheral
static inline void _StartMotion(void)
{
	;
}

// stops the peripheral
static inline void _StopMotion(void)
{
	;
}

motion_t* motion_ctor(void)
{
	__Motion.public.Start = &_StartMotion;  // ditto
	__Motion.public.Stop = &_StopMotion;  // ditto

	return &__Motion.public;  // return public parts
}

#endif // MJ838_

#endif /* MJ838_MOTION_C_ */
