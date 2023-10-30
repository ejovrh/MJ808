/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define TIMER1_PERIOD 1249
#define UART2_BAUD 115200
#define TIMER_PRESCALER 799
#define UART_DUMP 1
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOF
#define BQ25798_INT_Pin GPIO_PIN_0
#define BQ25798_INT_GPIO_Port GPIOA
#define BQ25798_INT_EXTI_IRQn EXTI0_1_IRQn
#define BQ25798_CE_Pin GPIO_PIN_1
#define BQ25798_CE_GPIO_Port GPIOA
#define VCP_TX_Pin GPIO_PIN_2
#define VCP_TX_GPIO_Port GPIOA
#define BQ25798_QON_Pin GPIO_PIN_3
#define BQ25798_QON_GPIO_Port GPIOA
#define BQ25798_STAT_Pin GPIO_PIN_4
#define BQ25798_STAT_GPIO_Port GPIOA
#define BQ25798_STAT_EXTI_IRQn EXTI4_15_IRQn
#define LMR34206_PG_Pin GPIO_PIN_5
#define LMR34206_PG_GPIO_Port GPIOA
#define LMR34206_PG_EXTI_IRQn EXTI4_15_IRQn
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define VCP_RX_Pin GPIO_PIN_15
#define VCP_RX_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_3
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define _BV(x) (1 << x)
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
