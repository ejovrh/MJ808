#ifndef CORE_INC_MJ514_MOTOR_H_
#define CORE_INC_MJ514_MOTOR_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

/*  more Begriffserklärung:
 *
 * 	in order to see how it works, it is best to open a e14 unit and see for oneself.
 *
 * 	in rough terms:
 *		the motor has a worm gear on its rotor.
 *		this worm gear rotates an idler gear which in turn rotates the "40-tooth e14 gear".
 *			the "40-tooth e14 gear" actuates the "17 tooth rohloff gear", which then does the actual shifting in the hub.
 *		the "40-tooth e14 gear" also drives the "23-tooth magnetic gear".
 *
 *	the "23-tooth magnetic gear" has a built-in magnet which rotation is detected by the as5601 encoder.
 *	the as5601 sits axially directly under this magnetic gear, on the inside of the e14 unit, on the PCB underside.
 *
 *
 *	gear ratios of the worm, idler and even "40-tooth e14 gear" gears are irrelevant.
 *	relevant ratios are "17 tooth rohloff gear" to "23-tooth magnetic gear".
 *	this is 17/23 = 0.7391...
 *		for every full rotation of the 17-tooth gear, the 23-tooth gear makes 0.7391... revolutions
 *
 *		51.4285... degrees of rotation of the 17-tooth gear are:
 *			 (360/7) * (17/23) = 38.0124... degrees of rotation of the 23-tooth gear.
 *
 *		these 38.0124... degrees are what needs to be measured.
 *
 *		in as5601 terms, 38.0124... degrees are:
 *			- 38.0124... / 360 = 0.1055... rotations of 360 degrees (in decimal)
 *			- 1024 * 0.1055...  = 108.1242 impulses, with an ABM setting of 0x08
 *			- 4096  * 0.1055...  = 432.4968 raw degrees, as read from as5601 ANGLE register
 *
 */

#define RAWANGLE_PER_GEAR 432	// 432.4968 raw degrees, as read from as5601 ANGLE register
#define PULSE_PER_GEAR 108 // 108.1242 impulses, with an ABM setting of 0x08

typedef struct	// struct describing the Motor functionality
{
	void (*Shift)(const direction_t dir, const uint8_t n);  // rotate a gear to shift up/down
} motor_t;

motor_t* motor_ctor(void);	// the Motor constructor

#endif

#endif /* CORE_INC_MJ514_MOTOR_H_ */
