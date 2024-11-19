#ifndef CORE_INC_MJ514_MOTOR_H_
#define CORE_INC_MJ514_MOTOR_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

typedef enum direction_t  // enum of direction of motor rotation
{  // note: the enum value (0 or 1) doesn't correlate to shifting direction
	  ShiftDown,	// shift rohloff from gear 14 towards 1
	  ShiftUp  // shift rohloff from gear 1 towards 14
} direction_t;

typedef struct	// struct describing the Motor functionality
{
	uint8_t (*Rotate)(const direction_t dir, const uint8_t n);  // rotate a cog to shift up/down
} motor_t;

motor_t* motor_ctor(void);	// the Motor constructor

#endif

#endif /* CORE_INC_MJ514_MOTOR_H_ */
