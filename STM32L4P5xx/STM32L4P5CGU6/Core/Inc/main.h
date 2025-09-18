/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

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
#define TIMER_PRESCALER 799
#define TIMER2_PERIOD 0xFFFFFFFF
#define TIMER1_PERIOD 1249
#define TIMER1_PRESCALER 799
#define TIMER2_CH1_IC_FILTER 0xF
#define TIMER2_CH3_IC_FILTER 0xF
#define RN4678_SWButton_Pin GPIO_PIN_13
#define RN4678_SWButton_GPIO_Port GPIOC
#define RN4678_TransStat_Pin GPIO_PIN_14
#define RN4678_TransStat_GPIO_Port GPIOC
#define RN4678_ConnStat_Pin GPIO_PIN_15
#define RN4678_ConnStat_GPIO_Port GPIOC
#define HSE_IN_Pin GPIO_PIN_0
#define HSE_IN_GPIO_Port GPIOH
#define HSE_OUT_Pin GPIO_PIN_1
#define HSE_OUT_GPIO_Port GPIOH
#define ZeroCross_Pin GPIO_PIN_0
#define ZeroCross_GPIO_Port GPIOA
#define SDMMC_CMD_Pin GPIO_PIN_1
#define SDMMC_CMD_GPIO_Port GPIOA
#define DynamoCurrent_Pin GPIO_PIN_2
#define DynamoCurrent_GPIO_Port GPIOA
#define TCAN334_Standby_Pin GPIO_PIN_3
#define TCAN334_Standby_GPIO_Port GPIOA
#define AdjustableLoadDAC_Pin GPIO_PIN_4
#define AdjustableLoadDAC_GPIO_Port GPIOA
#define ADXL_INT1_Pin GPIO_PIN_5
#define ADXL_INT1_GPIO_Port GPIOA
#define QSPI_IO3_Pin GPIO_PIN_6
#define QSPI_IO3_GPIO_Port GPIOA
#define QSPI_IO2_Pin GPIO_PIN_7
#define QSPI_IO2_GPIO_Port GPIOA
#define QSPI_IO1_Pin GPIO_PIN_0
#define QSPI_IO1_GPIO_Port GPIOB
#define QSPI_IO0_Pin GPIO_PIN_1
#define QSPI_IO0_GPIO_Port GPIOB
#define RN4678_Reset_Pin GPIO_PIN_2
#define RN4678_Reset_GPIO_Port GPIOB
#define QSPI_SCK_Pin GPIO_PIN_10
#define QSPI_SCK_GPIO_Port GPIOB
#define QSPI_CS_Pin GPIO_PIN_11
#define QSPI_CS_GPIO_Port GPIOB
#define SDMMC_CLK_Pin GPIO_PIN_12
#define SDMMC_CLK_GPIO_Port GPIOB
#define SDMMC_CardDetect_Pin GPIO_PIN_13
#define SDMMC_CardDetect_GPIO_Port GPIOB
#define SDMMC_Data0_Pin GPIO_PIN_14
#define SDMMC_Data0_GPIO_Port GPIOB
#define SDMMC_Data1_Pin GPIO_PIN_15
#define SDMMC_Data1_GPIO_Port GPIOB
#define ApplicationLoadGate_Pin GPIO_PIN_8
#define ApplicationLoadGate_GPIO_Port GPIOA
#define RN4678_TX_Pin GPIO_PIN_9
#define RN4678_TX_GPIO_Port GPIOA
#define RN4678_RX_Pin GPIO_PIN_10
#define RN4678_RX_GPIO_Port GPIOA
#define RN4678_CTS_Pin GPIO_PIN_11
#define RN4678_CTS_GPIO_Port GPIOA
#define RN4678_RTS_Pin GPIO_PIN_12
#define RN4678_RTS_GPIO_Port GPIOA
#define AdjustableLoadGate_Pin GPIO_PIN_15
#define AdjustableLoadGate_GPIO_Port GPIOA
#define SDMMC_Data2_Pin GPIO_PIN_3
#define SDMMC_Data2_GPIO_Port GPIOB
#define SDMMC_Data3_Pin GPIO_PIN_4
#define SDMMC_Data3_GPIO_Port GPIOB
#define RN4678_Wakeup_Pin GPIO_PIN_5
#define RN4678_Wakeup_GPIO_Port GPIOB
#define I2C_SCL_Pin GPIO_PIN_6
#define I2C_SCL_GPIO_Port GPIOB
#define I2C_SDA_Pin GPIO_PIN_7
#define I2C_SDA_GPIO_Port GPIOB
#define LED_Reset_Pin GPIO_PIN_3
#define LED_Reset_GPIO_Port GPIOH
#define CAN_RX_Pin GPIO_PIN_8
#define CAN_RX_GPIO_Port GPIOB
#define CAN_TX_Pin GPIO_PIN_9
#define CAN_TX_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
