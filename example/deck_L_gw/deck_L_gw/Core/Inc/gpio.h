/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.h
  * @brief   This file contains all the function prototypes for
  *          the gpio.c file
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
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */
#define Set_Pin(X)     HAL_GPIO_WritePin(X##_GPIO_Port,X##_Pin,GPIO_PIN_SET)
#define Reset_Pin(X)   HAL_GPIO_WritePin(X##_GPIO_Port,X##_Pin,GPIO_PIN_RESET)
#define Read_Pin(X) 	 HAL_GPIO_ReadPin(X##_GPIO_Port,X##_Pin)
#define Toggle_Pin(X)  HAL_GPIO_TogglePin(X##_GPIO_Port,X##_Pin)
/* USER CODE END Private defines */

void MX_GPIO_Init(void);

/* USER CODE BEGIN Prototypes */
void Enter_Standby_Mode(void);
void Send_single(void);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

