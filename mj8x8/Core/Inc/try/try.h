#ifndef CORE_INC_TRY_TRY_H_
#define CORE_INC_TRY_TRY_H_

#include "mj808\mj808.h"	// the particular activity_t is public and is used below
#include "mj818\mj818.h"	// ditto
#include "mj828\mj828.h"
#include "mj838\mj838.h"

typedef struct	// struct describing bus-wide device activity
{
	activity_t *_0;  // 0A - ?
	activity_t *_1;  // 0B - ?
	activity_t *_2;  // 0C - ?
	mj828_activity_t *mj828;  // 0D - dashboard
	mj838_activity_t *mj838;  // 1A - Čos dynamo/generator
	activity_t *_5;  // 1B - ?
	activity_t *_6;  // 1C - ?
	activity_t *_7;  // 1D - ?
	mj808_activity_t *mj808;  // 2A - front light
	mj818_activity_t *mj818;  // 2B - rear light
	activity_t *_10;  // 2C - ?
	activity_t *_11;  // 2D - ?
	activity_t *_12;  // 3A - ?
	activity_t *_13;  // 3B - ?
	activity_t *_14;  // 3C - ?
	activity_t *_15;  // 3D - ?
} status_t;

typedef struct 	// struct describing devices on MJ808
{
	status_t *BusActivity;  // pointer to bus-wide activity container for all devices
	void (*PopulatedBusOperation)(message_handler_t *const in_handler);  // executes function pointer identified by message command
	void (*EventHandler)(const uint8_t val);  // executes code depending on argument (which is looked up in lookup tables such as FooButtonCaseTable[]
	void (*EmptyBusOperation)(void);  // defines device operation on empty bus
} try_t;

void try_ctor(void);	// the try-object constructor

extern try_t *const Try;  // declare pointer to public struct part

#endif /* CORE_INC_TRY_TRY_H_ */
