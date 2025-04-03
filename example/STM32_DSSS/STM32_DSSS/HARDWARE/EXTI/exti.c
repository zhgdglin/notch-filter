#include "system.h"


//外部中断初始化
void EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOI_CLK_ENABLE();               	//开启GPIOI时钟
    
    GPIO_Initure.Pin=GPIO_PIN_9;                	//PI9
    GPIO_Initure.Mode=GPIO_MODE_IT_RISING;      	//上升沿触发
    GPIO_Initure.Pull=GPIO_PULLDOWN;							//下拉
    HAL_GPIO_Init(GPIOI,&GPIO_Initure);
    
    HAL_NVIC_SetPriority(EXTI9_5_IRQn,4,0);       //抢占优先级为2，子优先级为0
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);             //使能中断线0 
	
}

void EXTI9_5_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);  				//调用中断处理公用函数
}

//中断服务程序中需要做的事情
//在HAL库中所有的外部中断服务函数都会调用此函数
//GPIO_Pin:中断引脚号

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{	
	if(g_IntMode==Receive)		//进入接收模式
	{
		LED_Toggle;
		QSPI_ReadBuffer(&g_CacheA[0],0,g_ReceiveBlock*4);
		memcpy(&g_CacheB[0],&g_CacheA[0],g_ReceiveBlock*4);
		S32ToF32(&g_CacheB[0],g_ReceiveBlock,&g_Cachef32[0]);
				
		if(g_1stSynIsOK==1)
		{
      if(g_RecvCnt<(g_ReceiveBlockNumber-1))
			{
				memcpy(&g_BackUp[g_ReceiveBlock*g_RecvCnt+g_ReceiveBlock*2],&g_Cachef32[0],g_ReceiveBlock*4);
				g_RecvCnt++;
			}
			else
			if(g_RecvCnt==(g_ReceiveBlockNumber-1))
			{ 
				memcpy(&g_BackUp[g_ReceiveBlock*g_RecvCnt+g_ReceiveBlock*2],&g_Cachef32[0],g_ReceiveBlock*4);
				memcpy(&g_BackUp[0],&g_SynBlk[0],2*g_ReceiveBlock*4);
				g_RecvCnt++;
				g_DecodeEN = 1;
//				Uart1_Puts(&SysAnswerCmd.SYN_ANSWER[0],9);				
			}
			else
			{}
		}
		else
		if(g_1stSynIsOK==0)
		{
      memcpy(&g_SynBlk[g_ReceiveBlock],&g_Cachef32[0],g_ReceiveBlock*4);
			memcpy(&g_SigBlk[g_Order],&g_Cachef32[0],g_ReceiveBlock*4);						//开始计时
      memcpy(&g_SigBlk[0],&g_BagTmpBlk[0],g_Order*4);
      DownConversion_SYN(&g_SigBlk[0],g_ReceiveBlock+g_Order,&g_SigMix[0]);
      memcpy(&g_BagBlk[2*g_ReceiveBlock],&g_SigMix[0],2*g_ReceiveBlock*4);
      DownSamplingCplx(&g_BagBlk[0],2*g_ReceiveBlock,g_DownSamplingNum,&g_SigBaseDown[0]);
      g_1stSynIsOK = Syn1st(&g_SigBaseDown[0],g_FFTLen,&g_1stCopy[0],&g_1stPoint);	//计时结束0.062秒     
    
      if(!g_1stSynIsOK)
      {       
        memcpy(&g_BagTmpBlk[0],&g_SigBlk[g_ReceiveBlock-2*g_Delay],g_Order*4);
        memmove(&g_BagBlk[0],&g_BagBlk[2*g_ReceiveBlock],2*g_ReceiveBlock*4);
        memmove(&g_SynBlk[0],&g_SynBlk[g_ReceiveBlock],g_ReceiveBlock*4);
      }
      else
      if(g_1stSynIsOK==1)
      {
        memset(&g_SigBlk[0],0,(g_ReceiveBlock+g_Order)*4);
        memset(&g_BagBlk[0],0,(4*g_ReceiveBlock)*4);
        memset(&g_SigBaseDown[0],0,2*g_FFTLen*4);
      }
		}
	}
	else
	if(g_IntMode==Transmit)	//进入发射模式
	{	
		LED_Toggle;
		QSPI_WriteBuffer(&uChar[2*g_TransmitBlock*g_TmitCnt],0,g_TransmitBlock*2);
		
		if(g_TmitCnt==g_TransmitBlockNumber-2)
		{
			g_TmitCnt++;
			FPGA_TRANS_LOW;
		}
		else
		if(g_TmitCnt==g_TransmitBlockNumber-1)
		{
			g_IntMode = Receive;
			g_TmitCnt = 0;
			DIR_FLAG_LOW;
			FPGA_TRANS_HIGH;
		}
		else
		{
			g_TmitCnt++;
		}
	}
  else    
  if(g_IntMode==Storage)	//进入存储模式
  {
    LED_Toggle;
    QSPI_ReadBuffer(&g_CacheA[0],0,g_ReceiveBlock*4);
		memcpy(&g_CacheB[0],&g_CacheA[0],g_ReceiveBlock*4);
    
    if(g_SaveCnt<14)
		{
			memcpy(&g_StoreDataBuf[g_SaveCnt*g_ReceiveBlock],&g_CacheB[0],g_ReceiveBlock*4);
			g_SaveCnt++;
		}
		else
    if(g_SaveCnt==14)
		{
      memcpy(&g_StoreDataBuf[g_SaveCnt*g_ReceiveBlock],&g_CacheB[0],g_ReceiveBlock*4);
      memcpy(&g_StoreDataBuf[0],&g_StoreDataBuf[0],g_ReceiveBlock*15*4);
			g_StorageDataOK  = 1;
			g_SaveCnt = 0;	
		}
  }
	else
	if(g_IntMode==Debug)
	{}
}



