#ifndef CORE_INC_MJ514_GEAR_H_
#define CORE_INC_MJ514_GEAR_H_

#include "main.h"

#if defined(MJ514_)	// if this particular device is active

#define WIPE_FRAM 0	// if activated, statistics will be zeroed out in gear_ctor()

/*  Begriffserklärung:
 *
 * 1. rohloff gears are numbered from 1 to 14,
 * 	1 is the lowest, lightest - slower speed
 * 	14 is the highest, hardest - faster speed
 *
 * 	2. shifting is achieved by rotating the "17 tooth rohloff gear".
 * 	 it is located on the hub on some 8mm hex and can rotate in either CW or CCW direction:
 * 		CCW shifts from gear 1 towards 14 - into heavier gear
 * 			this is shifting up - direction_t ShiftUp
 * 		CW shifts from gear 14 towards 1 = into lighter gear
 * 			this is shifting down - direction_t ShiftDown
 *
 * 	3. CW and CCW are defined as as rotational directions when one looks onto the e14 unit as if it were mounted on the hub.
 * 		i.e. from the bike's left side, viewing axially onto the hub. when riding forward, the rear wheel would rotate CCW...
 * 		this definition applies to the gear_t object (gear.c and gear.h)
 *
 *
 *
 * 	the "17 tooth rohloff gear" is actuated by the "40-tooth e14 gear", which is barely visible on the e14 unit.
 *
 * 	shifting directions (ShiftUp and ShiftDown) in the hub are relevant only when pedalling the bike, otherwise it is not discernible.
 *
 * 	while debugging, the "40-tooth e14 gear" rotation is the only visible indicator of shifting direction.
 * 		therefore it is central to the shifting definition in terms of rotation:
 * 			"40-tooth e14 gear" CW rotation is "17 tooth rohloff gear" CCW rotation, therefore shifting up
 * 				the "23-tooth magnetic gear" rotates CCW; dir = 0
 * 			"40-tooth e14 gear" CCW rotation is "17 tooth rohloff gear" CW rotation, therefore shifting down
 * 				the "23-tooth magnetic gear" rotates CW; dir == 1
 *
 * 	in terms of data types, it is like this:
 * 		direction_t is defined on the gear_t level.
 * 			in gear.c it is used so that a human can understand calls like __Gear.Motor->Shift(ShiftUp, n);
 * 			in motor.c it is used so that a human can understand what "if (dir == ShiftUp)  then do something" means
 *
 * 		on the motor_t level and below, relevant becomes:
 * 			"23-tooth magnetic gear" rotation in terms of CW or CCW
 * 			this gear is an idle gear and it is driven by the "40-tooth e14 gear"; the rotations of these two gears are reversed.
 * 			the sole purpose of "23-tooth magnetic gear" is to be able to measure how far something has rotated.
 *
 * 			"40-tooth e14 gear" CW is "23-tooth magnetic gear" CCW and vice versa.
 *
 * 			"amount" of rotation translates to counting as5601_t rotary encoder signal impulses and readout of degrees
 * 			"23-tooth magnetic gear" rotation pulses (IRQ3 capture compare) are accumulated and saved.
 *
 *	in the end - for a shift of one gear in any direction - the question becomes:
 *		how may degrees does the "17 tooth rohloff gear" need to rotate in order to shift one gear?
 *	in other terms:
 *		how many degrees (or impulses) does the "23-tooth magnetic gear" need to rotate?
 *
 *	"17 tooth rohloff gear" per gear rotation can be discerned by referencing the hub-related PDF in the datasheets folder.
 *		- part number49 is what defines increments - 7 per 360 degree revolution.
 *			count the notches.
 *		this means that "17 tooth rohloff gear" needs to rotate 360/7 = 51.4285... degrees per shifted gear.
 *
 *	the e14 unit (i.e. the mj514) is a kind of rotational servo-motor.
 *	it has positional control (as5600), speed/direction control (PWM on Motor_IN1/2).
 *	it doesn't have force feedback, speed/acceleration control and most likely any form of PID is not needed.
 *
 *	if you can call a "while(some positional check) ;" a closed loop, then it even has a closed loop feedback thing going.
 *
 *	the e14 unit as a component is abstracted by gear_t.
 *		one can tell it to e.g. shift to gear 7 or shift up/down by 2 gears, with some sanity checks in place.
 *		it by itself doesn't know how to physically shift a gear other than running a motor.
 *
 *		gear_t is stateful in the sense that it utilises a non-volatile FeRAM to save data across power cycles.
 *		saved are:
 *			- the current gear, TODO the previous gear and TODO the future gear, if shifting is in operation
 *			- accumulated as5601 rotation pulses (1024 pulses per rotation)
 *			- accumulated gear shift count.
 *
 *
 *	the servo part is motor_t. it only does know revolution and direction.
 *		if one tells it to shift(3), it knows the direction and "distance", but it doesn't know the meaning.
 *		it doesn't know gears and doesn't keep track across power cycles.
 *
 *	motor_t utilises the as5601_t contactless rotary encoder to determine position.
 *
 *	time to shift one gear: Rohloff's original hardware: 180ms (their claim)
 */

typedef enum direction_t  // enum of direction of motor rotation
{  // note: the enum value (0 or 1) doesn't correlate to shifting direction
	  ShiftUp = 0,  // shift rohloff from gear 1 towards 14 - "40-tooth e14 gear" CW;
	  ShiftDown = 1  // shift rohloff from gear 14 towards 1 - "40-tooth e14 gear" CCW;
} direction_t;

typedef struct	// struct describing the Gear Shifter functionality
{
	uint8_t (*GetGear)(void);  // return current Rohloff gear (1 to 14)
	void (*ShiftToN)(const uint8_t n);  // shifts Rohloff into gear n (1 to 14)
	void (*ShiftByN)(const int8_t n);  // shifts the Rohloff hub n gears (-13 to + 13, except 0) up or down
} gear_t;

gear_t* gear_ctor(void);	// the Gear constructor

#endif

#endif /* CORE_INC_MJ514_GEAR_H_ */
