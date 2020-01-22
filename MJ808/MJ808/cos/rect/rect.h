#ifndef RECT_H_
#define RECT_H_

#include <inttypes.h>

enum RegulatorOperatesAs												// enum of AC rectifier operation: Graetz bridge, tuning capacitors on/off, Delon voltage doubler on/off
{
																		// bit2:0 == 0 -- no tuning cap, i.e. naked graetz (w./wo. delon)
	_2200uF,															// bit 0 - 1st cap - 2200uF tuning cap on
	_3300uF,															// bit 1 - 2nd cap - ditto, 3300uF cap
	_4700uF,															// bit 2 - 3rd cap - ditto, 4700uF cap

																		// bit 4:3 - none || graetz || delon || combo
	_graetz,															// bit 3 - graetz swtich closed
	_delon																// bit 4 - delon switch closed
};

typedef struct 															// reg_t struct describing the AC rectifier as a whole
{
	uint8_t RectifierMode;												// rectifier operating mode data member
	void (* SetRectifierMode)(const uint8_t in_mode);					// sets rectifier operating mode
} rect_t __attribute__((aligned(8)));

extern rect_t * const RECT;												// declare pointer to public struct part

#endif /* RECT_H_ */