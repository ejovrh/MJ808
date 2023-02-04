#include "main.h"
#include "stm32f0xx_it.h"

// Cortex-M0 Processor Interruption and Exception Handlers

void NMI_Handler(void)	// This function handles Non maskable interrupt
{
	while(1)
		{
			;
		}
}

void HardFault_Handler(void)	// This function handles Hard fault interrupt
{
	while(1)
		{
			;
		}
}

void SVC_Handler(void)	// This function handles System service call via SWI instruction
{
	;
}

void PendSV_Handler(void)  // This function handles Pendable request for system service
{
	;
}
