#ifndef FUNCTION_H_
#define FUNCTION_H_
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

void ParameterInit(void)
{
	g_f0 = (g_fh+g_fl)/2;
	g_fb = (g_fh-g_fl);
	g_ReceiveBlock = g_SynTime*g_fs;
	g_TransmitBlock = 6144;
	g_Nsap = g_fs/g_fb*(1+g_alpha);
	g_ModuLen = (g_MaxByteLen*8+1)*g_Nsap*(1<<g_MOrder);
	g_SigLen = g_ModuLen+g_ReceiveBlock*4;
	
	if(g_MOrder==6)//八进制103
	{
		g_FeedFinal[5] = 1;
		g_FeedFinal[4] = 0;
		g_FeedFinal[3] = 0;
		g_FeedFinal[2] = 0;
		g_FeedFinal[1] = 0;
		g_FeedFinal[0] = 1;
		g_MCodeInital[5] = 0;
		g_MCodeInital[4] = 0;
		g_MCodeInital[3] = 1;
		g_MCodeInital[2] = 1;
		g_MCodeInital[1] = 0;
		g_MCodeInital[0] = 1;
	}
	else
	{}
	
	if(g_SigLen%g_ReceiveBlock == 0)
	{
		g_ReceiveBlockNumber = g_SigLen/g_ReceiveBlock;
	}
	else
	{
		g_ReceiveBlockNumber = g_SigLen/g_ReceiveBlock+1;
	}
	
	if(g_SigLen%g_TransmitBlock == 0)
	{
		g_TransmitBlockNumber = g_SigLen/g_TransmitBlock;
	}
	else
	{
		g_TransmitBlockNumber = g_SigLen/g_TransmitBlock+1;
	}
	
	g_FFTLen = 4096;
	
	g_CacheA = mymalloc(SRAMEX,g_ReceiveBlock*4);
	memset(&g_CacheA[0],0,g_ReceiveBlock*4);
	
	g_CacheB = mymalloc(SRAMEX,g_ReceiveBlock*4);
	memset(&g_CacheB[0],0,g_ReceiveBlock*4);
	
	g_Cachef32 = mymalloc(SRAMEX,g_ReceiveBlock*4);
	memset(&g_Cachef32[0],0,g_ReceiveBlock*4);
	
	g_SynHFM = mymalloc(SRAMEX,g_ReceiveBlock*4);
	memset(&g_SynHFM[0],0,g_ReceiveBlock*4);
	
	g_1stCopy = mymalloc(SRAMEX,2*g_FFTLen*4);
	memset(&g_1stCopy[0],0,2*g_FFTLen*4);
	
	g_2ndCopy = mymalloc(SRAMEX,2*g_ReceiveBlock*4);
	memset(&g_2ndCopy[0],0,2*g_ReceiveBlock*4);
	
//	g_SynLFM = mymalloc(SRAMEX,g_ReceiveBlock*4);
//	memset(&g_SynLFM[0],0,g_ReceiveBlock*4);
//	
//	g_1stCopyLFM = mymalloc(SRAMEX,2*g_FFTLen*4);
//	memset(&g_1stCopyLFM[0],0,2*g_FFTLen*4);
	
	g_SigBlk = mymalloc(SRAMEX,(g_ReceiveBlock+g_Order)*4);
	memset(&g_SigBlk[0],0,(g_ReceiveBlock+g_Order)*4);
	
	g_SigMix = mymalloc(SRAMEX,2*(g_ReceiveBlock+g_Delay)*4);
	memset(&g_SigMix[0],0,2*(g_ReceiveBlock+g_Delay)*4);
	
  g_BagBlk = mymalloc(SRAMEX,4*g_ReceiveBlock*4);
	memset(&g_BagBlk[0],0,4*g_ReceiveBlock*4);
  
  g_SynBlk = mymalloc(SRAMEX,2*g_ReceiveBlock*4);
	memset(&g_SynBlk[0],0,2*g_ReceiveBlock*4);
  
	g_SigBaseDown = mymalloc(SRAMEX,2*g_FFTLen*4);
	memset(&g_SigBaseDown[0],0,2*g_FFTLen*4);
	
  g_BagTmpBlk = mymalloc(SRAMEX,g_Order*4);
	memset(&g_BagTmpBlk[0],0,g_Order*4);
	
	g_BackUp = mymalloc(SRAMEX,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
	memset(&g_BackUp[0],0,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
	  
  g_StoreDataBuf = mymalloc(SRAMEX,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
	memset(&g_StoreDataBuf[0],0,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
	
	HFMSignalGen(g_SynTime,g_SynHFM);
	
  HFMCopyGen(g_SynHFM,g_ReceiveBlock,g_1stCopy,g_2ndCopy);
	
//	LFMSignalGen(g_SynTime,g_SynLFM);	

//	HFMCopyGen(g_SynLFM,g_ReceiveBlock,g_1stCopyLFM,g_2ndCopy);
}

void ParameterFree(void)
{
	myfree(SRAMEX,g_CacheA);
	myfree(SRAMEX,g_CacheB);
	myfree(SRAMEX,g_Cachef32);
	myfree(SRAMEX,g_SynHFM);
	myfree(SRAMEX,g_1stCopy);
	myfree(SRAMEX,g_2ndCopy);
//	myfree(SRAMEX,g_SynLFM);
//	myfree(SRAMEX,g_1stCopyLFM);
	myfree(SRAMEX,g_SigBlk);
	myfree(SRAMEX,g_SigMix);
	myfree(SRAMEX,g_BagBlk);
	myfree(SRAMEX,g_SynBlk);
	myfree(SRAMEX,g_SigBaseDown);
	myfree(SRAMEX,g_BagTmpBlk);
	myfree(SRAMEX,g_BackUp);
}

void EvenOddCombine(float *pSrcA,float *pSrcB,float *pDst,u32	blockSize)
{
	for(u32 i=0;i<blockSize;i++)
	{
		pDst[2*i] = pSrcA[i];
		pDst[2*i+1] = pSrcB[i];
	}
}

void HFMSignalGen(float T,float *SynSig)
{	
	//双曲调频信号
  float tt1 = T*(g_f0/g_fb);
  float K1 = g_fl*(tt1+T/2);
  float t0;
  float Tmp;
  for(u32 i=0;i<(int)(g_fs*T);i++)
  {
    t0 = (float)i/g_fs;
    Tmp = 2*PI*K1*log(1+(t0-T/2)/tt1);
    SynSig[i] = 0.8*sin(Tmp);
  }
}

void LFMSignalGen(float SigTime,float *SynSig)
{
	//线性调频
	float K0 = g_fb/SigTime;
	float t0;
	for(u32 i=0;i<g_fs*SigTime;i++)
	{
		t0 = (float)i/g_fs;
		SynSig[i] = 0.8*cos(2*PI*g_fl*i/g_fs+K0*PI*t0*t0);
	}
}

void S32ToF32(s32 *DataIn,u32 Len,float *DataOut)
{
	for(u32 i=0;i<Len;i++)
	{
		DataOut[i] = (float)DataIn[i]/21296;
	}
}

void DownSamplingCplx(float *Sig,u32 SigLen,u32 Q,float *SigDownSamp)
{
	for(u32 i=0;i<SigLen/Q;i++)
	{
		SigDownSamp[2*i] = Sig[2*Q*i];
		SigDownSamp[2*i+1] = Sig[2*Q*i+1];
	}
}

void HFMCopyGen(float *HFMSig,u32 HFMSigLen,float *HFMFreqConj,float *BaseBandSig)
{
	const u32	BlockSize = HFMSigLen+g_Delay;
	float Result[2];
	float SqrtTmp;
	
	float *MixSigReal;
	MixSigReal = mymalloc(SRAMEX,(HFMSigLen+g_Delay)*4);
	memset(&MixSigReal[0],0,(HFMSigLen+g_Delay)*4);
	
	float *MixSigImag;
	MixSigImag = mymalloc(SRAMEX,(HFMSigLen+g_Delay)*4);
	memset(&MixSigImag[0],0,(HFMSigLen+g_Delay)*4);
	
	float *BandSigReal;
	BandSigReal = mymalloc(SRAMEX,(HFMSigLen+g_Delay)*4);
	memset(&BandSigReal[0],0,(HFMSigLen+g_Delay)*4);
	
	float *BandSigImag;
	BandSigImag = mymalloc(SRAMEX,(HFMSigLen+g_Delay)*4);
	memset(&BandSigImag[0],0,(HFMSigLen+g_Delay)*4);
	
	float *SigSamp;
	SigSamp = mymalloc(SRAMEX,g_FFTLen*4);
	memset(&SigSamp[0],0,g_FFTLen*4);
	
	float *SigSampConj;
	SigSampConj = mymalloc(SRAMEX,g_FFTLen*4);
	memset(&SigSampConj[0],0,g_FFTLen*4);
	
	float *SigDown;
	SigDown = mymalloc(SRAMEX,(2*g_FFTLen)*4);
	memset(&SigDown[0],0,(2*g_FFTLen)*4);
	
	float *firState;
	firState = mymalloc(SRAMEX,(BlockSize+g_Order)*4);
	memset(&firState[0],0,(BlockSize+g_Order)*4);
	
	arm_fir_instance_f32 S;
	arm_fir_init_f32(&S,g_Order+1,&g_LowPassCoeff[0],&firState[0],BlockSize);
	
	for(u32 i=0;i<HFMSigLen;i++)
	{
		MixSigReal[i] = HFMSig[i]*cos(2*PI*g_f0*i/g_fs);
		MixSigImag[i] = HFMSig[i]*(-1)*sin(2*PI*g_f0*i/g_fs);
	}
	
	//滤波，获得基带信号	
	arm_fir_f32(&S,MixSigReal,BandSigReal,BlockSize);
	arm_fir_f32(&S,MixSigImag,BandSigImag,BlockSize);
	
	//将实部和虚部信号按偶、奇顺序合成复数信号
	EvenOddCombine(&BandSigReal[g_Delay],&BandSigImag[g_Delay],&BaseBandSig[0],HFMSigLen);
	
	//抽样
	DownSamplingCplx(&BaseBandSig[0],g_ReceiveBlock,g_DownSamplingNum,&SigSamp[0]);
	
	//向量相乘
	arm_cmplx_conj_f32(&SigSamp[0],&SigSampConj[0],g_FFTLen/2);
	arm_cmplx_dot_prod_f32(SigSamp,SigSampConj,g_FFTLen/2,&Result[0],&Result[1]);
	
	//开方
	arm_sqrt_f32(Result[0],&SqrtTmp);
	
	//复数FFT
	arm_scale_f32(SigSamp,1.0f/SqrtTmp,&SigDown[0],g_FFTLen);
	arm_cfft_f32(&arm_cfft_sR_f32_len4096,&SigDown[0],0,1);
	
	arm_cmplx_conj_f32(&SigDown[0],&HFMFreqConj[0],g_FFTLen);
	
	myfree(SRAMEX,MixSigReal);
	myfree(SRAMEX,MixSigImag);
	myfree(SRAMEX,BandSigReal);
	myfree(SRAMEX,BandSigImag);
	myfree(SRAMEX,SigSamp);
	myfree(SRAMEX,SigSampConj);
	myfree(SRAMEX,SigDown);
	myfree(SRAMEX,firState);
}

void DownConversion_SYN(float *Sig,u32 SigLen,float *BaseSig)//DigitalDownConvtFrequency
{
	const u32	BlockSize = SigLen+g_Delay;
	
	float *SigMixReal;
	SigMixReal = mymalloc(SRAMIN,(SigLen+g_Delay)*4);
	memset(&SigMixReal[0],0,(SigLen+g_Delay)*4);
	
	float *SigMixImag;
	SigMixImag = mymalloc(SRAMIN,(SigLen+g_Delay)*4);
	memset(&SigMixImag[0],0,(SigLen+g_Delay)*4);
	
	float *BandSigReal;
	BandSigReal = mymalloc(SRAMIN,(BlockSize+g_Delay)*4);
	memset(&BandSigReal[0],0,(BlockSize+g_Delay)*4);
	
	float *BandSigImag;
	BandSigImag = mymalloc(SRAMIN,(BlockSize+g_Delay)*4);
	memset(&BandSigImag[0],0,(BlockSize+g_Delay)*4);
			
	float *firState;
	firState = mymalloc(SRAMIN,(BlockSize+g_Delay)*4);
	memset(&firState[0],0,(BlockSize+g_Delay)*4);
	
	arm_fir_instance_f32 S;
	arm_fir_init_f32(&S,g_Order+1,&g_LowPassCoeff[0],&firState[0],BlockSize);
	
	//信号和正弦和余弦信号相乘,欧拉公式e(-j*wt)=cos(wt)-j*sin(wt),因此需乘以sin的相反数
	for(u32 i=0;i<SigLen;i++)
	{
		SigMixReal[i] = Sig[i]*cos(2*PI*g_f0*i/g_fs);
		SigMixImag[i] = Sig[i]*(-1)*sin(2*PI*g_f0*i/g_fs);
	}

	//滤波，获得基带信号	
	arm_fir_f32(&S,&SigMixReal[0],&BandSigReal[0],BlockSize);
	arm_fir_f32(&S,&SigMixImag[0],&BandSigImag[0],BlockSize);

	//将实部和虚部信号按偶、奇顺序合成复数信号
	EvenOddCombine(&BandSigReal[2*g_Delay],&BandSigImag[2*g_Delay],&BaseSig[0],SigLen);
	
	myfree(SRAMIN,SigMixReal);
	myfree(SRAMIN,SigMixImag);
	myfree(SRAMIN,BandSigReal);
	myfree(SRAMIN,BandSigImag);
	myfree(SRAMIN,firState);
}


u8 Syn1st(float *Sig,u32 SigLen,float *RefCopy,u32 *Pose1st)
{
	float Result[2] = {0,0};
	float SqrtResult = 0;
	float Peak = 0;
	u32   Pose = 0;
	u8		SynFlag = 0;
	
	float *SigBlkConj;
	SigBlkConj = mymalloc(SRAMIN,2*SigLen*4);
	memset(&SigBlkConj[0],0,2*SigLen*4);
	
	float *SigDownBlk;
	SigDownBlk = mymalloc(SRAMIN,2*SigLen*4);
	memset(&SigDownBlk[0],0,2*SigLen*4);
	
	float *FrqBlock;
	FrqBlock = mymalloc(SRAMIN,2*SigLen*4);
	memset(&FrqBlock[0],0,2*SigLen*4);
	
	float *CorrAbs;
	CorrAbs = mymalloc(SRAMIN,(SigLen/2)*4);
	memset(&CorrAbs[0],0,(SigLen/2)*4);
	
	//向量相乘
	arm_cmplx_conj_f32(&Sig[0],&SigBlkConj[0],SigLen);
	arm_cmplx_dot_prod_f32(&Sig[0],&SigBlkConj[0],SigLen,&Result[0],&Result[1]);
	
	//开方
	arm_sqrt_f32(Result[0],&SqrtResult);
	
	//复数FFT
	arm_scale_f32(&Sig[0],1.0f/SqrtResult,&SigDownBlk[0],2*SigLen);
	arm_cfft_f32(&arm_cfft_sR_f32_len4096,&SigDownBlk[0],0,1);
	
	//复数点乘
	arm_cmplx_mult_cmplx_f32 (&SigDownBlk[0],&RefCopy[0],&FrqBlock[0],g_FFTLen);
	arm_cfft_f32(&arm_cfft_sR_f32_len4096,&FrqBlock[0],1,1);
	
	//求模值
	arm_cmplx_mag_f32(&FrqBlock[0],&CorrAbs[0],(g_FFTLen/2));	
    
	//求最大值，并计算最大值位置
	arm_max_f32(&CorrAbs[0],(g_FFTLen/2),&Peak,&Pose);
	
	if(Peak>0.2f)
	{
		*Pose1st = (Pose)*g_DownSamplingNum;
		SynFlag = 1;
	}
	else
	{
		SynFlag = 0;
	}
	
	myfree(SRAMIN,SigBlkConj);
	myfree(SRAMIN,SigDownBlk);
	myfree(SRAMIN,FrqBlock);
	myfree(SRAMIN,CorrAbs);
	
	return SynFlag;
}

u8 XORCaculate(u8 *Data,u32 Len)
{
  u8 Result = 0;
  for(u32 i=0;i<Len;i++)
  {
    Result = Data[i]^Result;
  }
  return Result;
}

u8 CheckSum(u8 *Data,u32 Len)
{
	u32 Result = 0;
  for(u32 i=0;i<Len;i++)
  {
    Result = Data[i]+Result;
  }
	if(Result > 255)
	{
		Result = ~(Result&0xFF)+1;
	}
  return (u8)Result;
}

void DSSSDiffCode(signed char *Code,u32 CodeLen,signed char *DiffCode)
{
	for(u16 i=0;i<CodeLen;i++)
	{
		DiffCode[i+1] = Code[i]*DiffCode[i];
	}
}

void MCodeGen(char *InitCode,signed char *Mcode)
{
	char x[g_MOrder];	
	u32 SumRS;
	
	x[g_MOrder-1] = 1;
	
	for(u32 i=0;i<(1<<g_MOrder);i++)
	{
		Mcode[i] = 2*InitCode[g_MOrder-1]-1;
		if(memcmp(&x[0],&InitCode[0],g_MOrder)==0)
		{
			i++;
			Mcode[i] = -1;
		}
		
		SumRS = DotProduct(&g_FeedFinal[0],&InitCode[0],g_MOrder);
		
		for(u8 i=g_MOrder-1;i>0;i--)
		{
			InitCode[i] = InitCode[i-1];
		}
		InitCode[0] = SumRS%2;
	}
}

void DecToBin(int pSrc, char *pDst, u32	blockSize)
{
	int	i,j,k,m,n;
	m = blockSize;
	n = pSrc;
	for(k=0;k<m;k++)
	{
		i = n % 2;      /*取2的余数*/
		j = n / 2;      /*取被2整除的结果*/
		n = j;          /*将得到的商赋给变量n*/
		*(pDst+k) = i;   
	}
}

void DSSSKron(signed char *DataA,u32 DataALen,signed char *DataB,u32 DataBLen,signed char *DSCode)
{
	for(u32 i=0;i<DataALen;i++)
	{
		for(u32 j=0;j<DataBLen;j++)
		{
			DSCode[i*DataBLen+j] = DataA[i]*DataB[j];
		}
	}
}

void Rectpulse(signed char *DataA,u32 DataALen,u32 DataB,signed char *DataC)
{
	u32 i,j;
	for(i=0;i<DataALen;i++)
	{
		for(j=0;j<DataB;j++)
		{
			DataC[i*DataB+j] = DataA[i];
		}
	}
}

u32 DotProduct(char *DataA,char *DataB,u32 DataSize)
{ 
	u32 Sum = 0;
	for(u32 i=0;i<DataSize;i++)
	{
		Sum = Sum + DataA[i]*DataB[i];
	}
	return Sum;
}

void DSSSBandPassFilter(float *pSrc,u32 SrcLen,float *Coeff,u32 CoeffLen,float *pDst,u32 blockSize)
{
	u32 numBlocks = SrcLen/blockSize;
	float	firState[CoeffLen+blockSize-1];
  
	arm_fir_instance_f32 S;
	arm_fir_init_f32(&S,CoeffLen,Coeff,&firState[0],blockSize);
  
	for(u32 i = 0;i < numBlocks;i++)
	{
		arm_fir_f32(&S,&pSrc[i*blockSize],&pDst[i*blockSize],blockSize);
	}
}

void DiffEnergyDector(float *Sig,u32 SigLen,signed char *Decode)
{
	u32 i,j,k,DoubleMCodeLen,DoubleSymbolLen,Width,Index1,Index2;
	DoubleMCodeLen = 2*(1<<g_MOrder);
	DoubleSymbolLen = 2*(1<<g_MOrder)*g_Nsap;
	Width = (1<<g_MOrder)/4;
	
	char *MCodeInital;
  MCodeInital = mymalloc(SRAMIN,g_MOrder);
	memset(&MCodeInital[0],0,g_MOrder);
	
	signed char *MCode;
	MCode = mymalloc(SRAMIN,(1<<g_MOrder));
	memset(&MCode[0],0,(1<<g_MOrder));
	
	//产生本地匹配副本（频域）
	memcpy(&MCodeInital[0],&g_MCodeInital[0],g_MOrder);
	MCodeGen(&MCodeInital[0],MCode);
	myfree(SRAMIN,MCodeInital);
	
	signed char *PN1TP;
	PN1TP = mymalloc(SRAMIN,DoubleMCodeLen);
	memset(&PN1TP[0],0,DoubleMCodeLen);
	
	signed char *PN2TP;
	PN2TP = mymalloc(SRAMIN,DoubleMCodeLen);
	memset(&PN2TP[0],0,DoubleMCodeLen);
	
	memcpy(&PN1TP[0],&MCode[0],(1<<g_MOrder));
	memcpy(&PN2TP[0],&MCode[0],(1<<g_MOrder));
	for(u32 i=0;i<(1<<g_MOrder);i++)
	{
		PN1TP[(1<<g_MOrder)+i] = MCode[i];
		PN2TP[(1<<g_MOrder)+i] = -MCode[i];
	}
	myfree(SRAMIN,MCode);
	
	float *PN1;
	PN1 = mymalloc(SRAMIN,DoubleMCodeLen*4);
	memset(&PN1[0],0,DoubleMCodeLen*4);
	
	float *PN2;
	PN2 = mymalloc(SRAMIN,DoubleMCodeLen*4);
	memset(&PN2[0],0,DoubleMCodeLen*4);
	
	for(u32 i=0;i<DoubleMCodeLen;i++)
	{
		PN1[i] = PN1TP[DoubleMCodeLen-1-i];
		PN2[i] = PN2TP[DoubleMCodeLen-1-i];
	}
	myfree(SRAMIN,PN1TP);
	myfree(SRAMIN,PN2TP);
	
	float *PN1_freq;
	PN1_freq = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&PN1_freq[0],0,2*DoubleMCodeLen*4);
	
	float *PN2_freq;
	PN2_freq = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&PN2_freq[0],0,2*DoubleMCodeLen*4);

	arm_rfft_fast_instance_f32 d;  
  arm_rfft_fast_init_f32(&d, DoubleMCodeLen); 
  arm_rfft_fast_f32(&d, PN1, PN1_freq, 0); 
	arm_rfft_fast_f32(&d, PN2, PN2_freq, 0); 
	for(u32 i=0;i<(1<<g_MOrder)-1;i++)
	{
		PN1_freq[2*(i+(1<<g_MOrder)+1)] = PN1_freq[2*((1<<g_MOrder)-1-i)];
		PN1_freq[2*(i+(1<<g_MOrder)+1)+1] = -PN1_freq[2*((1<<g_MOrder)-1-i)+1];
		PN2_freq[2*(i+(1<<g_MOrder)+1)] = PN2_freq[2*((1<<g_MOrder)-1-i)];
		PN2_freq[2*(i+(1<<g_MOrder)+1)+1] = -PN2_freq[2*((1<<g_MOrder)-1-i)+1];
	}
	myfree(SRAMIN,PN1);
	myfree(SRAMIN,PN2);

	float _Complex *SigCplx;
	SigCplx = mymalloc(SRAMEX,2*g_ModuLen*4);
	memset(&SigCplx[0],0,2*g_ModuLen*4);
	
	float _Complex *SigBag;
	SigBag = mymalloc(SRAMIN,2*DoubleSymbolLen*4);
	memset(&SigBag[0],0,2*DoubleSymbolLen*4);

	float _Complex *SigBlk;
	SigBlk = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&SigBlk[0],0,2*DoubleMCodeLen*4);
	
	float *FrqBlk;
	FrqBlk = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&FrqBlk[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *FrqBlkCplx;
	FrqBlkCplx = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&FrqBlkCplx[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *FrqBlk1;
	FrqBlk1 = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&FrqBlk1[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *FrqBlk2;
	FrqBlk2 = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&FrqBlk2[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *PN1Frq;
	PN1Frq = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&PN1Frq[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *PN2Frq;
	PN2Frq = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&PN2Frq[0],0,2*128*4);
	
	float _Complex *Corr1Cplx;
	Corr1Cplx = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&Corr1Cplx[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *Corr2Cplx;
	Corr2Cplx = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&Corr2Cplx[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *JK1Cplx;
	JK1Cplx = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&JK1Cplx[0],0,2*DoubleMCodeLen*4);
	
	float _Complex *JK2Cplx;
	JK2Cplx = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&JK2Cplx[0],0,2*DoubleMCodeLen*4);
	
	float *Corr1;
	Corr1 = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&Corr1[0],0,2*DoubleMCodeLen*4);
	
	float *Corr2;
	Corr2 = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&Corr2[0],0,2*DoubleMCodeLen*4);
	
	float *JK1;
	JK1 = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&JK1[0],0,2*DoubleMCodeLen*4);
	
	float *JK2;
	JK2 = mymalloc(SRAMIN,2*DoubleMCodeLen*4);
	memset(&JK2[0],0,2*DoubleMCodeLen*4);
	
	float *Abs1;
	Abs1 = mymalloc(SRAMIN,DoubleMCodeLen*4);
	memset(&Abs1[0],0,DoubleMCodeLen*4);
	
	float *Abs2;
	Abs2 = mymalloc(SRAMIN,DoubleMCodeLen*4);
	memset(&Abs2[0],0,DoubleMCodeLen*4);
	
	float *EN1;
	EN1 = mymalloc(SRAMIN,g_MaxByteLen*8*4);
	memset(&EN1[0],0,g_MaxByteLen*8*4);
	
	float *EN2;
	EN2 = mymalloc(SRAMIN,g_MaxByteLen*8*4);
	memset(&EN2[0],0,g_MaxByteLen*8*4);
	
	memcpy(&SigCplx[0],&Sig[0],2*g_ModuLen*4);
	
	for(i=0;i<g_MaxByteLen*8;i++)
	{
		//接收信号预处理
		memcpy(&SigBag[0],&SigCplx[i*DoubleSymbolLen/2],2*DoubleSymbolLen*4);
		memset(&SigBlk[0],0,2*DoubleMCodeLen*4);

		for(j=0;j<DoubleMCodeLen;j++)
		{
			for(k=0;k<g_Nsap;k++)
			{
				SigBlk[j] = SigBlk[j] + SigBag[j*g_Nsap+k];
			}
		}
		
		//求FFT
		memcpy(&FrqBlk[0],&SigBlk[0],2*DoubleMCodeLen*4);
		arm_cfft_f32(&arm_cfft_sR_f32_len128,&FrqBlk[0],0,1);
		
		//复数相乘		
		memcpy(&PN1Frq[0],&PN1_freq[0],2*DoubleMCodeLen*4);
		memcpy(&PN2Frq[0],&PN2_freq[0],2*DoubleMCodeLen*4);
		memcpy(&FrqBlkCplx[0],&FrqBlk[0],2*DoubleMCodeLen*4);
		for(u32 n=0;n<DoubleMCodeLen;n++)
		{
			FrqBlk1[n] = FrqBlkCplx[n]*PN1Frq[n];
			FrqBlk2[n] = FrqBlkCplx[n]*PN2Frq[n];
		}
		
		//求逆FFT
		memcpy(&Corr1[0],&FrqBlk1[0],2*DoubleMCodeLen*4);
		memcpy(&Corr2[0],&FrqBlk2[0],2*DoubleMCodeLen*4);
		arm_cfft_f32(&arm_cfft_sR_f32_len128,&Corr1[0],1,1);//0,1为FFT;1,1为IFFT
		arm_cfft_f32(&arm_cfft_sR_f32_len128,&Corr2[0],1,1);//0,1为FFT;1,1为IFFT		
		
		//复数相乘
		memcpy(&Corr1Cplx[0],&Corr1[0],2*DoubleMCodeLen*4);
		memcpy(&Corr2Cplx[0],&Corr2[0],2*DoubleMCodeLen*4);
		for(u32 m=0;m<(Width*2+1);m++)
		{
			JK1Cplx[m] = Corr1Cplx[(1<<g_MOrder)-Width-1+m]*Corr1Cplx[(1<<g_MOrder)-Width-1+m];
			JK2Cplx[m] = Corr2Cplx[(1<<g_MOrder)-Width-1+m]*Corr2Cplx[(1<<g_MOrder)-Width-1+m];
		}
		memcpy(&JK1[0],&JK1Cplx[0],2*(Width*2+1)*4);
		memcpy(&JK2[0],&JK2Cplx[0],2*(Width*2+1)*4);
		
		//求最值
		arm_cmplx_mag_f32(&JK1[0],&Abs1[0],(Width*2+1));
		arm_cmplx_mag_f32(&JK2[0],&Abs2[0],(Width*2+1));
		arm_max_f32(&Abs1[0],(Width*2+1),&EN1[i],&Index1);
		arm_max_f32(&Abs2[0],(Width*2+1),&EN2[i],&Index2);
		
		//判决
		if(EN1[i]>EN2[i])
		{
			Decode[i] = 1;
		}
		else
		{
			Decode[i] = 0;
		}
	}
	
	myfree(SRAMIN,PN1_freq);
	myfree(SRAMIN,PN2_freq);
	myfree(SRAMEX,SigCplx);
	myfree(SRAMIN,SigBag);
	myfree(SRAMIN,SigBlk);
	myfree(SRAMIN,FrqBlk);
	myfree(SRAMIN,FrqBlkCplx);
	myfree(SRAMIN,FrqBlk1);
	myfree(SRAMIN,FrqBlk2);
	myfree(SRAMIN,PN1Frq);
	myfree(SRAMIN,PN2Frq);
	myfree(SRAMIN,Corr1);
	myfree(SRAMIN,Corr2);
	myfree(SRAMIN,Corr1Cplx);
	myfree(SRAMIN,Corr2Cplx);
	myfree(SRAMIN,JK1);
	myfree(SRAMIN,JK2);
	myfree(SRAMIN,JK1Cplx);
	myfree(SRAMIN,JK2Cplx);
	myfree(SRAMIN,Abs1);
	myfree(SRAMIN,Abs2);
	myfree(SRAMIN,EN1);
	myfree(SRAMIN,EN2);
}

void EvenOddRank(float *EvenSig,float *OddSig,u32 SigLen,float *CplxSig)
{
	for(u32 i=0;i<SigLen;i++)
	{
		CplxSig[2*i] = EvenSig[i];
		CplxSig[2*i+1] = OddSig[i];
	}
}

void SignedBinToChar (signed char *pSrc, u8 *pDst, u32 pSrc_len,u32 bits_len)
{
	u16 i,j;
	u16 args,a;
	for(i=0;i<pSrc_len/bits_len;i++)
	{
		args = 0;
		for(j=0;j<bits_len;j++)
		{
			a = pSrc[bits_len*i+j]*pow(2,j);
			args = args + a;
			if(j==bits_len-1)
			{
				pDst[i] = args;
			}
		}
	}
}

void ScaleTo1(float *data, u32 data_len, float *data_normal)
{
	float max;
	uint32_t index;
	
	//	求最值
	arm_max_f32(data,data_len,&max,&index);
	
	//	进行归一化
	arm_scale_f32(data,1.0f/max,data_normal,data_len);	
}

void CharToBinStream(u8 *pSrc, u8 *pDst, u32 pSrc_len,u32 bits_len)
{
	int	i,j,k,m,n;

	for(j=0;j<pSrc_len;j++)
	{
		n = pSrc[j];
		for(i=0;i<bits_len;i++)
		{
			k = n % 2;                               /*取2的余数*/
			m = n / 2;                               /*取被2整除的结果*/
			n = m;                                   /*将得到的商赋给变量n*/
			pDst[bits_len*j+i] = k;   
		}
	}
}

u32 DopplerEstimation(float *data)
{
//	g_1stPoint = 318+96;    //MATLAB仿真结果
	u32 pos2 = 0;
	u32 SynLenRec = g_ReceiveBlock*2;;
	
	float *Cache;
	Cache = mymalloc(SRAMEX,g_ReceiveBlock*4);
	memset(&Cache[0],0,g_ReceiveBlock*4);
	
	float *SigBlk;
	SigBlk = mymalloc(SRAMEX,(g_ReceiveBlock+g_Order)*4);
	memset(&SigBlk[0],0,(g_ReceiveBlock+g_Order)*4);
	
	float *SigMix;
	SigMix = mymalloc(SRAMEX,2*(g_ReceiveBlock+g_Delay)*4);
	memset(&SigMix[0],0,2*(g_ReceiveBlock+g_Delay)*4);
	
	float *BagBlk;
  BagBlk = mymalloc(SRAMEX,4*g_ReceiveBlock*4);
	memset(&BagBlk[0],0,4*g_ReceiveBlock*4);
  
	float *SigBaseDown;
	SigBaseDown = mymalloc(SRAMEX,2*g_FFTLen*4);
	memset(&SigBaseDown[0],0,2*g_FFTLen*4);
	
	float *BagTmpBlk;
	BagTmpBlk = mymalloc(SRAMEX,g_Order*4);
	memset(&BagTmpBlk[0],0,g_Order*4);
	
	for(u8 i=0;i<4;i++)
	{
		  memcpy(&Cache[0],&data[g_1stPoint-96+g_ModuLen+g_ReceiveBlock*(2+i)],g_ReceiveBlock*4);
			memcpy(&SigBlk[g_Order],&Cache[0],g_ReceiveBlock*4);
      memcpy(&SigBlk[0],&BagTmpBlk[0],g_Order*4);
      DownConversion_SYN(&SigBlk[0],g_ReceiveBlock+g_Order,&SigMix[0]);
      memcpy(&BagBlk[2*g_ReceiveBlock],&SigMix[0],2*g_ReceiveBlock*4);
      DownSamplingCplx(&BagBlk[0],2*g_ReceiveBlock,g_DownSamplingNum,&SigBaseDown[0]);
      g_1stSynIsOK = Syn1st(&SigBaseDown[0],g_FFTLen,&g_1stCopy[0],&pos2); 
      if(!g_1stSynIsOK)
      {       
        memcpy(&BagTmpBlk[0],&SigBlk[g_ReceiveBlock-2*g_Delay],g_Order*4);
        memmove(&BagBlk[0],&BagBlk[2*g_ReceiveBlock],2*g_ReceiveBlock*4);
      }
      else
      if(g_1stSynIsOK==1)
      {
        memset(&SigBlk[0],0,(g_ReceiveBlock+g_Order)*4);
        memset(&BagBlk[0],0,(4*g_ReceiveBlock)*4);
        memset(&SigBaseDown[0],0,2*g_FFTLen*4);
				SynLenRec = g_ReceiveBlock*i+pos2-g_Delay+6000;
				break;
      }
	}
	
	myfree(SRAMEX,Cache);	
  myfree(SRAMEX,SigBlk);
	myfree(SRAMEX,SigMix);
	myfree(SRAMEX,BagBlk);
	myfree(SRAMEX,SigBaseDown);
	myfree(SRAMEX,BagTmpBlk);
//	myfree(SRAMEX,g_1stCopyLFM);
	return SynLenRec;
}

void Resample(float *input, u32 inputlen, float *output, u32 outputlen, float frac)
{
	float interp_idx;
	u32 floor_idx;
	float frac_idx;
	for(u32 i=0;i<outputlen;i++)
	{
		interp_idx = i*frac;
		floor_idx = floor(interp_idx);
		frac_idx = interp_idx-floor_idx;
		if(floor_idx>=inputlen-1)
		{
			output[i] = input[i];   //防止越界
		}
		else
		{
			output[i] = input[floor_idx]*(1-frac_idx)+input[floor_idx+1]*frac_idx;
		}
	}
	
	float *Cache;
	Cache = mymalloc(SRAMEX,g_ReceiveBlock*4);
	memset(&Cache[0],0,g_ReceiveBlock*4);
	
	float *SigBlk;
	SigBlk = mymalloc(SRAMEX,(g_ReceiveBlock+g_Order)*4);
	memset(&SigBlk[0],0,(g_ReceiveBlock+g_Order)*4);
	
	float *SigMix;
	SigMix = mymalloc(SRAMEX,2*(g_ReceiveBlock+g_Delay)*4);
	memset(&SigMix[0],0,2*(g_ReceiveBlock+g_Delay)*4);
	
	float *BagBlk;
  BagBlk = mymalloc(SRAMEX,4*g_ReceiveBlock*4);
	memset(&BagBlk[0],0,4*g_ReceiveBlock*4);
  
	float *SigBaseDown;
	SigBaseDown = mymalloc(SRAMEX,2*g_FFTLen*4);
	memset(&SigBaseDown[0],0,2*g_FFTLen*4);
	
	float *BagTmpBlk;
	BagTmpBlk = mymalloc(SRAMEX,g_Order*4);
	memset(&BagTmpBlk[0],0,g_Order*4);
	
	for(u8 i=0;i<5;i++)
	{
		  memcpy(&Cache[0],&output[g_ReceiveBlock*i],g_ReceiveBlock*4);
			memcpy(&SigBlk[g_Order],&Cache[0],g_ReceiveBlock*4);
      memcpy(&SigBlk[0],&BagTmpBlk[0],g_Order*4);
      DownConversion_SYN(&SigBlk[0],g_ReceiveBlock+g_Order,&SigMix[0]);
      memcpy(&BagBlk[2*g_ReceiveBlock],&SigMix[0],2*g_ReceiveBlock*4);
      DownSamplingCplx(&BagBlk[0],2*g_ReceiveBlock,g_DownSamplingNum,&SigBaseDown[0]);
      g_1stSynIsOK = Syn1st(&SigBaseDown[0],g_FFTLen,&g_1stCopy[0],&g_1stPoint); 
      if(!g_1stSynIsOK)
      {       
        memcpy(&BagTmpBlk[0],&SigBlk[g_Order+g_ReceiveBlock-2*g_Delay],2*g_Delay*4);
        memmove(&BagBlk[0],&BagBlk[2*g_ReceiveBlock],2*g_ReceiveBlock*4);
      }
      else
      if(g_1stSynIsOK==1)
      {
        memset(&SigBlk[0],0,(g_ReceiveBlock+g_Order)*4);
        memset(&BagBlk[0],0,(4*g_ReceiveBlock)*4);
        memset(&SigBaseDown[0],0,2*g_FFTLen*4);
				break;
      }
	}
	myfree(SRAMEX,Cache);
	myfree(SRAMEX,SigBlk);
	myfree(SRAMEX,SigMix);
	myfree(SRAMEX,BagBlk);
	myfree(SRAMEX,SigBaseDown);
	myfree(SRAMEX,BagTmpBlk);
}
#endif
