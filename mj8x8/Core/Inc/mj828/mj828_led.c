#ifndef MJ828_LED_C_
#define MJ828_LED_C_

#include <inttypes.h>

#include "mj828\mj828.h"
#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

static primitive_led_t __primitive_led[8] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

static void __mj828_led_gpio_init(void)
{
// PRT - Charlie-plexed pin configuration
}

static void __LED_red(const uint8_t state)	// red LED on/off
{
	// PRT - 	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);								// pin b1 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// pin b2 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// pin b2 - cathode
		;
}

static void __LED_green(const uint8_t state)	// green LED on/off
{
	// PRT -	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);								// pin b2 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// pin b1 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// pin b1 - cathode
		;
}

static void __LED_blue1(const uint8_t state)	// blue1 LED on/off
{
	// PRT -	gpio_conf(LED_CP2_pin, OUTPUT, HIGH);								// pin b1 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// pin b0 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// pin b0 - cathode
		;
}

static void __LED_yellow(const uint8_t state)  // yellow LED on/off
{
	// PRT -	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);								// pin b2 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP2_pin, OUTPUT, LOW);							// pin b1 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP2_pin, OUTPUT, HIGH);							// pin b1 - cathode
		;
}

static void __LED_blue2(const uint8_t state)	// blue2 LED on/off
{
	// PRT -	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);								// pin d6 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP3_pin, OUTPUT, LOW);							// pin b0 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP3_pin, OUTPUT, HIGH);							// pin b0 - cathode
		;
}

static void __LED_blue3(const uint8_t state)	// blue3 LED on/off
{
	// PRT -	gpio_conf(LED_CP3_pin, OUTPUT, HIGH);								// pin b0 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// pin d6 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// pin d6 - cathode
		;
}

static void __LED_blue4(const uint8_t state)	// blue4 LED on/off
{
	// PRT -	gpio_conf(LED_CP1_pin, OUTPUT, HIGH);								// pin b2 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP4_pin, OUTPUT, LOW);							// pin d6 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP4_pin, OUTPUT, HIGH);							// pin d6 - cathode
		;
}

static void __LED_blue5(const uint8_t state)	// blue5 LED on/off
{
	// PRT -	gpio_conf(LED_CP4_pin, OUTPUT, HIGH);								// pin d6 - anode

	if(state)															// on
		// PRT - 		gpio_conf(LED_CP1_pin, OUTPUT, LOW);							// pin b2 - cathode
		;
	else
		// off
		// PRT - 		gpio_conf(LED_CP1_pin, OUTPUT, HIGH);							// pin b2 - cathode
		;
}

// private function, used only by the charlieplexing_handler() function
static void __glow(uint8_t led, uint8_t state)
{
	if(!state)	// if we get 0x00 (off argument) - do nothing and get out
		return;

//	static uint8_t (*fptr)(const uint8_t in_val);  // declare pointer for function pointers in branchtable_led[]
// PRT - progmem	static void (*const branchtable_led[])(const uint8_t in_val)
//	PROGMEM =		// array of function pointers for basic LED handling in PROGMEM
//		{
//			&__LED_red,														// index 0
//			&__LED_green,// index 1
//			&__LED_blue1,//	and so on...
//			&__LED_yellow,
//			&__LED_blue2,
//			&__LED_blue3,
//			&__LED_blue4,
//			&__LED_blue5
//		};

	__mj828_led_gpio_init();	// set LED pins to initial state

	// TODO - implement blinking
	//static uint8_t counter;
	//counter++;

	// PRT - 	fptr = pgm_read_ptr(&branchtable_led[led]);  // get appropriate function pointer from PROGMEM
	// PRT - 	fptr(state);														// execute with arguments given
}

static void _charlieplexing_handler()
{
	if(!__LED.flags)	// if there is any LED to glow at all
		return;

	static uint8_t i = 0;  // iterator to loop over all LEDs on device

// PRT -	__glow(i, (__LED.flags & _BV(i)));	// pass glow the LED number and the appropriate bit in the flag struct

	// !!!!
	(i == 7) ? i = 0 : ++i;  // count up to led_count and then start from zero
}

// toggles a bit in the LED flags variable; charlieplexer in turn makes it shine
static void _component_led_mj828(const uint8_t val)
{
	// PRT -	__LED.flags ^= _BV(val);											// just toggle
}

static __composite_led_t              __LED =
	{.public.led = __primitive_led, .public.Shine = &_component_led_mj828, .public.Handler = &_charlieplexing_handler, .flags = 0};

// implementation of virtual constructor for LEDs
static composite_led_t* _virtual_led_ctorMJ828()
{
	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif /* MJ828_LED_C_ */
