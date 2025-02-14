#ifndef CORE_INC_MJ838_PAC1952_PAC1952_H_
#define CORE_INC_MJ838_PAC1952_PAC1952_H_

#include "main.h"

#if USE_PAC1952	// if this particular device is active

typedef struct	// struct describing the PAC1952 functionality
{
	uint8_t (*ReadByte)(const uint8_t RegAddr);  // read one byte from register
	void (*WriteByte)(const uint8_t RegAddr, const uint8_t data);  // write one byte to register
	uint8_t (*ReceiveByte)(void);  // read byte from register pointer at location
	void (*SendByte)(const uint8_t byte);  // set internal address register pointer to location
	void (*BlockWrite)(const uint8_t RegAddr, uint8_t *buffer, const uint8_t len);  // block write
	void (*BlockRead)(const uint8_t RegAddr, uint8_t *buffer, const uint8_t len);  // block read
	void (*Refresh)(void);  // refresh
	void (*RefreshV)(void);  // refresh_v
	void (*PowerOn)(void);  // power device on & initialize
	void (*PowerOff)(void);  // power device off
} pac1952_t;

pac1952_t* pac1952_ctor(void);	// the PAC1952 constructor

#endif

#endif /* CORE_INC_MJ838_PAC1952_PAC1952_H_ */
