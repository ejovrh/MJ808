#ifndef CORE_INC_MJ514_GEAR_H_
#define CORE_INC_MJ514_GEAR_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

/*  Begriffserklärung:
 *
 * rohloff gears are numbered from 1 to 14,
 * 	1 is the lowest, lightest - slower speed
 * 	14 is the highest, hardest - faster speed
 *
 * 	shifting is achieved by rotating the "17 tooth rohloff gear" (which sits on the hub on some 8mm hex) in either CW or CCW direction:
 * 		CCW shifts from gear 1 towards 14 - into heavier gear
 * 			this is shifting up - direction_t ShiftUp
 * 		CW shifts from gear 14 towards 1 = into lighter gear
 * 			this is shifting down - direction_t ShiftDown
 *
 * 	the "17 tooth rohloff gear" is actuated by a the "40-tooth e14 gear", which is barely visible on the e14 unit.
 *
 * 	shifting directions (ShiftUp and ShiftDown) are relevant only when pedalling the bike, otherwise it is not discernible.
 *
 * 	while debugging, the "40-tooth e14 gear" rotation is the only visible indicator of shifting direction.
 * 		therefore it is central to the shifting definition in terms of rotation:
 * 			"40-tooth e14 gear" CW rotation is "17 tooth rohloff gear" CCW rotation, therefore shifting up
 * 			"40-tooth e14 gear" CCW rotation is "17 tooth rohloff gear" CW rotation, therefore shifting down
 *
 * 	in terms of data types, it is like this:
 * 		direction_t is defined on the gear_t level.
 * 			in gear.c it is used so that a human can understand calls like __Gear.Motor->Shift(ShiftUp, n);
 * 			in motor.c it is used so that a human can understand what "if (dir == ShiftUp)  then do something" means
 *
 * 		on the motor_t level and below, relevant becomes:
 * 			"23-tooth magnetic gear" rotation in terms of CW or CCW
 * 			this gear is an idle gear and it is driven by the "40-tooth e14 gear".
 * 			the sole purpose of "23-tooth magnetic gear" is to be able to measure how far something has rotated.
 *
 * 			CW or CCW translates to motor controller IN1 or IN2 being high or low
 * 			"amount" of rotation translates to counting as5601_t rotary encoder signal impulses and readout of degrees
 *
 *	in the end -for a shift of one gear in any direction- the question becomes:
 *		how may degrees does the "17 tooth rohloff gear" need to rotate?
 *	in other terms:
 *		how many degrees (or impulses) does the "23-tooth magnetic gear" need to rotate?
 *
 *	"17 tooth rohloff gear" per gear rotation can be discerned by referencing the hub-related PDF in the datasheets folder.
 *		- part number49 is what defines increments - 7 per 360 degree revolution
 *			count the notches.
 *		this means that "17 tooth rohloff gear" needs to rotate 360/7 = 51.4285... degrees per shifted gear.
 *
 *
 *
 *
 */

typedef enum direction_t  // enum of direction of motor rotation
{  // note: the enum value (0 or 1) doesn't correlate to shifting direction
	  ShiftDown,	// shift rohloff from gear 14 towards 1
	  ShiftUp  // shift rohloff from gear 1 towards 14
} direction_t;

typedef struct	// struct describing the Gear Shifter functionality
{
	uint8_t (*GetGear)(void);  // return current Rohloff gear
	void (*ShiftToN)(const uint8_t n);  // shifts Rohloff into gear n (1 to 14)
	void (*ShiftByN)(const int8_t n);  // shifts Rohloff by n (-13 to + 13, except 0)
} gear_t;

gear_t* gear_ctor(void);	// the Gear constructor

#endif

#endif /* CORE_INC_MJ514_GEAR_H_ */
