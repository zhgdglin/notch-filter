
#include "bsp.h"			 /* 底层硬件驱动 */

/* 定义例程名和例程发布日期 */

extern void DemoFatFS(void);

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: c程序入口
*	形    参: 无
*	返 回 值: 错误代码(无需处理)
*********************************************************************************************************
*/
int main(void)
{
	bsp_Init();		/* 硬件初始化 */
	
	u32 pos;
//	u32 pos_buchang;
	volatile u32 SynLenRec;
	volatile u32 SynLenTx;
	u32 pos1;
	u32 Pos_Resamp;
	float frac;
	float *g_BackUpResample;
	float *data;
	float TD_Delay;
	u32 p;
	u32 q;
	u32 inputlen;
	u32 outputlen;

		
  while(1)
  {
    if(g_CommunicationMode==Transmit)
    {      
			if(g_UUVControlBegin == 1)
			{
//				Uart1_Puts(&SysAnswerCmd.DATA_TRANSMIT_START[0],8);
				DecodeLogSave(&SysAnswerCmd.DATA_TRANSMIT_START[0],8);
				//开启功放
				PowerAmplifierON();
				
				u8 *UratHex;
				UratHex = mymalloc(SRAMEX, (g_FrameByteNum+3)*sizeof(char));
				memset(UratHex, 0, (g_FrameByteNum+3)*sizeof(char));
				
				u16 *uShort;
				uShort = mymalloc(SRAMEX,g_SigSendLen*sizeof(short));//276k      
	      memset(&uShort[0],0,g_SigSendLen*sizeof(short));
				
				float *SigSend; //信号帧结构 [0.6*SigSynLFM Delay SigMod Delay 0.6*SigSynLFM Delay]  
				SigSend = mymalloc(SRAMEX, g_SigSendLen*sizeof(float));
				memset(&SigSend[0], 0, g_SigSendLen*sizeof(float));
				
				IWDG_Feed();//喂狗
				
				if(g_SendByteLen>g_FrameByteNum)
				{
					g_SendByteLen = g_FrameByteNum;
				}
				
				UratHex[0] = g_SendByteLen;
				memcpy(&UratHex[1],&g_RxBuff[0],g_SendByteLen*sizeof(char));
				UratHex[g_SendByteLen+1] = XORCaculate(UratHex,g_SendByteLen+1);	
        DecodeLogSave(&UratHex[0],g_SendByteLen+2);				
				ModulateMaSS(UratHex,g_SendByteLen+2,SigSend);
									
				for(u32 j=0; j<g_SigSendLen; j++)					
				{	
					uShort[j] = (int)(SigSend[j]*g_PlvLevel + 32767);	
				}
				memset(&g_uChar[0],0,g_SigSendLen*2*sizeof(char));
			  memcpy(g_uChar,uShort,g_SigSendLen*2*sizeof(char));
				memset(&SigSend[0],0,g_SigSendLen*4);	//测试，正常运行应该解除注释
				
				IWDG_Feed();//喂狗
				
				//将数据送至AD
				g_IntMode = Transmit;
				DIR_FLAG_HIGH;
		    FPGA_TRANS_HIGH;			
			  while(HAL_GPIO_ReadPin(GPIOK,GPIO_PIN_1)==1){};//等待传输结束标志
					
				HAL_Delay(500);    //1.防止自己接收发送信号  2.防止信号没有发完就关闭功放
				PowerAmplifierOFF();		//关闭功放

				myfree(SRAMEX,uShort);
			  myfree(SRAMEX,SigSend);
				myfree(SRAMEX,UratHex);
				memset(&g_RxBuff[0],0,(2048+128)*sizeof(char));	
		
			}
//			Uart1_Puts(&SysAnswerCmd.DATA_TRANSMIT_OVER[0],8);
			DecodeLogSave(&SysAnswerCmd.DATA_TRANSMIT_OVER[0],8);
			g_CommunicationMode = Receive;
			g_IntMode = Receive;
			bsp_spiTransfer();//既然是SPI中断中的函数为什么还要在此处专门写出

    }
    else if(g_CommunicationMode==Receive)
    {
			if(g_DecodeEN==1)
			{
				IWDG_Feed();//喂狗
//				Uart1_Puts(&SysAnswerCmd.SYN_ANSWER[0],9);	
				
//				if(g_Pose>g_BuchangCont)
//				{
//					pos_buchang = Syn2nd(&g_FrameSig[g_Pose-g_BuchangCont],g_BuchangCont*2+1);
//					pos = g_Pose+pos_buchang-g_BuchangCont;								
//				}else{
//					pos = g_Pose;
//				}
				pos = g_Pose;
				SynLenTx = 2*g_LFMLen;//同步头之间距离
									
				data = mymalloc(SRAMEX,3*g_ReceiveBlock*4);
				memset(&data[0],0,3*g_ReceiveBlock*4);			
				
				IWDG_Feed();//喂狗
				SynLenRec = 0;
				pos1 = pos+SynLenTx-5500;//开始做第二次同步的点位（第二个同步头往前6144点）
				memcpy(data,&g_FrameSig[pos1],3*g_ReceiveBlock*4);//实际运行程序需解除注释
				
				SynLenRec = DopplerEstimation(data);
				SynLenRec = SynLenRec+pos1-pos;//（估计长度）
				p = 10000;
				q = round((1.0*SynLenRec/SynLenTx)*10000);
				inputlen = g_SigSendLen+2*g_LFMLen;
				outputlen = round(1.0*(p-1)/(q-1)*inputlen);
				frac = 1.0*q/p;
				
				g_BackUpResample = mymalloc(SRAMEX,outputlen*4);
				memset(&g_BackUpResample[0],0,outputlen*4);
				Pos_Resamp = Resample(&g_FrameSig[0],inputlen,g_BackUpResample,outputlen,frac);//实际运行需解除注释
//					if(Pos_Resamp>g_BuchangCont)
//					{
//						pos_buchang = Syn2nd(&g_BackUpResample[Pos_Resamp-g_BuchangCont],g_BuchangCont*2+1);
//						Pos_Resamp = Pos_Resamp+pos_buchang-g_BuchangCont;								
//					}
//				if(Pos_Resamp == 0)
//				{
//					Pos_Resamp = pos;
//				}
				DemodulateMaSS(g_BackUpResample,Pos_Resamp);
				IWDG_Feed();//喂狗					
				myfree(SRAMEX,g_BackUpResample);
				myfree(SRAMEX,data);		
				
				g_DecodeEN = 0;
				g_CommunicationMode = Receive;
				g_IntMode = Receive;
				memset(&g_FrameSig[0],0,g_DataBagLen*(21+2)*sizeof(float));				
			}
    }
    else if(g_CommunicationMode==Storage)
    {
			g_StoreDataBuf = mymalloc(SRAMIN,g_StoreBlockNum*g_ReceiveBlock*sizeof(int));
	        memset(&g_StoreDataBuf[0], 0, g_StoreBlockNum*g_ReceiveBlock*sizeof(int));
//		    printf("\nSD Save Data Init");
//		    g_IntMode=Storage;
            DataSave();
		    myfree(SRAMIN,g_StoreDataBuf);
			g_CommunicationMode = Receive;
			g_IntMode=Receive;
			Uart1_Puts(SysAnswerCmd.DATA_SAVE_ANSWER,9);
    }
		else if(g_CommunicationMode == TD)
		{
//			PowerSet(0x34);
			IWDG_Feed();//喂狗

//			memset(&g_SigSend[0],0,(g_SigLen+10*g_ReceiveBlock)*4);
//			if(g_TDSuccess == 1)
//			{
//				TD_Delay = 1.0*g_TDPose*1000/96;
//				bsp_DelayUS((u32)TD_Delay);
//				u32 distance;
//				HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeTD1,RTC_FORMAT_BIN);
//				HAL_RTC_GetDate(&RTC_Handler,&RTC_DateTD1,RTC_FORMAT_BIN);
//				u16 g_TDTimeEndSubS = RTC_TimeTD1.SubSeconds;
//				u8 g_TDTimeEndS = RTC_TimeTD1.Seconds;
////				SysAnswerCmd.TD_MASTER_ANSWER[6] = g_TDTimeEndSubS/256;
////				SysAnswerCmd.TD_MASTER_ANSWER[7] = g_TDTimeEndSubS%256;
////				SysAnswerCmd.TD_MASTER_ANSWER[5] = g_TDTimeEndS;
////				Uart1_Puts(SysAnswerCmd.TD_MASTER_ANSWER,10);
//				u32 TDTime;
//				if(g_TDTime == 1)
//				{
//					TDTime = 8192*g_TDTimeEndS+0x1FFF-g_TDTimeEndSubS;//-0x5F0C;//-0x33F5;//-0x3420;//-0x19f5;
////						distance = (0.122*TDTime*g_VSound/100/2);  //(?à?ê¨￠?1??D?¨oy|ì?)1/8192*1000=0.122
//					SysAnswerCmd.TD_MASTER_ANSWER[6] = TDTime/256;
//					SysAnswerCmd.TD_MASTER_ANSWER[7] = TDTime%256;
//				}
//				else
//				{
//					TDTime = 8192*g_TDTimeEndS+0x1FFF-g_TDTimeEndSubS-g_BuChang;//-0x33F5;//-0x3420;//-0x19f5;
//					distance = (0.122*TDTime*g_VSound/100/2);  //(?à?ê¨￠?1??D?¨oy|ì?)1/8192*1000=0.122
//					SysAnswerCmd.TD_MASTER_ANSWER[6] = distance/256;
//					SysAnswerCmd.TD_MASTER_ANSWER[7] = distance%256;						
//				}
//				g_TDMaster = 0;
//				g_TDSlave = 0;
//				g_TDSuccess = 0;
//				Uart1_Puts(SysAnswerCmd.TD_MASTER_ANSWER,10);
//				HAL_Delay(1000);
//			}
			if(g_TDMaster!=g_TDSlave)
			{
				if(g_TDSlave == 1)
				{
					TD_Delay = 1.0*g_TDPose*1000/96;
					bsp_DelayUS((u32)TD_Delay);
				}
				
				//开启功放
				PowerAmplifierON();
//				HAL_Delay(500);				

				//数据类型转换			
				u8 LFMQianNum = 10;   //64*4ms
//				u8 LFMHouNUm = 4;    //64*4ms
				memcpy(&g_Sig[g_ReceiveBlock*LFMQianNum],g_LFM,g_ReceiveBlock*4);

//				if(g_TDSlaveNum == 1)
//				{
//					memcpy(&g_SigSend[g_ReceiveBlock*LFMQianNum],g_SynLFM,g_ReceiveBlock*4);
//				}
//				if(g_TDSlaveNum == 2)
//				{
//					memcpy(&g_Sig[g_ReceiveBlock*LFMQianNum],g_SynLFMDown,g_ReceiveBlock*4);
//				}
//				g_SigLenTem = g_ReceiveBlock*(LFMQianNum+1+LFMHouNUm);
				
				for(u32 j=0; j<6144*15; j++)					
				{	
					g_uShortTD[j] = (int)(g_Sig[j]*g_PlvLevel + 32767);	
				}
//				memset(&g_uChar[0],0,g_SigSendLen*2*sizeof(char));
			  memcpy(g_uCharTD,g_uShortTD,6144*15*2*sizeof(char));
//				memset(&g_SigSend[0],0,g_SigSendLen*4);	//测试，正常运行应该解除注释
				
//				if(g_TDMaster == 1)
//				{
//					RTC_Set_Time(12,0,0,RTC_HOURFORMAT12_AM);
//				}
				
				//将数据送至AD				
				IWDG_Feed();//喂狗
				
				//将数据送至AD
				g_IntMode = TransmitTD;
				DIR_FLAG_HIGH;
		    FPGA_TRANS_HIGH;			
			  while(HAL_GPIO_ReadPin(GPIOK,GPIO_PIN_1)==1){};//等待传输结束标志
					
				HAL_Delay(200);    //1.防止自己接收发送信号  2.防止信号没有发完就关闭功放
				PowerAmplifierOFF();		//关闭功放
//				if(g_TDMaster == 1)
//				{
//					 Uart1_Puts(SysAnswerCmd.TD_MASTER_SEND_ANSWER,8);
//				}                             
//				
//				PowerAmplifierOFF();		//关闭功放
				if(g_TDSlave == 1)
				{
//					if(g_TDSlaveNum == 1)
//					{
//						SysAnswerCmd.TD_SLAVE_ANSWER[4] = 0x12;
//					}
//					else if(g_TDSlaveNum == 2)
//					{
//						SysAnswerCmd.TD_SLAVE_ANSWER[4] = 0x22;
//					}
//					else{}
					Uart1_Puts(&SysAnswerCmd.TD_SLAVE_ANSWER[0],8);
//					DecodeLogSave(&SysAnswerCmd.TD_SLAVE_ANSWER[0],8);
					HAL_Delay(200);
				}
				g_TDSlave = 0;
			}
			g_CommunicationMode = Receive;
			g_IntMode = Receive;

//			PowerSet(0x32);			
		}
		else
		{}
			
		if(g_DeleteData == 1)
		{
			DataSaveDelete();
			Uart1_Puts(SysAnswerCmd.DATA_SAVE_DELETE_ANSWER,9);
			g_DeleteData = 0;
		}
		if(g_ReadLog == 1)
		{
			DecodeLogRead();
			g_ReadLog = 0;
		}
		if(g_DeleteLog == 1)
		{
			DecodeLogDelete();
			Uart1_Puts(SysAnswerCmd.DECODE_LOG_DELETE_ANSWER,9);
			g_DeleteLog = 0;
		}
  }
}

/***************************** 声威海洋科技 www.armfly.com (END OF FILE) *********************************/
