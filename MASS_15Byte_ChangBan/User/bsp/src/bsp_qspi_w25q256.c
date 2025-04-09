/*
*********************************************************************************************************
*
*	模块名称 : W25Q256 QSPI驱动模块
*	文件名称 : bsp_qspi_w25q256.c
*	版    本 : V1.0
*	说    明 : 使用CPU的QSPI总线驱动串行FLASH，提供基本的读写函数，采用4线方式，MDMA传输
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2020-11-01  armfly  正式发布
*
*	Copyright (C), 2020-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#include "bsp.h"



/* 
    STM32-V7开发板接线

	PG6/QUADSPI_BK1_NCS AF10
	PF10/QUADSPI_CLK	AF9
	PF8/QUADSPI_BK1_IO0 AF10
	PF9/QUADSPI_BK1_IO1 AF10
	PF7/QUADSPI_BK1_IO2 AF9
	PF6/QUADSPI_BK1_IO3 AF9

	W25Q256JV有512块，每块有16个扇区，每个扇区Sector有16页，每页有256字节，共计32MB
*/

/* QSPI引脚和时钟相关配置宏定义 */
#define QSPI_CLK_ENABLE()              __HAL_RCC_QSPI_CLK_ENABLE()
#define QSPI_CLK_DISABLE()             __HAL_RCC_QSPI_CLK_DISABLE()
#define QSPI_CS_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOG_CLK_ENABLE()
#define QSPI_CLK_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D0_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D1_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D2_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()
#define QSPI_BK1_D3_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()

#define QSPI_MDMA_CLK_ENABLE()         __HAL_RCC_MDMA_CLK_ENABLE()
#define QSPI_FORCE_RESET()             __HAL_RCC_QSPI_FORCE_RESET()
#define QSPI_RELEASE_RESET()           __HAL_RCC_QSPI_RELEASE_RESET()

#define QSPI_CS_PIN                    GPIO_PIN_6
#define QSPI_CS_GPIO_PORT              GPIOG

#define QSPI_CLK_PIN                   GPIO_PIN_10
#define QSPI_CLK_GPIO_PORT             GPIOF

#define QSPI_BK1_D0_PIN                GPIO_PIN_8
#define QSPI_BK1_D0_GPIO_PORT          GPIOF

#define QSPI_BK1_D1_PIN                GPIO_PIN_9
#define QSPI_BK1_D1_GPIO_PORT          GPIOF

#define QSPI_BK1_D2_PIN                GPIO_PIN_7
#define QSPI_BK1_D2_GPIO_PORT          GPIOF

#define QSPI_BK1_D3_PIN                GPIO_PIN_6
#define QSPI_BK1_D3_GPIO_PORT          GPIOF


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
