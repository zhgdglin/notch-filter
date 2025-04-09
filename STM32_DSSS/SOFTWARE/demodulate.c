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
#include "system.h"

void DSSSDemodulate(float *DsssSig,u32 Pos)
{	
	signed char *BinCode;
	BinCode = mymalloc(SRAMIN,g_MaxByteLen*8);					
	memset(&BinCode[0],0,g_MaxByteLen*8);						
	
	float *SigMixReal;
	SigMixReal = mymalloc(SRAMEX,(g_ModuLen+32)*4);
	memset(&SigMixReal[0],0,(g_ModuLen+32)*4);
	
	float *SigMixImag;
	SigMixImag = mymalloc(SRAMEX,(g_ModuLen+32)*4);
	memset(&SigMixImag[0],0,(g_ModuLen+32)*4);
	
	float *BandSigReal;
	BandSigReal = mymalloc(SRAMEX,(g_ModuLen+32)*4);
	memset(&BandSigReal[0],0,(g_ModuLen+32)*4);
	
	float *BandSigImag;
	BandSigImag = mymalloc(SRAMEX,(g_ModuLen+32)*4);
	memset(&BandSigImag[0],0,(g_ModuLen+32)*4);
	
	u8 *Decode;
	Decode = mymalloc(SRAMEX,g_MaxByteLen);
	memset(&Decode[0],0,g_MaxByteLen);
		
	//下变频
	for(u32 i=0;i<g_ModuLen;i++)
	{
		double x;
		x = 2*PI*g_f0*i/g_fs;
		SigMixReal[i] = cos(x)*DsssSig[Pos+i];
		SigMixImag[i] = sin(x)*DsssSig[Pos+i];
	}
	
	//滤波	
	DSSSBandPassFilter(&SigMixReal[0],g_ModuLen+32,&g_BandPassCoeff[0],65,&BandSigReal[0],32);//实部滤波
	DSSSBandPassFilter(&SigMixImag[0],g_ModuLen+32,&g_BandPassCoeff[0],65,&BandSigImag[0],32);//虚部滤波	
	
  myfree(SRAMEX,SigMixReal);
	myfree(SRAMEX,SigMixImag);
  
	//实部虚部合成
  float *BandSigCplx;
	BandSigCplx = mymalloc(SRAMEX,2*g_ModuLen*4);
	memset(&BandSigCplx[0],0,2*g_ModuLen*4);
  
	EvenOddRank(&BandSigReal[32],&BandSigImag[32],g_ModuLen,&BandSigCplx[0]);
	
  myfree(SRAMEX,BandSigReal);
	myfree(SRAMEX,BandSigImag);
  
	//能量差分检测器
	DiffEnergyDector(&BandSigCplx[0],g_ModuLen,&BinCode[0]);
	
	//数据转换
	SignedBinToChar(&BinCode[0],&Decode[0],g_MaxByteLen*8,8);
	
	u8 DecodedLen;
	DecodedLen = Decode[0];
	DecodeLogSave(&Decode[0],30);
	Uart1_Puts(&Decode[0],30);
	if(g_save == 1)
	{
		s32 *g_BackUps;
		g_BackUps = mymalloc(SRAMEX,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
		memset(&g_BackUps[0],0,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
		for(u32 i=0;i<g_ReceiveBlock*(g_ReceiveBlockNumber+2);i++)
		{
			g_BackUps[i] = DsssSig[i]*21296;
		}
		DataSaveDecode(g_BackUps,g_ReceiveBlock*(g_ReceiveBlockNumber+2));
		myfree(SRAMEX,g_BackUps);
	}
	if(XORCaculate(&Decode[0],DecodedLen+1) == Decode[DecodedLen+1])
	{
//		Uart1_Puts(&Decode[0],DecodedLen+1);
//		DecodeAnalyse(&DecodedBits[1]);
	}
	myfree(SRAMIN,BinCode);
	myfree(SRAMEX,BandSigCplx);	
}

