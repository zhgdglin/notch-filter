/*
*********************************************************************************************************
*
*	模块名称 : BSP模块(For STM32H7)
*	文件名称 : bsp.h
*	版    本 : V1.0
*	说    明 : 这是硬件底层驱动程序的主文件。每个c文件可以 #include "bsp.h" 来包含所有的外设驱动模块。
*			   bsp = Borad surport packet 板级支持包
*	修改记录 :
*		版本号  日期         作者       说明
*		V1.0    2018-07-29  Eric2013   正式发布
*
*	Copyright (C), 2018-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef _BSP_H_
#define _BSP_H_

#define STM32_V7    

//定义一些常用的数据类型短关键字 
typedef unsigned       char u8;
typedef unsigned short int  u16;
typedef unsigned       int  u32;

typedef signed         char s8;
typedef signed short   int  s16;
typedef signed         int  s32;

/* 检查是否定义了开发板型号 */
#if !defined (STM32_V7)
	#error "Please define the board model : STM32_V7"
#endif

/* 定义 BSP 版本号 */
#define __STM32H7_BSP_VERSION		"1.1"

/* CPU空闲时执行的函数 */
//#define CPU_IDLE()		bsp_Idle()

/* 开关全局中断的宏 */
#define ENABLE_INT()	__set_PRIMASK(0)	/* 使能全局中断 */
#define DISABLE_INT()	__set_PRIMASK(1)	/* 禁止全局中断 */

/* 这个宏仅用于调试阶段排错 */
#define BSP_Printf		printf
//#define BSP_Printf(...)

#define EXTI9_5_ISR_MOVE_OUT		/* bsp.h 中定义此行，表示本函数移到 stam32f4xx_it.c。 避免重复定义 */

#define ERROR_HANDLER()		Error_Handler(__FILE__, __LINE__);

/* 默认是关闭状态 */
#define  Enable_EventRecorder  0

#if Enable_EventRecorder == 1
	#include "EventRecorder.h"
#endif

#include "stm32h7xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <complex.h>

#ifndef TRUE
	#define TRUE  1
#endif

#ifndef FALSE
	#define FALSE 0
#endif

/* 定义优先级分组 */
#define NVIC_PREEMPT_PRIORITY	4

/* 通过取消注释或者添加注释的方式控制是否包含底层驱动模块 */

#include "bsp_beep.h"
#include "bsp_dwt.h"
#include "bsp_led.h"
#include "bsp_key.h"
#include "bsp_timer.h"
#include "bsp_uart_fifo.h"
#include "bsp_fmc_sdram.h"
#include "bsp_fmc_io.h"
#include "bsp_gpio.h"
#include "bsp_spi_bus.h"
#include "bsp_sdio_sd.h"
#include "bsp_tim_pwm.h"
#include "bsp_qspi.h"
#include "bsp_fatfs.h"
#include "bsp_rtc.h"	

#include "Parameter.h"
#include "Function.h"
#include "malloc.h"
#include "modulate.h"
#include "demodulate.h"

#include "arm_math.h"
#include "arm_const_structs.h"

#include "ConvCode.h"
#include "LogicCtrl.h"


/* 提供给其他C文件调用的函数 */
void bsp_Init(void);
void bsp_Idle(void);
void AlgorithmVarialbeInit(void);

void bsp_GetCpuID(uint32_t *_id);
void Error_Handler(char *file, uint32_t line);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);
#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
