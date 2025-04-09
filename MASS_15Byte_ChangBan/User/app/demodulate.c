#include "bsp.h"

/************************************************
 STM32H7	单载波解调程序
 解调过程：信号抽样、同步检测、
 技术开发：Jason Lee
 时间：2020/07/20（疫情严重）
************************************************/

void DemodulateMaSS(float *SigReceived,u32 Pos)
{
	u8 *UratHex;
	UratHex = mymalloc(SRAMIN,g_InfoByteNumPerFrame);      
	memset(&UratHex[0],0,g_InfoByteNumPerFrame);

  //解码 
	MassDemodulate(&SigReceived[Pos],g_ModuLen,&UratHex[0]);

	u8 DecodedLen;
	DecodedLen = UratHex[1];
//	Uart1_Puts(&UratHex[0],32);
	DecodeLogSave(&UratHex[0],DecodedLen+3);
	if(g_save == 1)
	{
		s32 *g_BackUps;
		g_BackUps = mymalloc(SRAMEX,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
		memset(&g_BackUps[0],0,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
		for(u32 i=0;i<g_ReceiveBlock*(g_ReceiveBlockNumber+2);i++)
		{
			g_BackUps[i] = SigReceived[i]*4194304;
		}
		DataSaveDecode(g_BackUps,g_ReceiveBlock*(g_ReceiveBlockNumber+2));
		myfree(SRAMEX,g_BackUps);
	}
	if(XORCaculate(&UratHex[0],DecodedLen+2) == UratHex[DecodedLen+2])
	{
		if(UratHex[0] == 0x00)
		{
			memcpy(&g_BitsDecoded[7+g_ReceiveNum*(g_InfoByteNumPerFrame-3)],&UratHex[2],g_InfoByteNumPerFrame-3);
			g_ReceiveNum = g_ReceiveNum+1;
		}
		else
		{
			g_BitsDecoded[0] = 0xA5;
			g_BitsDecoded[1] = 0xA5;
			g_BitsDecoded[2] = 0xA5;
			g_BitsDecoded[3] = 0xD5;	
			g_BitsDecoded[4] = 0x01;
			g_BitsDecoded[5] = 0x00;	
			g_BitsDecoded[6] = (g_InfoByteNumPerFrame-3)*g_ReceiveNum+UratHex[1];			
			memcpy(&g_BitsDecoded[7+g_ReceiveNum*(g_InfoByteNumPerFrame-3)],&UratHex[2],DecodedLen);
			g_BitsDecoded[7+g_ReceiveNum*(g_InfoByteNumPerFrame-3)+DecodedLen] = 0xA5;			
			g_BitsDecoded[7+g_ReceiveNum*(g_InfoByteNumPerFrame-3)+DecodedLen+1] = 0xD4;
			Uart1_Puts(&g_BitsDecoded[0],g_ReceiveNum*(g_InfoByteNumPerFrame-3)+DecodedLen+9);
			g_ReceiveNum = 0;
			g_TimeCnt = 0;	
//			DecodeAnalyse(&g_BitsDecoded[2]);
		}
	}
	myfree(SRAMEX,UratHex);
}	
