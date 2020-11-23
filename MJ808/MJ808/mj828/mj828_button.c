#ifndef MJ828_BUTTON_C_
#define MJ828_BUTTON_C_

#include "mj828\mj828.h"
#include "button\button_types_actual.c"									// various __*button_t types - for convenience in one place for all LED devices

extern void _debounce(__individual_button_t * const in_button, event_handler_t * const in_event);

// wrapper function for the debouncer
static void _debounce_wrapper()
{
	for (uint8_t i=0; i<=__Button.__button_count; ++i)					// loop over all available buttons and debounce them
		_debounce(&__Button.__button[i], EventHandler);					// from here on the button is debounced and states can be consumed
};

static uint8_t PushButtonCaseTable[] =									// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x00,	// 0 - "momentary" not defined
	0x00,	// 1 - "toggle" not defined
	0x02,	// 2 - "hold" - jump to case 0x02 in _event_execution_function_mj828			// turn lights on/off
	0x01	// 3 - "error hold" not defined
};

static uint8_t HighBeamCaseTable[] =									// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x04,	// 0 - "momentary" - jump to case 0x04 in _event_execution_function_mj828 		// turn high beam on for the duration of the brake lever actuation
	0x00,	// 1 - "toggle" not defined
	0x00,	// 2 - "hold" not defined
	0x01	// 3 - "error hold" not defined
};

static uint8_t BrakeLightCaseTable[] =									// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x08,	// 0 - "momentary" - jump to case 0x08 in _event_execution_function_mj828		// turn brake light on for the duration of the brake lever actuation
	0x00,	// 1 - "toggle" not defined
	0x00,	// 2 - "hold" not defined
	0x01	// 3 - "error hold" not defined
};

static uint8_t PhotoTransistorCaseTable[] =									// array value at position #foo gets passed into __mjxxx_button_execution_function, where it is evaluated in a switch-case statement
{
	0x16,	// 0 - "momentary" - jump to case 0x16 in _event_execution_function_mj828		// turn brake light on for the duration of the brake lever actuation
	0x00,	// 1 - "toggle" not defined
	0x00,	// 2 - "hold" not defined
	0x01	// 3 - "error hold" not defined
};

//FIXME - left button doesn't work anymore as previously
static __individual_button_t __button_actual[] =						// instantiate array of actual button struct's
{
	// index0 - Pushbutton
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 0,												// pushbutton is connected to pin D0
		.__ButtonCaseptr = PushButtonCaseTable,							// button press-to-case binding
		.__inverse = 0													// button active high
	},
	// index 1 - HighBeam
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 4,												// high beam signal is connected to pin D4
		.__ButtonCaseptr = HighBeamCaseTable,							// button press-to-case binding
		.__inverse = 1													// button active low

	},
	// index 2 - BrakeLight
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 5,												// brake light signal is connected to pin D5
		.__ButtonCaseptr = BrakeLightCaseTable,							// button press-to-case binding
		.__inverse = 1													// button active low
	},
	// index 3 - PhotoTransistor
	{
		.__PIN = (uint8_t *) 0x30,										// 0x020 offset plus address - PIND register
		.__pin_number = 1,												// phototransistor collector is connected to pin D1
		.__ButtonCaseptr = PhotoTransistorCaseTable,					// button press-to-case binding
		.__inverse = 1													// low on light, high on darkness
	}
};

static individual_button_t * const __button_public[] =					// array of pointers to public members of button actual
{
	&__button_actual[Pushbutton].public,								// pushbutton on device
	&__button_actual[HighBeam].public,									// brake lever pressed forward - active low
	&__button_actual[BrakeLight].public,								// brake lever in brake position - active low
	&__button_actual[PhotoTransisitor].public							// phototransistor - light low
};

// implementation of virtual constructor for buttons
static button_t *_virtual_button_ctorMJ828(void)
{
	__Button.__button = __button_actual;								// assign pointer to button array
	__Button.__button_count = 3;										// btn0 - pushbutton, btn1 - high beam, btn2 - brake light

	__Button.public.button = *__button_public;							// assign pointer to array of pointers to public members of button actual
	__Button.public.deBounce = &_debounce_wrapper;						// tie in debounce function

	return &__Button.public;											// return address of public part; calling code accesses it via pointer
};

#endif /* MJ828_BUTTON_C_ */