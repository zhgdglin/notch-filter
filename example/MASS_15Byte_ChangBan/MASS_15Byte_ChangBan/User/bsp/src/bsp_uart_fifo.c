/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪@sunyubo
Version: 	     // V.2
Date: 	       // 2023/11/23
Description:   // 长条板通信机驱动
History:       // V.1
*******************************************************************/

#include "bsp.h"

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

UART_HandleTypeDef huart8;
DMA_HandleTypeDef hdma_uart8_rx;
DMA_HandleTypeDef hdma_uart8_tx;

__align(32) __attribute__((at(0x30000000))) uint8_t RxBuff[USART_REC_LEN];

__IO uint16_t RxNum = 0;
__IO uint8_t g_UartIsOK = 0;
__IO uint8_t g_UartIsOK8 = 0;

//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((USART1->ISR&0x40)==0);//循环发送,直到发送完毕   
	USART1->TDR=(uint8_t)ch;      
	return ch;
}

/* USART1 & UART8 init function */

void bsp_UartInit(uint32_t Bound)
{ 
  if(1)
  {
    huart1.Instance                    = USART1;
    huart1.Init.BaudRate               = 115200;
    huart1.Init.WordLength             = UART_WORDLENGTH_8B;
    huart1.Init.StopBits               = UART_STOPBITS_1;
    huart1.Init.Parity                 = UART_PARITY_NONE;
    huart1.Init.Mode                   = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    __HAL_UART_ENABLE_IT(&huart1,UART_IT_IDLE);
  }
  
  if(1)
  {
    huart8.Instance                    = UART8;
    huart8.Init.BaudRate               = 115200;
    huart8.Init.WordLength             = UART_WORDLENGTH_8B;
    huart8.Init.StopBits               = UART_STOPBITS_1;
    huart8.Init.Parity                 = UART_PARITY_NONE;
    huart8.Init.Mode                   = UART_MODE_TX_RX;
    huart8.Init.HwFlowCtl              = UART_HWCONTROL_NONE;
    huart8.Init.OverSampling           = UART_OVERSAMPLING_16;
    huart8.Init.OneBitSampling         = UART_ONE_BIT_SAMPLE_DISABLE;
    huart8.Init.ClockPrescaler         = UART_PRESCALER_DIV1;
    huart8.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    
    if (HAL_UART_Init(&huart8) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    if (HAL_UARTEx_SetTxFifoThreshold(&huart8, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    if (HAL_UARTEx_SetRxFifoThreshold(&huart8, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    if (HAL_UARTEx_DisableFifoMode(&huart8) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    __HAL_UART_ENABLE_IT(&huart8,UART_IT_IDLE);
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  
  if(uartHandle->Instance==USART1)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2;
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART1 DMA Init */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    /* USART1_RX Init */
    hdma_usart1_rx.Instance = DMA1_Stream0;
    hdma_usart1_rx.Init.Request = DMA_REQUEST_USART1_RX;
    hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_rx.Init.Mode = DMA_CIRCULAR;
    hdma_usart1_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_usart1_rx);

    /* USART1_TX Init */
    hdma_usart1_tx.Instance = DMA1_Stream1;
    hdma_usart1_tx.Init.Request = DMA_REQUEST_USART1_TX;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_usart1_tx);

    /* DMA1_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 1, 0);
//	HAL_NVIC_SetPriority(DMA1_Stream0_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    /* DMA1_Stream1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
//	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
    
    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);
//	HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  else if(uartHandle->Instance==UART8)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART8;
    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
    
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }
    
    /* UART8 clock enable */
    __HAL_RCC_UART8_CLK_ENABLE();
    __HAL_RCC_GPIOJ_CLK_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
    HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);
    
    /* UART8 DMA Init */
    __HAL_RCC_DMA1_CLK_ENABLE();
    
    /* UART8_RX Init */
    hdma_uart8_rx.Instance = DMA1_Stream2;
    hdma_uart8_rx.Init.Request = DMA_REQUEST_UART8_RX;
    hdma_uart8_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart8_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart8_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart8_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart8_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart8_rx.Init.Mode = DMA_CIRCULAR;
    hdma_uart8_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    hdma_uart8_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    if (HAL_DMA_Init(&hdma_uart8_rx) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmarx,hdma_uart8_rx);

    /* USART1_TX Init */
    hdma_uart8_tx.Instance = DMA1_Stream3;
    hdma_uart8_tx.Init.Request = DMA_REQUEST_UART8_TX;
    hdma_uart8_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart8_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart8_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart8_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart8_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart8_tx.Init.Mode = DMA_NORMAL;
    hdma_uart8_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_uart8_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    
    if (HAL_DMA_Init(&hdma_uart8_tx) != HAL_OK)
    {
      Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(uartHandle,hdmatx,hdma_uart8_tx);

    /* DMA1_Stream0_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);

    /* DMA1_Stream1_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
    
    /* USART1 interrupt Init */
    HAL_NVIC_SetPriority(UART8_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(UART8_IRQn);
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{
  if(uartHandle->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_9);

    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);
    
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }
  else if(uartHandle->Instance==UART8)
  {
    __HAL_RCC_UART8_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOJ, GPIO_PIN_8|GPIO_PIN_9);

    HAL_DMA_DeInit(uartHandle->hdmarx);
    HAL_DMA_DeInit(uartHandle->hdmatx);
    
    HAL_NVIC_DisableIRQ(UART8_IRQn);
  }
}

void DMA1_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
}

void DMA1_Stream1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
}

void DMA1_Stream2_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart8_rx);
}

void DMA1_Stream3_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_uart8_tx);
}

void USART1_IRQHandler(void)
{
  if(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_IDLE)==SET)
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    
    HAL_UART_DMAStop(&huart1);
    RxNum = USART_REC_LEN - __HAL_DMA_GET_COUNTER(&hdma_usart1_rx);
    CmdPickUp1(RxBuff,RxNum);  //用户程序
    
    memset(RxBuff,0,USART_REC_LEN);
    HAL_UART_Receive_DMA(&huart1,RxBuff,USART_REC_LEN);
  }
  
  HAL_UART_IRQHandler(&huart1);
}

void UART8_IRQHandler(void)
{
  if(__HAL_UART_GET_FLAG(&huart8,UART_FLAG_IDLE)==SET)
  {
    __HAL_UART_CLEAR_IDLEFLAG(&huart8);
    
    HAL_UART_DMAStop(&huart8);
    RxNum = USART_REC_LEN - __HAL_DMA_GET_COUNTER(&hdma_uart8_rx);
    
//    CmdPickUp8(RxBuff,RxNum);  //用户程序
    
    memset(RxBuff,0,USART_REC_LEN);
    HAL_UART_Receive_DMA(&huart8,RxBuff,USART_REC_LEN);
  }
  
  HAL_UART_IRQHandler(&huart8);
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

void Uart8_Puts(u8 *data,u32 data_len)
{
	u32 i;
	for(i=0;i<data_len;i++)
	{
		UART8->TDR=data[i];
		while((UART8->ISR&0x40)==0);//等待发送结束
	}
}

/***************************** 会上树的猪 (END OF FILE) *********************************/
