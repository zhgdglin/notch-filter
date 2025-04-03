#include "system.h"
/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // 
Author: 	     // 会上树的猪
Version: 	     // V.0
Date: 	       // 2020/12/28
Description:   // 串口驱动函数，针对硬件改版编写的底层驱动
History:       // V.0
*******************************************************************/
            
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
};
FILE __stdout;   

//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 

UART_Protocol STM32UartProtocol = 
{
	.Preamble = {0xA5,0xA5,0xA5,0xD5},
	.End_flag	= {0xA5,0xD4},
};

char Uart_Flag = 0;
u8  g_UartHEX[300];
//注意,读取USARTx->SR能避免莫名其妙的错误  
u16 USART_RX_STA=0;       			    //接收状态标记
u8  aRxBuffer[RXBUFFERSIZE];			    //HAL库使用的串口接收缓冲
u8  USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.

UART_HandleTypeDef UART1_Handler; 	//UART句柄

//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0X40)==0);			//循环发送,直到发送完毕   
	USART1->TDR=(u8)ch;      
	return ch;
}

void Uart1_Init(u32 bound)
{	
	//UART 初始化设置
	UART1_Handler.Instance = USART1;                      //USART1
	UART1_Handler.Init.BaudRate = bound;                  //波特率
	UART1_Handler.Init.WordLength = UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART1_Handler.Init.StopBits = UART_STOPBITS_1;        //一个停止位
	UART1_Handler.Init.Parity = UART_PARITY_NONE;         //无奇偶校验位
	UART1_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;   //无硬件流控
	UART1_Handler.Init.Mode = UART_MODE_TX_RX;            //收发模式
	HAL_UART_Init(&UART1_Handler);                        //HAL_UART_Init()会使能UART1
	
	HAL_UART_Receive_IT(&UART1_Handler, (u8 *)aRxBuffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量 
}

//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄
//PA9--->ST1_UART1_TX-->PC
//PA10-->ST1_UART1_RX-->PC
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{	
	//GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)                   //如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();               //使能GPIOD时钟
		__HAL_RCC_USART1_CLK_ENABLE();              //使能USART2时钟
	
		GPIO_Initure.Pin=GPIO_PIN_9|GPIO_PIN_10;    //PA9,PA10
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;          //复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;              //上拉
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;    //高速
		GPIO_Initure.Alternate=GPIO_AF7_USART1;     //复用为USART2
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);         //初始化PA9,PA10
		
		HAL_NVIC_EnableIRQ(USART1_IRQn);            //使能USART1中断通道
		HAL_NVIC_SetPriority(USART1_IRQn,1,0);      //抢占优先级1,子优先级0
	}	
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{	
	if(huart->Instance==USART1)                   //如果是串口1
	{
		if(aRxBuffer[0] == 0xA5 && Uart_Flag == 0)
		{			
			USART_RX_BUF[USART_RX_STA] = aRxBuffer[0];
			USART_RX_STA++;
			
			if(memcmp((char*)USART_RX_BUF,STM32UartProtocol.Preamble,3)==0)
			{
				Uart_Flag = 1;
			}
		}
		else 
		if(Uart_Flag==1)
		{
			if(aRxBuffer[0] == 0xD5)
			{
				USART_RX_BUF[USART_RX_STA] = aRxBuffer[0] ;
				USART_RX_STA++;
				Uart_Flag = 2;
			}
		}
		else 
		if(Uart_Flag == 2)
		{
			USART_RX_BUF[USART_RX_STA] = aRxBuffer[0] ;
			USART_RX_STA++;

			if(aRxBuffer[0] == 0xD4)
			{
				if(USART_RX_BUF[USART_RX_STA-2] == 0xA5)
				{
					g_UartHEXLen = USART_RX_STA;
					UART_CMD_ANALYSE();
					Uart_Flag = 0;
					USART_RX_STA = 0;
				}
			}
		}
	}	
}

//串口1中断服务程序
void USART1_IRQHandler(void)                	
{ 
	u32 timeout=0;
	u32 maxDelay=0x1FFFF;
	
	HAL_UART_IRQHandler(&UART1_Handler);    //调用HAL库中断处理公用函数
	
	timeout=0;
	while(HAL_UART_GetState(&UART1_Handler)!=HAL_UART_STATE_READY)    //等待就绪
	{
		timeout++;    //超时处理
		if(timeout>maxDelay) 
		{
			break;		
		}
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler,(u8 *)aRxBuffer, RXBUFFERSIZE)!=HAL_OK)    //一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
		timeout++;    //超时处理
		if(timeout>maxDelay) 
		{
			break;	
		}
	}	
}

void Uart1_Puts(u8 *data,u32 data_len)
{
	u32 i;
	for(i=0;i<data_len;i++)
	{
		USART1->TDR=data[i];
		while((USART1->ISR&0x40)==0);//等待发送结束
	}
}
