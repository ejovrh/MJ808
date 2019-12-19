#ifndef MCP73871_H_
#define MCP73871_H_

#include <inttypes.h>

typedef struct															// struct describing the LiIon Charger&Powerpath controller object
{
	void (* const SetResistor)(const uint8_t in_val);					// sets charge current control resistor, see :
	/*
		p.23, 4.5 Constant Current Mode - Fast Charge, 5.1.2
		Ireg = 1000v/Rprog1
		500mA = 1000V/2000 Ohm
	*/
	uint8_t (* const GetStatus)(void);									// gets the device status (PG, STAT1, STAT2)
} mcp73871_t __attribute__((aligned(8)));

#endif /* MCP73871_H_ */