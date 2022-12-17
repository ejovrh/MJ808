#ifndef __STM32F0xx_IT_H
#define __STM32F0xx_IT_H

void NMI_Handler(void);  // declaration of non-maksable interrupt handler
void HardFault_Handler(void);  // declaration of hard fault handler
void SVC_Handler(void);  // declaration of system service call via SWI instruction handler
void PendSV_Handler(void);	// declaration of pendable request for system service handler
void SysTick_Handler(void);  // declaration of  systick handler

#endif /* __STM32F0xx_IT_H */
