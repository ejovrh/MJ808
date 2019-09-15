#include <inttypes.h>

#include "led.h"

typedef struct															// composite_led_t actual
{
	composite_led_t public;												// public struct

	uint8_t flags;														// private - uint8_t interpreted bit-wise for flagging individual LEDs to be lit
} __composite_led_t;

static __composite_led_t __LED;											// instantiate object