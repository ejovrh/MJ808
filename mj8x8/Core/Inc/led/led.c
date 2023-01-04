//#include "led.h"
#include <inttypes.h>

// TODO - fade() needs testing
// fades OCR from off to value / from value to off
void _fade(const uint8_t value, volatile uint8_t *ocr)
{
	uint32_t prim = __get_PRIMASK();  // read PRIMASK register - 0 if enabled, non-zero if disabled

	// TODO - implement fading so that the human eye sees it as uniform
	// TODO - verify interrupts are interfering with fade
	__disable_irq();  // if with interrupts, *ocr gets corrupted; i'm suspecting an ISR while OCR is incrementing/decrementing
	//	hence an atomic fade()

	if(value > *ocr)	// we need to get brighter
		{
			while(++*ocr < value)  // loop until we match the OCR with the requested value & increment the OCR
				HAL_Delay(1);  // delay it a bit for visual stimulus ...

			if(!prim)  //
				__enable_irq();  // enable interrupts

			return;
		}

	if(value < *ocr)	// we need to get dimmer & decrement the OCR
		{
			while(--*ocr > value)  // loop until we match the OCR with the requested value
				HAL_Delay(1);  // delay it a bit for visual stimulus ...

			if(!prim)  //
				__enable_irq();  // enable interrupts

			return;
		}
}
