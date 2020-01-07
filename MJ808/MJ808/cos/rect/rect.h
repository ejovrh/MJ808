#ifndef RECT_H_
#define RECT_H_

#include <inttypes.h>

enum RegulatorOperatesAs												// enum of AC rectifier operation: Graetz bridge, tuning capacitors on/off, Delon voltage doubler on/off
{
	Graetz,																// 0
	Tuning,																// 1
	Delon																// 2
};

typedef struct 															// reg_t struct describing the AC rectifier as a whole
{
	uint8_t RectifierMode;												// rectifier operating mode data member
	void (* SetRectifierMode)(const uint8_t in_mode);					// sets rectifier operating mode
} rect_t __attribute__((aligned(8)));

extern rect_t * const RECT;												// declare pointer to public struct part

#endif /* RECT_H_ */