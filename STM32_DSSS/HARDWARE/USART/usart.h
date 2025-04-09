#ifndef __USART_H
#define __USART_H

#include "system.h"
 	
#define USART_REC_LEN	2048  						//定义最大接收字节数 2048
#define EN_USART2_RX	1									//使能（1）/禁止（0）串口2接收
#define EN_USART1_RX	1									//使能（1）/禁止（0）串口1接收
#define RXBUFFERSIZE	1 								//缓存大小

extern u8 g_UartHEX[300];
extern u8 USART_RX_BUF[USART_REC_LEN];  //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8 aRxBuffer[RXBUFFERSIZE];			//HAL库USART接收Buffer

void Uart1_Init(u32 bound);
void Uart1_Puts(u8 *data,u32 data_len);


typedef struct _UART_Protocol
{
  char Preamble[4]; //前导码
  char End_flag[2]; //结束标志
}UART_Protocol;

extern UART_Protocol STM32UartProtocol;
#endif /*__USART_H*/









