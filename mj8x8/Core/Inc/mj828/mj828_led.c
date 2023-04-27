#ifndef MJ828_LED_C_
#define MJ828_LED_C_

#if defined(MJ828_)	// if this particular device is active

#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

extern TIM_HandleTypeDef htim14;  // charlieplexed LED handling - 2ms

static primitive_led_t __primitive_led[8] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data
static __composite_led_t __LED;	// forward declaration of object
static GPIO_InitTypeDef GPIO_InitStruct =  // GPIO initialisation structure
	{0};

static uint32_t (*_fptr)(const uint8_t state);	// function pointer for branch table
static uint8_t _BlinkExclusionMask;	// exclusion mask used for blinking

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
static void __physicalRedLED(const uint8_t state)  // red LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, (!state));  // cathode
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // cathode
}

// handler for physical LED
static void __physicalGreenLED(const uint8_t state)  // green LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // cathode
}

// handler for physical LED
static void __physicalBlueLED(const uint8_t state)  // blue1 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // cathode
}

// handler for physical LED
static void __physicalYellowLED(const uint8_t state)  // yellow LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP3_GPIO_Port, CP3_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP2_GPIO_Port, CP2_Pin);  // cathode
}

// handler for physical LED
static void __physicalBatt1LED(const uint8_t state)  // blue2 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP3_GPIO_Port, CP3_Pin);  // cathode
}

// handler for physical LED
static void __physicalBatt2LED(const uint8_t state)  // blue3 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP3_GPIO_Port, CP3_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // cathode
}

// handler for physical LED
static void __physicalBatt3LED(const uint8_t state)  // blue4 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // cathode
}

// handler for physical LED
static void __physicalBatt4LED(const uint8_t state)  // blue5 LED on/off
{
	// state == 1 - on
	// state == 0 - off
	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, (!state));	// cathode
	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_SET);	// anode

	___SetPinToOutput(&GPIO_InitStruct, CP4_GPIO_Port, CP4_Pin);  // anode
	___SetPinToOutput(&GPIO_InitStruct, CP1_GPIO_Port, CP1_Pin);  // cathode
}

// branch table for direct primitive LED execution
static uint32_t (*__physicalLEDBranchTable[])(const uint8_t state) =
		{//
				(void *)&__physicalRedLED,	// physical red LED on/off
				(void *)&__physicalGreenLED,	// physical green LED on/off
				(void *)&__physicalYellowLED,	// ditto
				(void *)&__physicalBlueLED,	// ditto
				(void *)&__physicalBatt1LED,
				(void *)&__physicalBatt2LED,
				(void *)&__physicalBatt3LED,
				(void *)&__physicalBatt4LED,
		};

// handles the time-based charlieplexing stuff, is being called by timer14 - every 2ms
static void _CharliePlexingHandler()
{
	static uint8_t i = 0;  // persistent iterator across function calls loops over all LEDs on device

	/* charlieplexing handler rationale
	 * with charlieplexing, the rationale is that only a single LED can be lit at any time so that GPIO current limits are not exhausted
	 * if e.g. two or more LEDs have to be lit, they must be lit one after the other in rapid succession;
	 * if this is done fast enough (below 20ms), the human eye will perceive both as lit at the same time
	 *
	 * the handler is to be called from a timer ISR periodically
	 * i is a static walker variable, running repeatedly from 0 to 7, thereby indicating which LED in numerical order (as defined in __primitive_led[8]) shall be lit up
	 */

	if((__LED._ShineFlags | __LED._BlinkFlags) == 0)	// if there is any LED to glow at all
		{
			__SetAllGPIOtoAnalog(&GPIO_InitStruct);	// set LED pins to initial state

			Device->StopTimer(&htim14);  // stop the timer
			Device->mj8x8->UpdateActivity(LEDS, OFF);	// mark inactivity
			return;
		}

	// 126 = slow, 64 - medium, 32 = fast
	if ((__LED._BlinkCounter & 32) == 0)	// blink frequency by means of ANDing whole base-2 numbers
		_BlinkExclusionMask &= (uint8_t) (~__LED._BlinkFlags);	// clear particular bits - populate exclusion (i.e. primitive LED off for blinking)
	else
		_BlinkExclusionMask = 0xFF;	// all on (normal glow)

	__SetAllGPIOtoAnalog(&GPIO_InitStruct);	// set LED pins to initial state

//	HAL_GPIO_WritePin(CP1_GPIO_Port, CP1_Pin, GPIO_PIN_RESET);	// begin charlieplexing GPIO sequence
//	HAL_GPIO_WritePin(CP2_GPIO_Port, CP2_Pin, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(CP3_GPIO_Port, CP3_Pin, GPIO_PIN_RESET);
//	HAL_GPIO_WritePin(CP4_GPIO_Port, CP4_Pin, GPIO_PIN_RESET);

	_fptr = __physicalLEDBranchTable[i];	// set function pointer to indicated address of primitive LED function
	(_fptr)(  ( (__LED._ShineFlags | (__LED._BlinkFlags & _BlinkExclusionMask) ) & _BV(i) ) > 0  );	// it is ridiculous, i know...

	/* blinking rationale
	 * for blinking, we start with a particular LED turned on, i.e. its bit is set, so (__LED._ShineFlags | __LED._BlinkFlags) is true
	 * _BlinkExclusionMask is an inverted __LED._BlinkFlags - a bitmask that:
	 * 	- periodically is 0xFF (a particular blinkable LED has its bit set) and
	 * 	- periodically has the blinkable LED bit unset
	 * the above _BlinkExclusionMask ANDed with the above two shine and blink masks sets bits for constantly lit LEDs and periodically sets/unsets
	 * bits for blinkable LEDs
	 *
	 * _BV(i) via the iterator does the charlieplexing; the final AND operator gives the blink frequency.
	 */

	(i >= 7) ? i = 0 : ++i;  // circular bit-wise iterator over a byte range, count up to 7 and then restart from zero (we have 8 LEDs)

	if (__LED._BlinkFlags)	// increment the counter only if there is stuff to blink
		++__LED._BlinkCounter;
}

// backend to all frontends - sets the shine/blink flags for _CharliePlexingHandler() to work on
static inline void __LEDBackEnd(const uint8_t led, const uint8_t state)
{
	/* note: since the charlieplexer operates on flags (bit 1 = red, bit 2 = green, .... -- see the mj828_leds enum),
	 * something has to set them -- regardless whether the LEDs are operated individually (via Shine() on the primitive LED type)
	 * or Shine() on the composite LED type
	 * what sets these flags is __primitivegeneralLEDHandler().
	 * one can think of it as being the backend, while the primitive (__primitiveRedLEDHandler)
	 * and composite (_componentLEDHandler) handlers are the frontends.
	 */

	if (state == ON)	// transition from BLINK to ON state (or OFF to ON, in which case BlinkFlags are empty anyway)
		__LED._OldBlinkFlags = __LED._BlinkFlags;	// store previous blink state

	if(state == OFF) // transition from SHINE to BLINK state (or ON to OFF, in which case BlinkFlags are empty anyway)
		{
			if (__LED._OldBlinkFlags != 0)	// ...really from SHINE to BLINK
				{
					__LED._BlinkFlags = __LED._OldBlinkFlags;	// restore previous blink state
					__LED._OldBlinkFlags &= ~__LED._OldBlinkFlags;	// clear the bits
				}
			else	// observe for dirty write condition (blink issued from other devices)
			__LED._BlinkFlags ^= ((-((state>>1) & 0x01) ^ __LED._BlinkFlags) & (1 << led));	// sets "led" bit to "state" value
		}
		else	// transition to BLINK (or true OFF) state
			__LED._BlinkFlags ^= ((-((state>>1) & 0x01) ^ __LED._BlinkFlags) & (1 << led));	// sets "led" bit to "state" value

	__LED._ShineFlags ^= ((-(state & 0x01) ^ __LED._ShineFlags) & (1 << led));	// sets "led" bit to "state" value

	Device->mj8x8->UpdateActivity(LEDS, (__LED._ShineFlags | __LED._BlinkFlags) > 0);	// mark in/activity

	Device->StartTimer(&htim14);  // start the timer
}

// frontend for the composite LED handler
static inline void _componentLEDFrontEnd(const uint8_t arg)
{
//	argument needs to be e.g. (RED | ON)

	__LEDBackEnd( ( ((arg & 0x3C) >> 2) - 1 ) , (arg & 0x03));
}

// frontend for the primitive red LED handler
static inline void __primitiveRedLEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Red, state);	// execute according to state
}

// frontend for the primitive green LED handler
static inline void __primitiveGreenLEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Green, state);	// execute according to state
}

// frontend for the primitive yellow LED handler
static inline void __primitiveYellowLEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Yellow, state);	// execute according to state
}

// frontend for the primitive blue LED handler
static inline void __primitiveBlueLEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Blue, state);	// execute according to state
}

// frontend for the primitive battery1 LED handler
static inline void __primitiveBatt1LEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Battery1, state);	// execute according to state
}

// frontend for the primitive battery2 LED handler
static inline void __primitiveBatt2LEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Battery2, state);	// execute according to state
}

// frontend for the primitive battery3 LED handler
static inline void __primitiveBatt3LEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Battery3, state);	// execute according to state
}

// frontend for the primitive battery4 LED handler
static inline void __primitiveBatt4LEDFrontEnd(const uint8_t state)
{
	__LEDBackEnd(Battery4, state);	// execute according to state
}

static __composite_led_t __LED =
	{  //
	.public.led = __primitive_led,	// addresses one single LED
	.public.Shine = &_componentLEDFrontEnd,  // addresses all the device's LEDs
	.public.Handler = &_CharliePlexingHandler,  // timer-based periodic LED control function (e.g. charlieplexing)
	._ShineFlags = 0	// bitwise representation of 8 LEDs
	};

// implementation of virtual constructor for LEDs
composite_led_t* _virtual_led_ctor()
{

	__LED._ShineFlags = 0;	// zero the flags
	__LED._BlinkFlags = 0;	// ditto
	__LED._BlinkCounter = 0;	//

	__LED.public.led[Red].Shine = &__primitiveRedLEDFrontEnd;  // control function for one single LED
	__LED.public.led[Green].Shine = &__primitiveGreenLEDFrontEnd;
	__LED.public.led[Yellow].Shine = &__primitiveYellowLEDFrontEnd;
	__LED.public.led[Blue].Shine = &__primitiveBlueLEDFrontEnd;
	__LED.public.led[Battery1].Shine = &__primitiveBatt1LEDFrontEnd;
	__LED.public.led[Battery2].Shine = &__primitiveBatt2LEDFrontEnd;
	__LED.public.led[Battery3].Shine = &__primitiveBatt3LEDFrontEnd;
	__LED.public.led[Battery4].Shine = &__primitiveBatt4LEDFrontEnd;

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ828_

#endif /* MJ828_LED_C_ */
