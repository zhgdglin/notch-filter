#include "bsp.h"

uint8_t Boot[4] = {'B','o','o','t'};
uint8_t Send[4] = {'S','e','n','d'};
uint8_t CSend[5] = {'C','S','e','n','d'};
uint8_t PowerCtrl[9] = {'P','o','w','e','r','C','t','r','l'};
uint8_t PreAmpCtrl[10] = {'P','r','e','A','m','p','C','t','r','l'};
uint8_t Save[4] = {'S','a','v','e'};
uint8_t ParaSet[7] = {'P','a','r','a','S','e','t'};
//0X31-0X38对应ASCII码1-8
char Name[3] = {0};

void CmdPickUp1(u8 *Data,u16 Len)//Len = RxNum
{		
	u8 	*data;
	data = mymalloc(SRAMIN,USART_REC_LEN);
	memset(&data[0],0,USART_REC_LEN);
	memcpy(&data[0],&Data[0],Len);
		
	if((memcmp(data,STM32UartProtocol.Preamble,4)==0)&&(memcmp(&data[Len-2],STM32UartProtocol.End_flag,2)==0))
	{
			/********************透传模式***********************/
			if(data[4] == Pass)//Pass = 0X01
			{
				g_SendByteLen = data[5]*16*16 + data[6];     //两个字节表示长度，大端在前
				if(g_SendByteLen == Len-9)
				{
					memcpy(&g_RxBuff[0],&data[7],g_SendByteLen);	// A5 A5 A5 D5 01 00 B8 信息byte A5 D4	
				    g_CommunicationMode = Transmit;
					g_TransCnt = 0;					
				}
				else
				{
					Uart1_Puts(SysAnswerCmd.DATA_LEN_ERROR,8);
				}				
			}
		    /********************控制模式***********************/
			else if(data[4] == Ctrl)//Ctrl =0X00
			{
				if(memcmp(&data[5],SysCmd.POWR_SET,1)==0)		//功率设置
				{
					PowerSet(data[6]);
					Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
				}
				else
				if(memcmp(&data[5],SysCmd.PAMP_SET,1)==0)		      //前放设置
				{
					PreAmpSet(data[6]);
					Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);	
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
					g_ReadLog = 1;
				}
				else
				if(memcmp(&data[5],SysCmd.DECODE_LOG_DELETE,1)==0)//解码日志删除
				{  
					g_DeleteLog = 1;
				}
				else
				if(memcmp(&data[5],SysCmd.RTC_SET,1)==0)		      //RTC设置
				{
					for(char i=6;i<13;i++)
					{
						SysAnswerCmd.RTC_ANSWER[i] = data[i];
					}
					RTC_Set_Time(SysAnswerCmd.RTC_ANSWER[10],SysAnswerCmd.RTC_ANSWER[11],SysAnswerCmd.RTC_ANSWER[12],RTC_HOURFORMAT12_AM);
					RTC_Set_Date(24,SysAnswerCmd.RTC_ANSWER[8],SysAnswerCmd.RTC_ANSWER[9],6);		
					Uart1_Puts(SysAnswerCmd.RTC_ANSWER,15);
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
					g_DeleteData = 1;
				}
				else
				if(memcmp(&data[5],SysCmd.PARA_CHECK,1)==0)	//参数查询
				{
					Uart1_Puts(SysAnswerCmd.PARA_ANSWER,14);
				}
				//以下功能暂未开放
				else
				if(memcmp(&data[5],SysCmd.SYS_SET,1)==0)	//模式设置
				{
//					ParameterFree();			
//					g_SysMode = data[6];
//					ParameterInitChange();
//					SysAnswerCmd.SYS_SET_ANSWER[6] = data[6];
//					RS485SendData(SysAnswerCmd.SYS_SET_ANSWER,9);
				}
				else
				if(memcmp(&data[5],SysCmd.FRE_SET,1)==0)	//声源级测试频点设置
				{
					g_f = data[6];
					SysAnswerCmd.FRE_SET_ANSWER[6] = data[6];
					Uart1_Puts(SysAnswerCmd.FRE_SET_ANSWER,9);
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
					memset(&g_Sig[0],0,g_SigLen*4);
					g_DopplerEstimation = data[6];
					SysAnswerCmd.DOPPLER_SET_ANSWER[6] = data[6];
					Uart1_Puts(SysAnswerCmd.DOPPLER_SET_ANSWER,9);
				}
				else
				if(memcmp(&data[5],SysCmd.TD,1)==0)	//测距
				{
					g_CommunicationMode = TD;
					g_TDMaster = 1;
					g_TDSlave = 0;
				}
				else
				if(memcmp(&data[5],SysCmd.UUV_CONTROL_BEGIN,1)==0)	//UUV入水后控制可以发射通信信号
				{
					g_UUVControlBegin = data[6];
					SysAnswerCmd.UUV_CONTROL_BEGIN_ANSWER[6] = data[6];
					Uart1_Puts(SysAnswerCmd.UUV_CONTROL_BEGIN_ANSWER,9);
				}
//				else
//				if(memcmp(&data[5],SysCmd.VSOUND_SET,1)==0)	//UUV入水后控制可以发射通信信号
//				{
//					g_VSound = data[6]*16*16+data[7];
//					Uart1_Puts(SysAnswerCmd.VSOUND_SET_ANSWER,8);
//				}
				else{}
			}		
	}
	
	if((data[0] == 0xFA)&&(data[Len-2] == 0xEF)&&(data[Len-1] == 0x0A))
	{
		g_SendByteLen = Len-3;  
		memcpy(&g_RxBuff[0],&data[1],g_SendByteLen);	
		g_CommunicationMode = Transmit;
		g_TransCnt = 0;	
	}
		
		RxNum = 0;
		myfree(SRAMIN , data);
}



void CmdPickUp8(u8 *Data,u16 Len)
{
	
}

void PowerSet(u8 Level)
{
	switch(Level)
	{
		case 0x30:
		{
			g_PlvLevel = g_PlvBase/200;
//			printf("\nPowerSet:0X29");
			break;
		}
		case 0x31://mVpp
		{
			g_PlvLevel = g_PlvBase/10;
//			g_PlvLevel = g_PlvBase/800;
			SysAnswerCmd.POWR_ANSWER[6] = 0x31;
//			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);
//			printf("\nPowerSet:0X30");
			break;
		}
		case 0x32://mVpp
		{
			g_PlvLevel = g_PlvBase;
			SysAnswerCmd.POWR_ANSWER[6] = 0x32;
//			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);
//			printf("\nPowerSet:0X31");
			break;
		}
		case 0x33://Vpp   DA输出170mVpp
		{
			g_PlvLevel = g_PlvBase*13;
			SysAnswerCmd.POWR_ANSWER[6] = 0x33;
//			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
//			printf("\nPowerSet:0X32");
			break;
		}
//		case 0x33://Vpp   DA输出500mVpp
//		{
//			g_PlvLevel = g_PlvBase;
//			SysAnswerCmd.POWR_ANSWER[6] = 0x33;
////			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);
////			printf("\nPowerSet:0X33");
//			break;
//		}			
//		case 0x34://Vpp    DA输出1.9Vpp
//		{
//			g_PlvLevel = g_PlvBase*5;
//			SysAnswerCmd.POWR_ANSWER[6] = 0x34;
////			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
////			printf("\nPowerSet:0X34");
//			break;
//		}
//		case 0x35://Vpp    DA输出3.8Vpp
//		{
//			g_PlvLevel = g_PlvBase*10;
//			SysAnswerCmd.POWR_ANSWER[6] = 0x35;
////			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
////			printf("\nPowerSet:0X35");
//			break;
//		}
//		case 0x36://Vpp     DA输出5Vpp
//		{
//			g_PlvLevel = g_PlvBase*13;
//			SysAnswerCmd.POWR_ANSWER[6] = 0x36;
////			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
////			printf("\nPowerSet:0X36");
//			break;
//		}
//		case 0x37://Vpp
//		{
//			g_PlvLevel = g_PlvBase*16;
////			g_PlvLevel = g_PlvBase*20;
//			SysAnswerCmd.POWR_ANSWER[6] = 0x37;
////			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
//			break;
//		}
//		case 0x38://Vpp
//		{
//			g_PlvLevel = g_PlvBase*50;
//			SysAnswerCmd.POWR_ANSWER[6] = 0x38;
////			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
//			break;
//		}
		default:
		{
			g_PlvLevel = g_PlvBase*13;//原来乘13是6档 此处改为4档对应基数改为5
			SysAnswerCmd.POWR_ANSWER[6] = 0x33; 
//			Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
//			printf("\nPowerSet:0X34");
			break;
		}
	}
}

void PreAmpSet(u8 Level)
{
	switch(Level)
	{
		case 0x30 ://0.125G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x30;
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X30");
			break;
		}
		case 0x31 ://0.25G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x31;
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X31");
			break;
		}
		case 0x32://0.5G   0.5G   此处G设为固定增益放大倍数
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x32;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X32");
			break;
		}
		case 0x33://1G     0.67G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x33;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X33");
			break;
		}
		case 0x34://2G     0.93G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x34;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X34");
			break;
		}
		case 0x35://4G     1.6G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x35;		
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X35");
			break;
		}
		case 0x36://8G     2.8G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x36;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X36");
			break;
		}
		case 0x37://16G    
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x37;		
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X37");
			break;
		}
		case 0x38://32G    
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x38;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X38");
			break;
		}
		case 0x39://64G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x38;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X39");
			break;
		}
		case 0x40://128G
		{
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_11,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_12,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_13,GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_14,GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIOJ,GPIO_PIN_15,GPIO_PIN_RESET);
			SysAnswerCmd.PAMP_ANSWER[6] = 0x38;			
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X40");
			break;
		}
		default:
		{   
			SysAnswerCmd.PAMP_ANSWER[6] = 0x31;
//			Uart1_Puts(SysAnswerCmd.PAMP_ANSWER,9);
//			printf("\nPreAmpSet:0X31");
			break;
		}
	}
}
