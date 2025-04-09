#include "system.h"

void UART_CMD_ANALYSE()
{	
	u8 	*data;
	data = mymalloc(SRAMEX,USART_REC_LEN);
	memset(&data[0],0,USART_REC_LEN);
	memcpy(&data[0],&USART_RX_BUF[0],g_UartHEXLen);
	memset(&USART_RX_BUF[0],0,g_UartHEXLen);
	
	if(data[4] == Pass)	//透传指令
	{	
		if((data[5] == g_UartHEXLen-8)&&(data[5] < 29))	
		{
			g_CommunicationMode = Transmit;
			memcpy(&g_UartHEX[0],&data[5],g_UartHEXLen-7);
			g_UartHEX[g_UartHEXLen-7] = XORCaculate(&data[5],g_UartHEXLen-7);
//			g_UartHEX[g_UartHEXLen-6] = CheckSum(&g_UartHEX[0],g_UartHEXLen-6);
			g_SendByteLen = data[5];
		}
		else
		{
			Uart1_Puts(SysAnswerCmd.DATA_LEN_ERROR,8);
		}
	}
	else
	if(data[4] == Ctrl)	//控制指令
	{
		if(memcmp(&data[5],SysCmd.POWR_SET,1)==0)		//功率设置
		{
			PowerSet(data[6]);
		}
		else
		if(memcmp(&data[5],SysCmd.PAMP_SET,1)==0)		      //前放设置
		{
			PreAmpSet(data[6]);
		}
		else
		if(memcmp(&data[5],SysCmd.POWR_CHECK,1)==0)     	//功率查询
		{
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);
		}
		else
		if(memcmp(&data[5],SysCmd.PAMP_CHECK,1)==0)	      //前放查询
		{
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
		}
		else
		if(memcmp(&data[5],SysCmd.DECODE_LOG_READ,1)==0)	//解码日志查询
		{
			DecodeLogRead();
		}
	  else
    if(memcmp(&data[5],SysCmd.DECODE_LOG_DELETE,1)==0)//解码日志删除
    {  
      DecodeLogDelete();
			Uart1_Puts(SysAnswerCmd.DECODE_LOG_DELETE_ANSWER,9);
    }
		else
		if(memcmp(&data[5],SysCmd.RTC_SET,1)==0)		      //RTC设置
		{
			for(char i=6;i<12;i++)
			{
				SysAnswerCmd.RTC_ANSWER[i] = data[i];
			}
			RTC_Set_Time(SysAnswerCmd.RTC_ANSWER[9],SysAnswerCmd.RTC_ANSWER[10],SysAnswerCmd.RTC_ANSWER[11],RTC_HOURFORMAT12_AM);
			RTC_Set_Date(SysAnswerCmd.RTC_ANSWER[6],SysAnswerCmd.RTC_ANSWER[7],SysAnswerCmd.RTC_ANSWER[8],6);		
			Uart1_Puts(SysAnswerCmd.RTC_ANSWER,14);
		}	
    else
    if(memcmp(&data[5],SysCmd.DATA_SAVE,1)==0)        //数据存储
    { 
			g_IntMode = Storage;
      g_CommunicationMode = Storage;
    }
		else
    if(memcmp(&data[5],SysCmd.DATA_SAVE_DELETE,1)==0) //数据存储删除
    {  
      DataSaveDelete();
			Uart1_Puts(SysAnswerCmd.DATA_SAVE_DELETE_ANSWER,9);
    }
//		else
//    if(memcmp(&data[5],SysCmd.PARA_SET,1)==0) //参数设置
//    {  
//      g_fl = data[6]*1000;
//			g_fh = data[7]*1000;
//			g_M1 = data[8]-32;
//			g_M2 = data[9]-32;
//			g_InfoSymbolNumPerFrame = data[10]-32;
//			g_T = data[11]/1000.0;
//			ParameterFree();
//			ParameterInit();
//			for(u8 i=6;i<12;i++)
//			{
//				SysAnswerCmd.PARA_ANSWER[i] = data[i];
//			}
//			Uart1_Puts(SysAnswerCmd.PARA_ANSWER,14);
//    }
		else
		if(memcmp(&data[5],SysCmd.PARA_CHECK,1)==0)	//参数查询
		{
			Uart1_Puts(SysAnswerCmd.PARA_ANSWER,14);
		}
		else
		if(memcmp(&data[5],SysCmd.SAVE_SET,1)==0)	//存储设置
		{
			g_save = data[6];
			SysAnswerCmd.SAVE_SET_ANSWER[6] = data[6];
			Uart1_Puts(SysAnswerCmd.SAVE_SET_ANSWER,9);
		}
		else
		if(memcmp(&data[5],SysCmd.POWER_SET,1)==0)	//功率设置
		{
			g_PlvLevel = data[6]*16*16+data[7];
			Uart1_Puts(SysAnswerCmd.POWER_SET_ANSWER,9);
		}
		else
		if(memcmp(&data[5],SysCmd.DOPPLER_SET,1)==0)	//多普勒设置
		{
			g_DopplerEstimation = data[6];
			SysAnswerCmd.DOPPLER_SET_ANSWER[6] = data[6];
			Uart1_Puts(SysAnswerCmd.DOPPLER_SET_ANSWER,9);
		}
	}	
	myfree(SRAMEX,data);
}

void PowerSet(u8 Level)
{
	switch(Level)
	{
		case 0x31://100mVpp
		{
			g_PlvLevel = g_PlvBase/2;
			SysAnswerCmd.POWR_ANSWER[6] = 0x31;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);
			break;
		}
		case 0x32://0.933Vpp
		{
			g_PlvLevel = g_PlvBase*5;
			SysAnswerCmd.POWR_ANSWER[6] = 0x32;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
		case 0x33://1.95Vpp
		{
			g_PlvLevel = g_PlvBase*10;
			SysAnswerCmd.POWR_ANSWER[6] = 0x33;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);
			break;
		}			
		case 0x34://3.0Vpp
		{
			g_PlvLevel = g_PlvBase*15;
			SysAnswerCmd.POWR_ANSWER[6] = 0x34;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
		case 0x35://4.0Vpp
		{
			g_PlvLevel = g_PlvBase*20;
			SysAnswerCmd.POWR_ANSWER[6] = 0x35;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
		case 0x36://5.7Vpp
		{
			g_PlvLevel = g_PlvBase*30;
			SysAnswerCmd.POWR_ANSWER[6] = 0x36;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
		case 0x37://7.447Vpp
		{
			g_PlvLevel = g_PlvBase*40;
			SysAnswerCmd.POWR_ANSWER[6] = 0x37;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
		case 0x38://9.48Vpp
		{
			g_PlvLevel = g_PlvBase*50;
			SysAnswerCmd.POWR_ANSWER[6] = 0x38;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
		default:
		{
			g_PlvLevel = g_PlvBase*5;
			SysAnswerCmd.POWR_ANSWER[6] = 0x32;
			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
			break;
		}
	}
}

void PreAmpSet(u8 Level)
{
	switch(Level)
	{
		case 0x31 ://1G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);   //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);	  //G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);	//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);	  //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);	  //G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x31;
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x32://4G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET); //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);	//G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);	  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);	//G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET); //G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x32;			
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x33://16G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET); //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);	  //G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);	  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);	//G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);	  //G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x33;			
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x34://64G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);   //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);	//G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);  	//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);	  //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);	//G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x34;			
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x35://128G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);   //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);	//G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);	  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);	  //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);	  //G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x35;		
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x36://256G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);   //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);	  //G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_SET);	  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);	  //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);	  //G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x36;			
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x37://512G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_SET);  //G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);   //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);	  //G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);	  //G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);	//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);	//G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET); //G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x37;		
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		case 0x38://1024G
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_SET);  //G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_RESET); //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_RESET);	//G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);	  //G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);	//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_RESET);	//G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_RESET);	//G0
			SysAnswerCmd.PAMP_ANSWER[6] = 0x38;			
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
		default:
		{
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_RESET);	//G4
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_14,GPIO_PIN_RESET);//G3
      HAL_GPIO_WritePin(GPIOG,GPIO_PIN_13,GPIO_PIN_RESET);//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_1,GPIO_PIN_SET);   //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_1,GPIO_PIN_SET);	  //G0
      
      //二级放大
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_11,GPIO_PIN_RESET);//G4
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);	//G3
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_4,GPIO_PIN_RESET);	//G2
      HAL_GPIO_WritePin(GPIOC,GPIO_PIN_5,GPIO_PIN_SET);	  //G1
      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_2,GPIO_PIN_SET);	  //G0
      
			SysAnswerCmd.PAMP_ANSWER[6] = 0x31;
			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
			break;
		}
	}
}
