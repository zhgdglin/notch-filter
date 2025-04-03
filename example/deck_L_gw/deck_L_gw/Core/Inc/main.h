/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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
#include "stm32h7xx_hal.h"

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
#define LMC567_IN_Pin GPIO_PIN_0
#define LMC567_IN_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_1
#define LED1_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_2
#define LED2_GPIO_Port GPIOA
#define ADC_CS_Pin GPIO_PIN_4
#define ADC_CS_GPIO_Port GPIOA
#define ADC_SYNC_Pin GPIO_PIN_4
#define ADC_SYNC_GPIO_Port GPIOC
#define ADC_DRDY_Pin GPIO_PIN_5
#define ADC_DRDY_GPIO_Port GPIOC
#define ADC_DRDY_EXTI_IRQn EXTI9_5_IRQn
#define RS2253_C_Pin GPIO_PIN_0
#define RS2253_C_GPIO_Port GPIOB
#define RS2253_B_Pin GPIO_PIN_1
#define RS2253_B_GPIO_Port GPIOB
#define RS2253_A_Pin GPIO_PIN_2
#define RS2253_A_GPIO_Port GPIOB
#define MAX3221_FORCEON_Pin GPIO_PIN_8
#define MAX3221_FORCEON_GPIO_Port GPIOE
#define MAX3221_FORCEOFF_Pin GPIO_PIN_9
#define MAX3221_FORCEOFF_GPIO_Port GPIOE
#define RS232_EN_Pin GPIO_PIN_10
#define RS232_EN_GPIO_Port GPIOE
#define KEY5_Pin GPIO_PIN_13
#define KEY5_GPIO_Port GPIOE
#define KEY6_Pin GPIO_PIN_14
#define KEY6_GPIO_Port GPIOE
#define KEY7_Pin GPIO_PIN_15
#define KEY7_GPIO_Port GPIOE
#define KEY8_Pin GPIO_PIN_10
#define KEY8_GPIO_Port GPIOB
#define KEY9_Pin GPIO_PIN_11
#define KEY9_GPIO_Port GPIOB
#define IR2110S_SD_Pin GPIO_PIN_13
#define IR2110S_SD_GPIO_Port GPIOD
#define POWER_CAP_Pin GPIO_PIN_15
#define POWER_CAP_GPIO_Port GPIOD
#define LCD_CLK_Pin GPIO_PIN_3
#define LCD_CLK_GPIO_Port GPIOD
#define LCD_SID_Pin GPIO_PIN_4
#define LCD_SID_GPIO_Port GPIOD
#define LCD_CS_Pin GPIO_PIN_5
#define LCD_CS_GPIO_Port GPIOD
#define KEY1_Pin GPIO_PIN_6
#define KEY1_GPIO_Port GPIOD
#define KEY2_Pin GPIO_PIN_7
#define KEY2_GPIO_Port GPIOD
#define KEY3_Pin GPIO_PIN_3
#define KEY3_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_4
#define KEY4_GPIO_Port GPIOB
#define KEY10_Pin GPIO_PIN_5
#define KEY10_GPIO_Port GPIOB
#define RTC_SCLK_Pin GPIO_PIN_8
#define RTC_SCLK_GPIO_Port GPIOB
#define RTC_IO_Pin GPIO_PIN_9
#define RTC_IO_GPIO_Port GPIOB
#define RTC_CE_Pin GPIO_PIN_0
#define RTC_CE_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
