/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪@sunyubo
Version: 	     // V.2
Date: 	       // 2023/11/23
Description:   // 长条板通信机驱动
History:       // V.1
*******************************************************************/

#ifndef _BSP_USART_FIFO_H_
#define _BSP_USART_FIFO_H_

#define USART_REC_LEN	2048  						//?¨ò?×?′ó?óê?×??úêy 2048

extern uint8_t RxBuff[USART_REC_LEN];

extern __IO uint16_t RxNum;
extern __IO uint8_t g_UartIsOK;
extern __IO uint8_t g_UartIsOK8;

extern UART_HandleTypeDef huart1;

void bsp_UartInit(uint32_t Bound);
void Uart1_Puts(uint8_t *data,uint32_t data_len);
void Uart8_Puts(uint8_t *data,uint32_t data_len);	
#endif

/***************************** 会上树的猪 (END OF FILE) *********************************/
