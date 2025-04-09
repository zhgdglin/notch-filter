/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.1
Date: 	       // 2023/02/06
Description:   // 简版通信机驱动，无任何算法应用
History:       // V.0
*******************************************************************/

#ifndef __BSP_GPIO_H
#define __BSP_GPIO_H

#include "bsp.h"

#define LED(n)		 (n?HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_3,GPIO_PIN_SET):HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_3,GPIO_PIN_RESET))
#define LED_Toggle (HAL_GPIO_TogglePin(GPIOJ, GPIO_PIN_3))

#define DRDY_High		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_SET)
#define DRDY_Low		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,GPIO_PIN_RESET)

#define DIR_FLAG_HIGH		HAL_GPIO_WritePin(GPIOK,GPIO_PIN_1,GPIO_PIN_SET)
#define DIR_FLAG_LOW		HAL_GPIO_WritePin(GPIOK,GPIO_PIN_1,GPIO_PIN_RESET)

#define FPGA_TRANS_HIGH	HAL_GPIO_WritePin(GPIOK,GPIO_PIN_2,GPIO_PIN_SET)
#define FPGA_TRANS_LOW	HAL_GPIO_WritePin(GPIOK,GPIO_PIN_2,GPIO_PIN_RESET)

void LED_Init(void);
void STM32H7ToFPGABusInit(void);
void PGA281CtrlInit(void);

void PowerAmplifierCtrlInit(void);
void PowerAmplifierON(void);
void PowerAmplifierOFF(void);

void EXTI_Init(void);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
