#ifndef MJ808_COMPOSITE_LED_ACTUAL_C_
#define MJ808_COMPOSITE_LED_ACTUAL_C_

#if defined(USE_LED)
#include <inttypes.h>

#include "led.h"

typedef struct	// composite_led_t actual
{
	composite_led_t public;  // public struct

	uint8_t _ShineFlags;	// private - uint8_t interpreted bit-wise for flagging individual LEDs to be lit
	uint8_t _BlinkFlags;	// private - uint8_t interpreted bit-wise for flagging individual LEDs to blink
	uint8_t _OldBlinkFlags;  // private - uint8_t interpreted bit-wise for storing _BlinkFlags before state changes
	uint8_t _BlinkCounter;	// private - counter for blinking in LED handler
} __composite_led_t;

static __composite_led_t __LED;	// instantiate object

#endif
#endif /* MJ808_COMPOSITE_LED_ACTUAL_C_ */
