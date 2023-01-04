#ifndef MJ828_LED_C_
#define MJ828_LED_C_

#include "main.h"
#if defined(MJ828_)	// if this particular device is active

#include <inttypes.h>
#include "mj828\mj828.h"
#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

static primitive_led_t __primitive_led[8] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

// TODO - improve GPIO initialisations below as they do work, yet are ugly
static void __mj828_led_gpio_init(void)
{
	static GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP1_Pin | CP2_Pin | CP3_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_RESET);
}

static void __LED_red(const uint8_t state)	// red LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP2_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP2_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP1_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// anode high

	if(state)
		// on
		HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_RESET);	// cathode low
	else
		// off
		HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// cathode high

	HAL_GPIO_Init(CP1_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_green(const uint8_t state)	// green LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP1_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP2_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// anode high

	if(state)
		// on
		HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_RESET);	// cathode low
	else
		// off
		HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// cathode high

	HAL_GPIO_Init(CP2_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_blue(const uint8_t state)  // blue1 LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP2_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP2_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP4_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// CP2 - anode

	if(state)
		// on
		HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_RESET);	// CP4 - cathode
	else
		// off
		HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// CP4 - cathode

	HAL_GPIO_Init(CP4_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_yellow(const uint8_t state)  // yellow LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP3_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP3_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP2_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// CP3 - anode

	if(state)
		// on
		HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_RESET);	// CP2 - cathode
	else
		// off
		HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// CP2 - cathode

	HAL_GPIO_Init(CP2_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_batt1(const uint8_t state)	// blue2 LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP4_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP4_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP3_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// CP4 - anode

	if(state)
		// on
		HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_RESET);	// CP3 - cathode
	else
		// off
		HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// CP3 - cathode

	HAL_GPIO_Init(CP3_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_batt2(const uint8_t state)	// blue3 LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP3_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP3_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP4_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// CP3 - anode

	if(state)
		// on
		HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_RESET);	// CP4 - cathode
	else
		// off
		HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// CP4 - cathode

	HAL_GPIO_Init(CP4_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_batt3(const uint8_t state)	// blue4 LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP1_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP1_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP4_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// CP1 - anode

	if(state)
		// on
		HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_RESET);	// CP4 - cathode
	else
		// off
		HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// CP4 - cathode

	HAL_GPIO_Init(CP4_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

static void __LED_batt4(const uint8_t state)	// blue5 LED on/off
{
	GPIO_InitTypeDef GPIO_InitStruct =
		{0};

	GPIO_InitStruct.Pin = CP4_Pin;	// anode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(CP4_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CP1_Pin;	// cathode
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// CP4 - anode

	if(state)
		// on
		HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_RESET);	// CP1 - cathode
	else
		// off
		HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// CP1 - cathode

	HAL_GPIO_Init(CP1_GPIO_Port, &GPIO_InitStruct);  // cathode commit
}

// private function, used only by the charlieplexing_handler() function
static void __glow(uint8_t led, uint8_t state)
{
	__mj828_led_gpio_init();	// set LED pins to initial state

	__LED.public.led[led].Shine(state);
}

// handles the time-based charlieplexing stuff
static void _charlieplexing_handler()
{
	/* charlieplexing handler rationale
	 * with charlieplexing, the rationale is that only a single LED can be lit at any time so that  GPIO current limits are not exhausted
	 * if e.g. two or more LEDs have to be lit, they must be lit one after the other in rapid succession;
	 * if this is done fast enough (below 20ms), the human eye will perceive both as lit at the same time
	 *
	 * the handler is to be called from a timer ISR periodically
	 */

	if(!__LED.flags)	// if there is any LED to glow at all
		return;

	static uint8_t i = 0;  // persistent iterator across function calls loops over all LEDs on device

	__glow(i, (__LED.flags & _BV(i)));	// pass glow the LED number and the appropriate bit in the flag struct

	// !!!!
	(i >= 7) ? i = 0 : ++i;  // count up to 7 and then restart from zero (we have 8 LEDs)
}

// toggles a bit in the LED flags variable; charlieplexer in turn makes it shine
static void _component_led_mj828(const uint8_t val)
{
	// val is a zero-indexed bit-value indicating the LED that shall be lit up
	//	for the bit order consider the branchtable_led[]
	__LED.flags ^= _BV(val);	// just toggle
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,	// addresses one single LED
	.public.Shine = &_component_led_mj828,	// addresses all the device's LEDs
	.public.Handler = &_charlieplexing_handler,  // timer-based periodic LED control function (e.g. charlieplexing)
	.flags = 0	// bitwise representation of 8 LEDs
	};

// implementation of virtual constructor for LEDs
static composite_led_t* _virtual_led_ctorMJ828()
{
	__LED.public.led[Red].Shine = &__LED_red;
	__LED.public.led[Yellow].Shine = &__LED_yellow;
	__LED.public.led[Green].Shine = &__LED_green;
	__LED.public.led[Blue].Shine = &__LED_blue;
	__LED.public.led[Battery1].Shine = &__LED_batt1;
	__LED.public.led[Battery2].Shine = &__LED_batt2;
	__LED.public.led[Battery3].Shine = &__LED_batt3;
	__LED.public.led[Battery4].Shine = &__LED_batt4;

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ828_

#endif /* MJ828_LED_C_ */
