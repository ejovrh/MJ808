#include "button.h"

typedef struct															// __indovodial_button_t actual
{
	individual_button_t public;											// public struct
	

} __individual_button_t;

typedef struct															// button_t actual
{
	button_t public;													// public struct
	uint8_t __button_count : 2;											// private - button count, max. 4 buttons
} __button_t;

static __button_t __Button __attribute__ ((section (".data")));		// define BUTTON object and put it into .data
