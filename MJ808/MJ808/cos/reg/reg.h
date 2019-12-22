#ifndef REG_H_
#define REG_H_

#include <inttypes.h>

enum RegulatorOperatesAs												// enum of AC regulator operation: Graetz bridge, tuning capacitors on/off, Delon voltage doubler on/off
{
	Graetz,																// 0
	Tuning,																// 1
	Delon																// 2
};

typedef struct 															// reg_t struct describing the AC regulator as a whole
{
	uint8_t RegulatorMode;												// regulator operating mode data member
	void (* SetRegulatorMode)(const uint8_t in_mode);					// sets regulator operating mode
} reg_t __attribute__((aligned(8)));

reg_t *reg_ctor();														// initializes reg_t object

#endif /* REG_H_ */