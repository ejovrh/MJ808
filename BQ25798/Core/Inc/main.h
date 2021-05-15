/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
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
void BlueButtonHandler(void);
void PQ25798IntHandler(void);

void Master_Reception_Callback(void);
void Master_Complete_Callback(void);
void UserButton_Callback(void);
void Error_Callback(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BlueButton_Pin GPIO_PIN_13
#define BlueButton_GPIO_Port GPIOC
#define BlueButton_EXTI_IRQn EXTI4_15_IRQn
#define AFE1_LED_ROLE_Pin GPIO_PIN_14
#define AFE1_LED_ROLE_GPIO_Port GPIOC
#define AFE1_LED_CC_Pin GPIO_PIN_15
#define AFE1_LED_CC_GPIO_Port GPIOC
#define ADC4_Pin GPIO_PIN_0
#define ADC4_GPIO_Port GPIOC
#define SAFE_RST_Pin GPIO_PIN_1
#define SAFE_RST_GPIO_Port GPIOC
#define AFE0_TX_EN_Pin GPIO_PIN_2
#define AFE0_TX_EN_GPIO_Port GPIOC
#define LMR34206_PG_Pin GPIO_PIN_3
#define LMR34206_PG_GPIO_Port GPIOC
#define AFE0_ABSIDE_Pin GPIO_PIN_0
#define AFE0_ABSIDE_GPIO_Port GPIOA
#define AFE0_ALERT_Pin GPIO_PIN_1
#define AFE0_ALERT_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define Rectifier_1_Pin GPIO_PIN_4
#define Rectifier_1_GPIO_Port GPIOA
#define LED_USER_Pin GPIO_PIN_5
#define LED_USER_GPIO_Port GPIOA
#define AFE1_LED_Vbus_Pin GPIO_PIN_6
#define AFE1_LED_Vbus_GPIO_Port GPIOA
#define AFE0_ADC_I_Pin GPIO_PIN_7
#define AFE0_ADC_I_GPIO_Port GPIOA
#define AFE0_ADC_V_Pin GPIO_PIN_4
#define AFE0_ADC_V_GPIO_Port GPIOC
#define AFE0_LED_ROLE_Pin GPIO_PIN_5
#define AFE0_LED_ROLE_GPIO_Port GPIOC
#define Rectifier_1B0_Pin GPIO_PIN_0
#define Rectifier_1B0_GPIO_Port GPIOB
#define AFE0_LED_Vbus_Pin GPIO_PIN_1
#define AFE0_LED_Vbus_GPIO_Port GPIOB
#define AFE0_LED_CC_Pin GPIO_PIN_2
#define AFE0_LED_CC_GPIO_Port GPIOB
#define STUSB_I2C_SCL_Pin GPIO_PIN_10
#define STUSB_I2C_SCL_GPIO_Port GPIOB
#define STUSB_I2C_SDA_Pin GPIO_PIN_11
#define STUSB_I2C_SDA_GPIO_Port GPIOB
#define AFE0_SPI_SS_Pin GPIO_PIN_12
#define AFE0_SPI_SS_GPIO_Port GPIOB
#define AFE0_SPI_SCK_Pin GPIO_PIN_13
#define AFE0_SPI_SCK_GPIO_Port GPIOB
#define AFE0_SPI_MISO_Pin GPIO_PIN_14
#define AFE0_SPI_MISO_GPIO_Port GPIOB
#define AFE0_SPI_MOSI_Pin GPIO_PIN_15
#define AFE0_SPI_MOSI_GPIO_Port GPIOB
#define AFE0_RESET_Pin GPIO_PIN_6
#define AFE0_RESET_GPIO_Port GPIOC
#define PQ25798_CE_Pin GPIO_PIN_7
#define PQ25798_CE_GPIO_Port GPIOC
#define PQ25798_QON_Pin GPIO_PIN_8
#define PQ25798_QON_GPIO_Port GPIOC
#define PQ25798_STAT_Pin GPIO_PIN_9
#define PQ25798_STAT_GPIO_Port GPIOC
#define DRP_Pin GPIO_PIN_8
#define DRP_GPIO_Port GPIOA
#define PQ25798_INT_Pin GPIO_PIN_9
#define PQ25798_INT_GPIO_Port GPIOA
#define PQ25798_INT_EXTI_IRQn EXTI4_15_IRQn
#define Rectifier_2_Pin GPIO_PIN_10
#define Rectifier_2_GPIO_Port GPIOA
#define USB_DN_Pin GPIO_PIN_11
#define USB_DN_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define SWD_TMS_Pin GPIO_PIN_13
#define SWD_TMS_GPIO_Port GPIOA
#define SWD_TCK_Pin GPIO_PIN_14
#define SWD_TCK_GPIO_Port GPIOA
#define AFE0_SRC_EN_Pin GPIO_PIN_10
#define AFE0_SRC_EN_GPIO_Port GPIOC
#define Rectifier_3_Pin GPIO_PIN_11
#define Rectifier_3_GPIO_Port GPIOC
#define AFE0_SNK_EN_Pin GPIO_PIN_12
#define AFE0_SNK_EN_GPIO_Port GPIOC
#define Rectifier_5_Pin GPIO_PIN_2
#define Rectifier_5_GPIO_Port GPIOD
#define Rectifier_4_Pin GPIO_PIN_3
#define Rectifier_4_GPIO_Port GPIOB
#define WheelZeroCross_Pin GPIO_PIN_4
#define WheelZeroCross_GPIO_Port GPIOB
#define PORT0PW1_Pin GPIO_PIN_6
#define PORT0PW1_GPIO_Port GPIOB
#define PORT0PW0_Pin GPIO_PIN_7
#define PORT0PW0_GPIO_Port GPIOB
#define SAFE_I2C_SCL_Pin GPIO_PIN_8
#define SAFE_I2C_SCL_GPIO_Port GPIOB
#define SAFE_I2C_SDA_Pin GPIO_PIN_9
#define SAFE_I2C_SDA_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
