#ifndef MJ808_LED_C_
#define MJ808_LED_C_

#include "main.h"
#if defined(MJ808_)	// if this particular device is active

#include "mj808\mj808.h"
#include "led\led.h"

#include "led\composite_led_actual.c"	// __composite_led_t struct definition & declaration - for convenience in one place for all LED devices

static primitive_led_t __primitive_led[2] __attribute__ ((section (".data")));	// define array of actual LEDs and put into .data

// called indirectly by timer1 (_SystemInterrupt()), handles the fading
static void _FadeHandler(void)
{
	if(Device->led->led[Front].ocr > OCR_FRONT_LIGHT)  // fade up
		++OCR_FRONT_LIGHT;

	if(Device->led->led[Front].ocr < OCR_FRONT_LIGHT)  // fade down
		--OCR_FRONT_LIGHT;
}

// set OCR value to fade to
static inline void _primitiveFrontLED(const uint8_t value)
{
	Device->led->led[Front].ocr = value;	// set OCR value, the handler will do the rest
}

// TODO - optimise & should be static and the caller in question should use an object
// concrete utility LED handling function
static void _primitiveUtilityLED(uint8_t in_val)
{
	uint8_t led = 0;	// holds the pin of the LED: D0 - green (default), D1 - red

	if(in_val & _BV(B3))	// if the 4th bit is set, the command is for a red led, otherwise it is green
		led = 1;	// red

	// the led variable is relevant for bit-shifting, since the red and green LEDs are pin-wise next door neighbours;
	//	RedLED_Pin shifted left by one is the green LED

	in_val &= 7;	// clear everything except B2:0, which is the blink count (1-6)

	if(in_val == 0x00)	// B3:B0 is 0 - turn off
		{
			HAL_GPIO_WritePin(GPIOB, (RedLED_Pin << led), GPIO_PIN_SET);  // set high to turn off
			return;
		}

	if(in_val == 0x07)	// B3:B0 is 7 - turn on
		{
			HAL_GPIO_WritePin(GPIOB, (RedLED_Pin << led), GPIO_PIN_RESET);  // set low to turn on
			return;
		}

	while(in_val--)  // blink loop
		{
			HAL_Delay(BLINK_DELAY);  // TODO - get rid of blocking HAL_Delay();
			HAL_GPIO_TogglePin(GPIOB, (RedLED_Pin << led));  // toggle the led pin
			HAL_Delay(BLINK_DELAY);  // TODO - get rid of blocking HAL_Delay();
			HAL_GPIO_TogglePin(GPIOB, (RedLED_Pin << led));  // toggle the led pin
		}
}

static void __componentLED_On(void)
{
	Device->led->led[Utility].Shine(UTIL_LED_GREEN_ON);  // green LED on
	Device->led->led[Front].Shine(20);  // front light on - low key; gets overwritten by LU command, since it comes in a bit later

	//send the messages out, UDP-style. no need to check if the device is actually online
	MsgHandler->SendMessage(MSG_BUTTON_EVENT_BUTTON0_ON, 0x00, 1);	// convey button press via CAN and the logic unit will do its own thing
	MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0xFF, 2);  // turn on rear light
	MsgHandler->SendMessage(DASHBOARD_LED_YELLOW_ON, 0x00, 1);	// turn on yellow LED
}

static void __componentLED_Off(void)
{
	Device->led->led[Utility].Shine(UTIL_LED_GREEN_OFF);	// green LED off
	Device->led->led[Front].Shine(0);  // front light off

	// send the messages out, UDP-style. no need to check if the device is actually online
	MsgHandler->SendMessage(MSG_BUTTON_EVENT_BUTTON0_OFF, 0x00, 1);  // convey button press via CAN and the logic unit will tell me what to do
	MsgHandler->SendMessage((CMND_DEVICE | DEV_LIGHT | REAR_LIGHT), 0x00, 2);  // turn off rear light
	MsgHandler->SendMessage(DASHBOARD_LED_YELLOW_OFF, 0x00, 1);  // turn off yellow LED
}

// delegates operations from LED component downwards to LED leaves
static void _componentLED(const uint8_t val)
{
	if(val)  // true - on, false - off
		__componentLED_On();	// delegate indirectly to the leaves
	else
		__componentLED_Off();
}

static __composite_led_t                                       __LED =
	{  //
	.public.led = __primitive_led,  // assign pointer to LED array
	.public.Shine = &_componentLED,  // component part ("interface")
	.flags = 0	//
	};

// implementation of virtual constructor for LEDs
static composite_led_t* _virtual_led_ctorMJ808()
{
	__LED.public.led[Utility].Shine = &_primitiveUtilityLED;  // LED-specific implementation
	__LED.public.led[Utility].ocr = 0;	// TODO - not needed yet but it has potential...
	__LED.public.led[Front].Shine = &_primitiveFrontLED;  // LED-specific implementation
	__LED.public.led[Front].ocr = 0;	// is already at 0, but nevertheless
	__LED.public.Handler = &_FadeHandler;  // timer-based periodic LED control function

	return &__LED.public;  // return address of public part; calling code accesses it via pointer
}

#endif // MJ808_

#endif /* MJ808_LED_C_ */
