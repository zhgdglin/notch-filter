/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.1
Date: 	       // 2023/02/06
Description:   // 简版通信机驱动，无任何算法应用
History:       // V.0
*******************************************************************/
#include "bsp.h"

void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
	__HAL_RCC_GPIOJ_CLK_ENABLE();

	GPIO_Initure.Pin=GPIO_PIN_1;
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_Initure.Pull=GPIO_PULLUP;
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOJ,&GPIO_Initure);

	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_1,GPIO_PIN_RESET);
  
  //配置传输方向GPIO-->PI10
//	__HAL_RCC_GPIOK_CLK_ENABLE();
//	GPIO_Initure.Pin=GPIO_PIN_1|GPIO_PIN_2;
//	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
//	GPIO_Initure.Pull=GPIO_PULLUP;
//	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
//	HAL_GPIO_Init(GPIOK,&GPIO_Initure);	
}

void STM32H7ToFPGABusInit(void)
{
  GPIO_InitTypeDef GPIO_Initure;
	
	//配置传输方向GPIO-->PI10
	__HAL_RCC_GPIOK_CLK_ENABLE();
	GPIO_Initure.Pin=GPIO_PIN_1|GPIO_PIN_2;
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_Initure.Pull=GPIO_PULLUP;
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	
	HAL_GPIO_Init(GPIOK,&GPIO_Initure);
}
/*
  PGA281控制（G4：PJ10 G3：PJ9 G2：PJ8 G1：PJ7 G0：PJ6）
  Code    Gain
  00000   0.125
  00001   0.25
  00010   0.5
  00011   1.0
  00100   2.0
  00101   4.0
  00110   8.0
  00111   16.0
  01000   32.0
  01001   64.0
  01010   128.0
*/

void PGA281CtrlInit(void)
{
  GPIO_InitTypeDef GPIO_Initure;
	
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  GPIO_Initure.Pin  = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_Initure.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_Initure.Pull = GPIO_PULLUP;
  GPIO_Initure.Speed= GPIO_SPEED_LOW;
  
  HAL_GPIO_Init(GPIOJ,&GPIO_Initure);
  
  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET); 
  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_14|GPIO_PIN_15,GPIO_PIN_RESET); 
}


void PowerAmplifierCtrlInit(void)
{
  GPIO_InitTypeDef GPIO_Initure;
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  
	GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_10;
	GPIO_Initure.Mode=GPIO_MODE_INPUT;
	GPIO_Initure.Pull=GPIO_PULLUP;
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	
  GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5;
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;
	GPIO_Initure.Pull=GPIO_PULLUP;
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOJ,&GPIO_Initure);

	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_2|GPIO_PIN_4|GPIO_PIN_5,GPIO_PIN_RESET); 
//  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_2,GPIO_PIN_SET); 
//	HAL_Delay(500);
//  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_5,GPIO_PIN_SET); 	
//	HAL_Delay(200);
//	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_4,GPIO_PIN_SET); 
}

void PowerAmplifierON(void)
{	 
  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_2,GPIO_PIN_SET); 
	HAL_Delay(500);
  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_5,GPIO_PIN_SET); 	
	HAL_Delay(500);
//	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_4,GPIO_PIN_RESET); 
//	HAL_Delay(500);
}

void PowerAmplifierOFF(void)
{
  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_5,GPIO_PIN_RESET); 
	HAL_Delay(500);
  HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_2,GPIO_PIN_RESET); 
//	HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_4,GPIO_PIN_SET); 
}

//外部中断初始化
void EXTI_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;
			
	__HAL_RCC_GPIOK_CLK_ENABLE();               	//开启GPIOI时钟

	GPIO_Initure.Pin=GPIO_PIN_0;                	//PK0
	GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      	//上升沿触发
	GPIO_Initure.Pull=GPIO_PULLDOWN;							//下拉
	HAL_GPIO_Init(GPIOK,&GPIO_Initure);

	HAL_NVIC_SetPriority(EXTI0_IRQn,4,0);       //抢占优先级为2，子优先级为0
	HAL_NVIC_EnableIRQ(EXTI0_IRQn);             //使能中断线0
} 

void EXTI0_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);  			//调用中断处理公用函数
}

//中断服务程序中需要做的事情
//在HAL库中所有的外部中断服务函数都会调用此函数
//GPIO_Pin:中断引脚号

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{	
//	printf("进入GPIO中断");
	if(g_IntMode==Transmit)	//进入发射模式
	{	
//		LED_Toggle;    新板子的发射状态灯是FPGA驱动的，发射信号时会灭掉
		QSPI_WriteBuffer(&g_uChar[g_DABit*g_DataBagLen*g_TransCnt],0,g_DataBagLen*g_DABit);
		
		if(g_TransCnt<g_FrameBlockNum-2)
		{
			g_TransCnt++;
		}
		else if(g_TransCnt==g_FrameBlockNum-2)
		{
			FPGA_TRANS_LOW;
			g_TransCnt++;
		}
		else if(g_TransCnt==g_FrameBlockNum-1)
		{
			g_IntMode = Receive;			
			g_TransCnt = 0;
			DIR_FLAG_LOW;
			FPGA_TRANS_HIGH;	
//			HAL_NVIC_EnableIRQ(SPI1_IRQn);
		}
	}
	if(g_IntMode==TransmitTD)	//进入发射模式
	{	
//		LED_Toggle;    新板子的发射状态灯是FPGA驱动的，发射信号时会灭掉
		QSPI_WriteBuffer(&g_uCharTD[g_DABit*g_DataBagLen*g_TransCnt],0,g_DataBagLen*g_DABit);
		
		if(g_TransCnt<15-2)
		{
			g_TransCnt++;
		}
		else if(g_TransCnt==15-2)
		{
			FPGA_TRANS_LOW;
			g_TransCnt++;
		}
		else if(g_TransCnt==15-1)
		{
			g_IntMode = Receive;			
			g_TransCnt = 0;
			DIR_FLAG_LOW;
			FPGA_TRANS_HIGH;	
//			HAL_NVIC_EnableIRQ(SPI1_IRQn);
		}
	}
}

/***************************** 声威海洋科技 (END OF FILE) *********************************/
