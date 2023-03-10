#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f0xx_hal.h"	// include HAL layer

#if !defined(_BV)
#define _BV(x) (1<<(x))
#endif

// TODO - investigate option bytes - RM0091 - 4.1.2 - in order to automatically determine a connected device

#define MJ808_	// what device to compile for

#define	USE_POWERSAVE	// use processor power save features (stop mode & sleep on exit)

#if defined(MJ808_)	// front light header include
#include "mj808/mj808.h"
#endif
#if defined(MJ818_)	// rear light header include
#include "mj818/mj818.h"
#endif
#if defined(MJ828_)	// user interface header include
#include "mj828/mj828.h"
#endif
#if defined(MJ838_)	// Čos header include
#include "mj838/mj838.h"
#endif
#if defined(MJ848_)	// logic unit header include
#include "mj848/mj848.h"
#endif

void Error_Handler(void);  //Exported functions prototypes

#endif /* __MAIN_H */
