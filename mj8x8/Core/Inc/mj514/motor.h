#ifndef CORE_INC_MJ514_MOTOR_H_
#define CORE_INC_MJ514_MOTOR_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

typedef enum direction_t  // enum of direction of motor rotation
{
	  Up,
	  Down
} direction_t;

typedef struct	// struct describing the Motor functionality
{
	uint8_t (*Rotate)(const direction_t dir, const uint8_t n);  //
} motor_t;

motor_t* motor_ctor(void);	// the Motor constructor

#endif

#endif /* CORE_INC_MJ514_MOTOR_H_ */
