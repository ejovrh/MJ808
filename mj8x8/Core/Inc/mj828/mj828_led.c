#ifndef MJ828_LED_C_
#define MJ828_LED_C_

#if defined(MJ828_)	// if this particular device is active

#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

extern TIM_HandleTypeDef htim14;  // Timer14 object;

static primitive_led_t __primitive_led[8] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data
static __composite_led_t __LED;	// forward declaration of object
static GPIO_InitTypeDef GPIO_InitStruct =  // GPIO initialisation structure
	{0};
static uint32_t (*_primitiveLEDfptr)(const uint8_t state);	// function pointer for above branch table

// switches a given pin on a port to output
static inline void ___SetPinToOutput(GPIO_InitTypeDef *a, GPIO_TypeDef *inPort, const uint16_t inPin)
{
	a->Pin = inPin;
	a->Mode = GPIO_MODE_OUTPUT_PP;
	a->Pull = GPIO_NOPULL;
	a->Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(inPort, &*a);
}

// sets all charlieplexing GPIOs back to Hi-Z (initial state)
static inline void __SetAllGPIOtoAnalog(GPIO_InitTypeDef *a)
{
	// set LED pins to initial state
	a->Pin = CP1_Pin | CP2_Pin | CP3_Pin;
	a->Mode = GPIO_MODE_ANALOG;
	a->Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &*a);

	a->Pin = CP4_Pin;
	a->Mode = GPIO_MODE_ANALOG;
	a->Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &*a);
}

// handler for physical LED
static void __primitiveRedLED(const uint8_t state)  // red LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, (!state));  // cathode
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // cathode
}

// handler for physical LED
static void __primitiveGreenLED(const uint8_t state)  // green LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // cathode
}

// handler for physical LED
static void __primitiveBlueLED(const uint8_t state)  // blue1 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // cathode
}

// handler for physical LED
static void __primitiveYellowLED(const uint8_t state)  // yellow LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP3_GPIO_Port, CP3_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // cathode
}

// handler for physical LED
static void __primitiveBatt1LED(const uint8_t state)  // blue2 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP3_GPIO_Port, CP3_Pin);  // cathode
}

// handler for physical LED
static void __primitiveBatt2LED(const uint8_t state)  // blue3 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP3_GPIO_Port, CP3_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // cathode
}

// handler for physical LED
static void __primitiveBatt3LED(const uint8_t state)  // blue4 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // cathode
}

// handler for physical LED
static void __primitiveBatt4LED(const uint8_t state)  // blue5 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // cathode
}

// branch table for direct primitive LED execution
static uint32_t (*__primitiveLEDBranchTable[])(const uint8_t state) =
		{//
				(void *)&__primitiveRedLED,
				(void *)&__primitiveGreenLED,
				(void *)&__primitiveYellowLED,
				(void *)&__primitiveBlueLED,
				(void *)&__primitiveBatt1LED,
				(void *)&__primitiveBatt2LED,
				(void *)&__primitiveBatt3LED,
				(void *)&__primitiveBatt4LED,
		};

// handles the time-based charlieplexing stuff, is being called by timer14 - every 2ms
static void _CharliePlexingHandler()
{
	/* charlieplexing handler rationale
	 * with charlieplexing, the rationale is that only a single LED can be lit at any time so that GPIO current limits are not exhausted
	 * if e.g. two or more LEDs have to be lit, they must be lit one after the other in rapid succession;
	 * if this is done fast enough (below 20ms), the human eye will perceive both as lit at the same time
	 *
	 * the handler is to be called from a timer ISR periodically
	 * i is a static walker variable, running repeatedly from 0 to 7, thereby indicating which LED in numerical order (as defined in __primitive_led[8]) shall be lit up
	 */
	// TODO - _CharliePlexingHandler - implement blinking
	if(!__LED._flags)	// if there is any LED to glow at all
		{
			Device->StopTimer(&htim14);  // stop the time
			Device->activity->LEDsOn = 0;	// mark inactivity
		}

	static uint8_t i = 0;  // persistent iterator across function calls loops over all LEDs on device

	// set LED pins to initial state
	__SetAllGPIOtoAnalog(&GPIO_InitStruct);

	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_RESET);

	_primitiveLEDfptr = __primitiveLEDBranchTable[i];	// set function pointer to indicated address of primitive LED function
	(_primitiveLEDfptr)(__LED._flags & _BV(i));	// pass arguments and execute

	// !!!!	iterator
	(i >= 7) ? i = 0 : ++i;  // count up to 7 and then restart from zero (we have 8 LEDs)
}

// toggles a bit in the LED flags variable; charlieplexer in turn makes it shine
static void _componentLEDHandler(const uint8_t led)
{
	Device->StartTimer(&htim14);  // start the timer

	// val is a zero-indexed bit-value indicating the LED that shall be lit up
	__LED._flags ^= _BV(led);	// just toggle
	Device->activity->LEDsOn = (__LED._flags > 0);	// mark in/activity
}

// sets the flag for the red LED to glow
static inline void __primitivegeneralLEDHandler(const uint8_t led, const uint8_t state)
{
	Device->activity->LEDsOn |= (state > 0);
	__LED._flags ^= ((-state ^ __LED._flags) & (1 << led));	// sets "led" bit to "state" value
		Device->StartTimer(&htim14);  // start the timer
}

// sets the flag for the red LED to glow
static inline void __primitiveRedLEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Red, state);	// execute according to state
}

// sets the flag for the green LED to glow
static inline void __primitiveGreenLEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Green, state);	// execute according to state
}

// sets the flag for the yellow LED to glow
static inline void __primitiveYellowLEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Yellow, state);	// execute according to state
}

// sets the flag for the blue LED to glow
static inline void __primitiveBlueLEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Blue, state);	// execute according to state
}

// sets the flag for the battery1 LED to glow
static inline void __primitiveBatt1LEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Battery1, state);	// execute according to state
}

// sets the flag for the battery2 LED to glow
static inline void __primitiveBatt2LEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Battery2, state);	// execute according to state
}

// sets the flag for the battery3 LED to glow
static inline void __primitiveBatt3LEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Battery3, state);	// execute according to state
}

// sets the flag for the battery4 LED to glow
static inline void __primitiveBatt4LEDHandler(const uint8_t state)
{
	__primitivegeneralLEDHandler(Battery4, state);	// execute according to state
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,	// addresses one single LED
	.public.Shine = &_componentLEDHandler,  // addresses all the device's LEDs
	.public.Handler = &_CharliePlexingHandler,  // timer-based periodic LED control function (e.g. charlieplexing)
	._flags = 0	// bitwise representation of 8 LEDs
	};

// implementation of virtual constructor for LEDs
composite_led_t* _virtual_led_ctor()
{
	__LED._flags = 0;	// zero the flags

	__LED.public.led[Red].Shine = &__primitiveRedLEDHandler;  // control function for one single LED
	__LED.public.led[Green].Shine = &__primitiveGreenLEDHandler;
	__LED.public.led[Yellow].Shine = &__primitiveYellowLEDHandler;
	__LED.public.led[Blue].Shine = &__primitiveBlueLEDHandler;
	__LED.public.led[Battery1].Shine = &__primitiveBatt1LEDHandler;
	__LED.public.led[Battery2].Shine = &__primitiveBatt2LEDHandler;
	__LED.public.led[Battery3].Shine = &__primitiveBatt3LEDHandler;
	__LED.public.led[Battery4].Shine = &__primitiveBatt4LEDHandler;

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ828_

#endif /* MJ828_LED_C_ */
