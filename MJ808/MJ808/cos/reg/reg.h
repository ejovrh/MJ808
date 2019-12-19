#ifndef REG_H_
#define REG_H_

#include <inttypes.h>

enum RegulatorOperatesAs												// enum of AC regulator operation: Graetz bridge, tuning capacitors on/off, Delon voltage doubler on/off
{
	Graetz,
	Tuning,
	Delon
};

typedef struct 															// reg_t struct describing the AC regulator as a whole
{
	uint8_t (* const GetRegulatorMode)(void);							// gets regulator operating mode
	void (* const SetRegulatorMode)(const uint8_t in_cap);				// sets regulator operating mode
} reg_t __attribute__((aligned(8)));

#endif /* REG_H_ */