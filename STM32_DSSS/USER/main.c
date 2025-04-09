/************************************************
 STM32H7 DSSS程序
 HAL库函数版
 技术支持：Jason Lee
 版本：V2.1
 算法：门伟
 作者：会上树的猪@sunyubo
 时间：2022/07/07@2023/03/21
 地点：哈工程青岛科技园B1
************************************************/

#ifndef __MAIN_H
#define __MAIN_H
/*main函数*/

#include "system.h"

int main()
{
	System_Init();
  
	//SD卡初始化
	exfuns_init();
	f_mount(fs[0],"0:",1);
	
//	u32 ReadDataLen = g_ReceiveBlock*(g_ReceiveBlockNumber+2);
//	char *SigRECV;
//	SigRECV = mymalloc(SRAMEX,ReadDataLen*11);
//	memset(&SigRECV[0],0,ReadDataLen*11);
//	char *Tmp;
//	Tmp = mymalloc(SRAMEX,9);
//	memset(&Tmp[0],0,9);
//	float *Test2;
//	Test2 = mymalloc(SRAMEX,ReadDataLen*4);
//	memset(&Test2[0],0,ReadDataLen*4);
 
	while(1)
	{		
		if(g_CommunicationMode == Transmit)
		{      
			//开启功放
			POWER_AMP_ON_ZZ_NEW();
			
			//调制
			float *g_SigDSSS;
			g_SigDSSS = mymalloc(SRAMEX,g_TransmitBlockNumber*g_TransmitBlock*4);
			memset(&g_SigDSSS[0],0,g_TransmitBlockNumber*g_TransmitBlock*4);
			DSSSModulate(g_UartHEX,g_SendByteLen+2,&g_SigDSSS[0]);

			//数据类型转换
			u16 *uShort;
			uShort = mymalloc(SRAMEX,g_TransmitBlockNumber*g_TransmitBlock*2);      
			memset(&uShort[0],0,g_TransmitBlockNumber*g_TransmitBlock*2);
			
			uChar = mymalloc(SRAMEX,g_TransmitBlockNumber*g_TransmitBlock*2);      
			memset(&uChar[0],0,g_TransmitBlockNumber*g_TransmitBlock*2);
			for(u32 i=0;i<g_TransmitBlockNumber*g_TransmitBlock;i++)
			{
				uShort[i] = (int)(g_SigDSSS[i]*g_PlvLevel + 32767);
			}
			memcpy(uChar,uShort,g_TransmitBlockNumber*g_TransmitBlock*2);
			
			//FPGA与STM32总线驱动
			g_IntMode = Transmit;
			DIR_FLAG_HIGH;
			FPGA_TRANS_HIGH;			
			while(HAL_GPIO_ReadPin(GPIOI,GPIO_PIN_10)==1){};//等待传输结束标志			
			
			//关闭功放
			POWER_AMP_OFF_ZZ_NEW();
			
			//切换回接收模式
			g_CommunicationMode = Receive;
			g_IntMode = Receive;
			myfree(SRAMEX,g_SigDSSS);
			myfree(SRAMEX,uShort);
			myfree(SRAMEX,uChar);
		}
		else if(g_CommunicationMode == Receive)
		{ 
			if(g_DecodeEN==1)
			{					
				u32 pos = g_1stPoint+g_ReceiveBlock*4-g_Delay;
				if(g_DopplerEstimation == 1)
				{
					u32 SynLenRec = 0;
					u32 SynLenTx = g_ReceiveBlock*2;
					u32 pos1 = g_1stPoint-g_Delay+g_ReceiveBlock+6000;
					float *data;
					data = mymalloc(SRAMEX,3*g_ReceiveBlock*4);
					memset(&data[0],0,3*g_ReceiveBlock*4);
					memcpy(data,&g_BackUp[pos1],3*g_ReceiveBlock*4);
					SynLenRec = DopplerEstimation(data);
					myfree(SRAMEX,data);
					
					if(fabs(SynLenRec-SynLenTx)<300)  //300是MATLAB仿真结果值，根据DSSS算法本身可以对抗+/-0.7m/s左右的多普勒
					{
						DSSSDemodulate(&g_BackUp[0],pos);
					}
					else
					{
						u32 p = 10000;
						u32 q = round((1.0*SynLenRec/SynLenTx)*10000);
						u32 inputlen = g_ReceiveBlock*(g_ReceiveBlockNumber+2);   //无需后面的同步信号
						u32 outputlen = round(1.0*(p-1)/(q-1)*inputlen);
						float frac = 1.0*q/p;
						
						float *g_BackUpResample;
						g_BackUpResample = mymalloc(SRAMEX,outputlen*4);
						memset(&g_BackUpResample[0],0,outputlen*4);
						Resample(g_BackUp,inputlen,g_BackUpResample,outputlen,frac);
						pos = g_1stPoint+g_ReceiveBlock*4-g_Delay;
						DSSSDemodulate(&g_BackUpResample[0],pos);
						myfree(SRAMEX,g_BackUpResample);
					}
				}
				else
				{					
					DSSSDemodulate(&g_BackUp[0],pos);
				}
				g_DecodeEN = 0;
				g_1stSynIsOK = 0;
				g_RecvCnt = 0;
			}
		}
    else if(g_CommunicationMode == Storage)
    {
      DataSave();
			Uart1_Puts(&SysAnswerCmd.DATA_SAVE_ANSWER[0],9);
      g_CommunicationMode = Receive;
			g_IntMode = Receive;
    }
		    else
    if(g_CommunicationMode==Debug)
    {
//			DataRecv(SigRECV,ReadDataLen*11,0);
//			for(u32 i=0;i<ReadDataLen;i++)
//			{
//				strncpy(&Tmp[0],&SigRECV[11*i],9);
//				Test2[i] = atof(&Tmp[0]);
//			}
//			myfree(SRAMEX,SigRECV);
//			memcpy(g_BackUp,Test2,ReadDataLen*4);
//			myfree(SRAMEX,Test2);
//			myfree(SRAMEX,Tmp);

//			g_CommunicationMode = Receive;
//			g_IntMode = Receive;
//			g_DecodeEN = 1;
//			Uart1_Puts(&SysAnswerCmd.SYN_ANSWER[0],9);				
		}
	}
}


#endif












