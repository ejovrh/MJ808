#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f0xx_hal.h"	// include HAL layer

#if !defined(_BV)
#define _BV(x) (1<<(x))
#endif

#define MJ828_	// what device to compile for

#define USE_HEARTBEAT	1 // use the heartbeat
#define	USE_POWERSAVE	1 // use processor power save feature stop mode
#define USE_SLEEPONEXIT 1 // use processor power save feature sleep on ISR exit
#define USE_CAN_BUSOFF 1 // let CAN go into bus-off state or not

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
