#ifndef CORE_INC_TRY_TRY_H_
#define CORE_INC_TRY_TRY_H_

#include "mj8x8/can_msg.h"

typedef struct 	// struct describing devices on MJ808
{
	void (*PopulatedBusOperation)(message_handler_t *const in_handler);  // executes function pointer identified by message command
	void (*EventHandler)(const uint8_t val);  // executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
	void (*EmptyBusOperation)(void);  // defines device operation on empty bus
} try_t;

//try_t* try_ctor(void);

extern try_t *const Try;  // declare pointer to public struct part

#endif /* CORE_INC_TRY_TRY_H_ */
