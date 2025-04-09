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

//uint8_t Boot[4] = {'B','o','o','t'};
//uint8_t Send[4] = {'S','e','n','d'};

void SynPickUp(void)
{
	if(g_FirstSynOK==1)
	{
		if(g_RxCnt<g_FrameBlockNum-1)
		{
			memcpy(&g_Backup[(g_RxCnt+2)*6144],&g_Cachef32[0],g_DataBagLen*4);
			g_RxCnt++;
		}
		else if(g_RxCnt==g_FrameBlockNum-1)
		{
			memcpy(&g_Backup[(g_RxCnt+2)*6144],&g_Cachef32[0],g_DataBagLen*4);
			//转移目前这帧数据准备解调，清空备份区等待下一帧数据存入
			memmove(&g_FrameSig[0],&g_Backup[0],g_DataBagLen*(g_FrameBlockNum+2)*sizeof(float));
			memset(&g_Backup[0],0,g_DataBagLen*(g_FrameBlockNum+2)*sizeof(float));
			
//			memmove(g_AdDataSave,g_SigBuchang,25*6144*4);
//			memset(g_SigBuchang,0,25*6144*4);
			
			g_RxCnt = 0;
			g_FirstSynOK = !g_OK;
			g_DecodeEN = 1;
			g_IntMode = Idle;//测试 解码时不进入同步
//			printf("\nSyn OK");
		}
	}
	else
	{
	    g_BackupSlideStore();
		SynDetectTD(&g_Backup[0],g_LFMRefConj,6,g_DataBagLen*2,&g_Pose);  
		if(g_TD == 0)
		{		
			g_FirstSynOK = SynDetect(&g_Backup[0],g_HFMRefConj,6,g_DataBagLen*2,&g_Pose);  
		}
		g_TD = 0;			
//		printf("\nend");
	}
	
}

void Soft_Reset(void)
{
	__set_FAULTMASK(1); 						// 关闭所有中断
	NVIC_SystemReset(); 						// 复位
}

void int31_to_float(int32_t *sound,uint32_t sound_len, float *data)
{
	uint32_t i;
//  float CodeTemp;
	for(i=0;i<sound_len;i++)
	{   
//    CodeTemp = sound[i]^0x800000;
//		data[i] = (CodeTemp - 0x800000)/838860.8f;
		data[i] = sound[i]/8388608.0f;
	}
}

void MemUsed(void)
{
	u16 MemUseSRAMIN = 0;
	u16 MemUseSRAM12 = 0;
	u16 MemUseSRAMEX = 0;	
	u16 MemUseSRAM4 = 0;	
	
	MemUseSRAMIN = my_mem_perused(SRAMIN);
	printf("SRAMIN当前共使用大小 = %d\r\n", MemUseSRAMIN);	
	MemUseSRAM12 = my_mem_perused(SRAM12);
	printf("SRAM12当前共使用大小 = %d\r\n", MemUseSRAM12);	
	MemUseSRAMEX = my_mem_perused(SRAMEX);
	printf("SRAMEX当前共使用大小 = %d\r\n", MemUseSRAMEX);	
	MemUseSRAM4 = my_mem_perused(SRAM4);
	printf("SRAM4当前共使用大小 = %d\r\n", MemUseSRAM4);	
}


/***************************** 声威海洋科技 (END OF FILE) *********************************/

void HFMSignalGen(float T,float *SynSig)
{	
  float tt1 = T*(g_f0/g_fb);
  float K1 = g_fl*(tt1+T/2);
  float t0;
  float Tmp;
  for(u32 i=0;i<(int)(g_fs*T);i++)
  {
    t0 = (float)i/g_fs;
    Tmp = 2*PI*K1*log(1+(t0-T/2)/tt1);
    SynSig[i] = 0.8f*sin(Tmp);
  }
}

void LFMSignalGen(float T,float *SynSig)
{
	//线性调频
	float K0 = 7000/T;
	float t0;
	float Tmp;
	for(u32 i=0;i<(int)(g_fs*T);i++)
	{
		t0 = (float)i/g_fs;
		Tmp = 2*PI*7000*t0+K0*PI*t0*t0;
		SynSig[i] = 0.6f*arm_cos_f32(Tmp);
	}
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





u32 DopplerEstimation(float *data)
{
	u32 pos2 = 0;
	u32 pos3 = 0;
	u32 SynLenRec = 5500;
    u8  SynIsOK = 0;
	
	for(char i=0;i<2;i++)
	{
		SynIsOK = SynDetect(&data[i*g_DataBagLen],g_HFMRefConj,6,g_DataBagLen*2,&pos2);  
		
		if(SynIsOK==1)
		{
			pos3 = Syn2nd(&data[pos2+i*g_DataBagLen-g_BuchangCont],g_BuchangCont*2+1);
			pos2 = pos2+pos3-g_BuchangCont;
			SynLenRec = pos2;//当前这一段信号的同步头位置（相对位置）
			SynIsOK = 0;
			break;			
		}
		
	}
	return SynLenRec;
}

u32 Resample(float *input, u32 inputlen, float *output, u32 outputlen, float frac)
{
	u32 Pos_Block;
	u32 Pos = 0;
	float interp_idx;
	u32 floor_idx;
	float frac_idx;
	u8  SynIsOK = 0;
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


	for(char i=0;i<5;i++)
	{
		SynIsOK = SynDetect(&output[i*g_DataBagLen],g_HFMRefConj,6,g_DataBagLen*2,&Pos_Block);  
		
		if(SynIsOK==1)
		{
			Pos = i*g_DataBagLen+Pos_Block;
			SynIsOK = 0;
			break;			
		}		
	}
	return Pos;
}

void BinToChar (u8 *pSrc, u8 *pDst, u32 pSrc_len,u32 bits_len)
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



void FirFilter(float *pSrc,u32 SrcLen,float *Coeff,u32 CoeffLen,float *pDst,u32 blockSize)
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


void BinToM_Code(u8 *pSrc, u8 *pDst, u32 SrcLen,u32 M_Code)
{
	u32 args,a;
	for(u32 i=0;i<SrcLen/M_Code;i++)
	{
		args = 0;
		for(u32 j=0;j<M_Code;j++)
		{
			a = pSrc[M_Code*i+(M_Code-1)-j]*pow(2,j);
			args = args + a;
			if(j==M_Code-1)
			{
				pDst[i] = args;
			}
		}
	}
}


void arm_real_mat_mult_cmplx_mat_f32(float *pSrcA,u32 RowA,u32 ColumnA,float *pSrcB,u32 RowB,u32 ColumnB,float *pDst,u32 RowC,u32 ColumnC)
{	
	float *CmplxB;
	CmplxB = mymalloc(SRAMEX,2*RowA*ColumnA*4);		
	memset(&CmplxB[0],0,2*RowA*ColumnA*4);
	
	for(u32 i=0;i<RowA*ColumnA;i++)
	{
		CmplxB[2*i] = pSrcA[i];
	}
	arm_matrix_instance_f32 CmplxMatA;
	arm_matrix_instance_f32 CmplxMatB;
	arm_matrix_instance_f32 CmplxMatC;
	
	arm_mat_init_f32(&CmplxMatA,RowA,ColumnA,CmplxB);
	arm_mat_init_f32(&CmplxMatB,RowB,ColumnB,pSrcB);
	arm_mat_init_f32(&CmplxMatC,RowC,ColumnC,pDst);
	
	arm_mat_cmplx_mult_f32(&CmplxMatA,&CmplxMatB,&CmplxMatC);
	
	myfree(SRAMEX,CmplxB);
}

void M_CodeToBin (u32 *M_Code, u8 *Bin, u32 M_CodeLen,u32 M)
{
	int	i,j,k,m,n;

	for(j=0;j<M_CodeLen;j++)
	{
		n = M_Code[j];
		for(i=0;i<M;i++)
		{
			k = n % 2;     /*取2的余数*/
			m = n / 2;     /*取被2整除的结果*/
			n = m;         /*将得到的商赋给变量n*/
			Bin[M*j+(M-1)-i] = k;   
		}
	}
}





/**
  *	FunctionAim：16进制转2进制
  *	pSrc：
  *	pSrc_len：
  *	pDst：
*/
void HEXToBin(u8 *HEX,u32 HEXLEN,u8 *BIN)
{
	int	i,j,k,m,n;

	for(j=0;j<HEXLEN;j++)
	{
		n = HEX[j];
		for(i=0;i<8;i++)
		{
			k = n % 2;     /*取2的余数*/
			m = n / 2;     /*取被2整除的结果*/
			n = m;         /*将得到的商赋给变量n*/
			BIN[8*j+i] = k;   
		}
	}
}

/**
  *	FunctionAim：复信号实部虚部组合
  *	CplxSig：输出复信号
  *	RealSig：输入实部信号
  *	ImagSig：输入虚部信号
  *	SigLen：输入信号长度
*/
void RealImagCombine(float *CplxSig,float *RealSig,float *ImagSig,u32 SigLen)
{
	for(u32 i=0;i<2*SigLen;i++)
	{
		if(i%2==0)
		CplxSig[i] = RealSig[i/2];
        else
        CplxSig[i] = ImagSig[(i-1)/2];			
	}
}

/**
  *	FunctionAim：数字下变频
  *	SigIn：输入带通信号
  *	SigLen：带通信号长度
  *	SigOut：输出基带信号
*/
void DDCF(float *SigIn,float *SigOut,u32 SigLen)
{
	//滤波块长度为数据包长度加上滤波器延时长度
	const u32 BlockSize = SigLen+g_LowPassFilterOrder/2;
	//输入信号下变频，根据欧拉定理将复指数信号转换为正弦信号与输入信号分别相乘
	float *sig_mix_real;
	float *sig_mix_imag;
	sig_mix_real = mymalloc(SRAM12,BlockSize*4);//48.0625k
	sig_mix_imag = mymalloc(SRAM12,BlockSize*4);//48.0625k
	memset(sig_mix_real,0,BlockSize*4);//memset(&sig_mix_real[0],0,g_DataBagLen);
	memset(sig_mix_imag,0,BlockSize*4);//memset按字节对内存进行初始化，此处初始化可能没有意义，因为内存马上被覆盖
	for(u32 i=0;i<(SigLen/8);i++)
	{
		arm_mult_f32(&SigIn[8*i],&g_CosineF0[0],&sig_mix_real[8*i],8);
		arm_mult_f32(&SigIn[8*i],&g_NegSineF0[0],&sig_mix_imag[8*i],8);   
	}
//	printf("\n 1");
	//对信号实部、虚部分别进行低通滤波；
	float *band_sig_real;
	float *band_sig_imag;
	float *FirState;
	
	band_sig_real = mymalloc(SRAMIN,BlockSize*4);  //48.0625k           //滤波前共耗时12ms
	band_sig_imag = mymalloc(SRAMIN,BlockSize*4); //48.0625k
	FirState = mymalloc(SRAMIN,(g_LowPassFilterOrder+BlockSize)*4);//48.0625k
//	printf("\n 2");
	memset(&band_sig_real[0],0,BlockSize*4);
    memset(&band_sig_imag[0],0,BlockSize*4);
	memset(&FirState[0],0,(g_LowPassFilterOrder+BlockSize)*4);
	
	arm_fir_instance_f32 S;
	arm_fir_init_f32(&S,g_LowPassFilterOrder+1,&g_LowpassFirCoeffs[0],&FirState[0],BlockSize);
	arm_fir_f32(&S,sig_mix_real,band_sig_real,BlockSize);
	arm_fir_f32(&S,sig_mix_imag,band_sig_imag,BlockSize);
//	printf("\n 3");
	//提前释放内存，减少内存压力
	myfree(SRAMIN,FirState);
	myfree(SRAM12,sig_mix_real);
	myfree(SRAM12,sig_mix_imag);
	
	//将信号实部与虚部相结合，实部为偶数位，虚部为奇数位
    float *CplxSig;
	CplxSig = mymalloc(SRAM12,BlockSize*4*2); //96.125k
	memset(&CplxSig[0],0,BlockSize*4*2);
	RealImagCombine(CplxSig,band_sig_real,band_sig_imag,BlockSize);
	memcpy(&SigOut[0],&CplxSig[g_LowPassFilterOrder/2*2],SigLen*4*2);
//	printf("\n 4");

	myfree(SRAMIN,band_sig_real);
	myfree(SRAMIN,band_sig_imag);
	myfree(SRAM12,CplxSig);
}


void CplxDownSamp(float *SigIn,float *SigOut,u8 SampNum,u32 SigLen)
{
	for(u32 i=0;i<(SigLen/SampNum);i++)
	{
		SigOut[2*i] = SigIn[2*i*SampNum];
		SigOut[2*i+1] = SigIn[2*i*SampNum+1];
	}
}



/**
  *	FunctionAim：本地Lfm信号生成
  *	LFM：输出线性调频信号
*/
void LfmGen(float *LFM)
{
	float LfmDuration = (float)g_LFMLen/g_fs;
	float k = (g_fh-g_fl)/LfmDuration;
	float t;
	for(u32 i=0;i<g_LFMLen;i++)
	{
		t=(float)i/g_fs;
		LFM[i] = (float)arm_cos_f32(2*PI*g_fl*t+k*PI*t*t);
	}	
}


/**
  *	FunctionAim：频域同步LFM生成(复信号)
  *	LfmIn：输入LFM信号
  *	FreqLfmOut：输出频域LFM信号
  *	SampNum：降采样倍数
  *	SigLen：输入float数据的个数
*/
void CplxFreqSampSynGen(float *LfmIn,float *FreqLfmOut,u8 SampNum,u32 SigLen)
{   
	//下变频
	float *SigLowpass;
	SigLowpass = mymalloc(SRAMEX,SigLen*4*2);//96k
	memset(&SigLowpass[0],0,SigLen*4*2);
    DDCF(LfmIn,SigLowpass,SigLen);                  //86ms
	//降采样
	float *SigLowSamp; //16k
	SigLowSamp = mymalloc(SRAMIN,SigLen*4*2/SampNum);
	memset(&SigLowSamp[0],0,SigLen*4*2/SampNum);
	CplxDownSamp(SigLowpass,SigLowSamp,SampNum,SigLen*2); //2ms,之后到函数结尾共用4ms
	//共轭
	float *SigLowSampConj;//16k
	SigLowSampConj = mymalloc(SRAMIN,SigLen*4*2/SampNum);
	memset(&SigLowSampConj[0],0,SigLen*4*2/SampNum);
	arm_cmplx_conj_f32(SigLowSamp,SigLowSampConj,SigLen/SampNum);
	//向量乘法
	float RealResult;
	float ImagResult;
	arm_cmplx_dot_prod_f32 (SigLowSamp,SigLowSampConj,SigLen/SampNum,&RealResult,&ImagResult);
	//开方
	float SqrtResult;
	arm_sqrt_f32(RealResult,&SqrtResult);
	//复数FFT
	float *SqrtSig; //16k
	SqrtSig = mymalloc(SRAMIN,2048*4*2);
	memset(&SqrtSig[0],0,2048*4*2);
	arm_scale_f32(SigLowSamp,1/SqrtResult,&SqrtSig[0],SigLen*2/SampNum);
	arm_cfft_f32(&arm_cfft_sR_f32_len2048,&SqrtSig[0],0,1);//信号长度不管，只做4096点FFT

    memcpy(FreqLfmOut,SqrtSig,2048*4*2);
	//arm_cmplx_conj_f32(SqrtSig,FreqLfmOut,SigLen/SampNum*2);	Conj在函数外做，生成本地LFM和接收信号的同步皆可用此函数
	myfree (SRAMEX,SigLowpass);
	myfree (SRAMIN,SigLowSamp);
	myfree (SRAMIN,SigLowSampConj);
	myfree (SRAMIN,SqrtSig);

}

/**
  *	FunctionAim：生成本地同步参考信号
  *	SigOut：输出同步参考信号
*/
void LocalSynRefGen_HFM(float *SigOut)
{
	float *HFM;
	float *CplxFreqSynHfm;
	HFM = mymalloc(SRAMIN,g_DataBagLen*4);
	CplxFreqSynHfm = mymalloc(SRAMIN,2048*4*2);
	memset(&HFM[0],0,g_DataBagLen*4);
	memset(&CplxFreqSynHfm[0],0,2048*4*2);
	
	HFMSignalGen(0.064f,HFM);
	CplxFreqSampSynGen(&HFM[0],&CplxFreqSynHfm[0],6,g_DataBagLen);
	arm_cmplx_conj_f32(CplxFreqSynHfm,SigOut,2048);
	
	myfree(SRAMIN,HFM);
	myfree(SRAMIN,CplxFreqSynHfm);
}

void LocalSynRefGen_LFM(float *SigOut)
{
	float *LFM;
	float *CplxFreqSynHfm;
	LFM = mymalloc(SRAMIN,g_DataBagLen*4);
	CplxFreqSynHfm = mymalloc(SRAMIN,2048*4*2);
	memset(&LFM[0],0,g_DataBagLen*4);
	memset(&CplxFreqSynHfm[0],0,2048*4*2);
	
	LFMSignalGen(0.064f,LFM);
	CplxFreqSampSynGen(&LFM[0],&CplxFreqSynHfm[0],6,g_DataBagLen);
	arm_cmplx_conj_f32(CplxFreqSynHfm,SigOut,2048);
	
	myfree(SRAMIN,LFM);
	myfree(SRAMIN,CplxFreqSynHfm);
}

/**
  *	FunctionAim：频域同步LFM生成(复信号)
  *	SigIn：输入信号
  *	SynFreqConj：共轭后的本地同步参考信号
  *	SampNum：降采样倍数
  *	SigLen：输入float数据的个数
*/
u8 SynDetect(float *SigIn,float *SynFreqConj,u8 SampNum,u32 SigLen,u32 *synpose)
{
	u32 Pose;	
	float *SigSampFreq;
	SigSampFreq = mymalloc(SRAMIN,2048*4*2);//16k
	CplxFreqSampSynGen(SigIn,SigSampFreq,SampNum,SigLen);//35ms
	//向量点乘，IFFT
	float *CorrFFT;
	CorrFFT = mymalloc(SRAMIN,2048*4*2);//16k
	memset(&CorrFFT[0],0,2048*4*2);
    arm_cmplx_mult_cmplx_f32(SigSampFreq, SynFreqConj, CorrFFT, 2048);

	arm_cfft_f32(&arm_cfft_sR_f32_len2048,&CorrFFT[0],1,1); //4ms
	float *Corr;
	Corr = mymalloc(SRAMIN,2048*4);//16k
	memset(&Corr[0],0,2048*4);
	arm_cmplx_mag_f32(&CorrFFT[0],&Corr[0],2048);
	//求相关峰位置
	float Peak;
	arm_max_f32(&Corr[0],2048,&Peak,&Pose);
	myfree (SRAMIN,SigSampFreq);
	myfree (SRAMIN,CorrFFT);
	myfree (SRAMIN,Corr);
	
	if(Peak>0.35f)
	{
	    *synpose = (Pose-1)*6;
		return g_OK;
	}
	else
	{
		return 0;
	}

}

void SynDetectTD(float *SigIn,float *SynFreqConj,u8 SampNum,u32 SigLen,u32 *synpose)
{
	u32 Pose;	
	float *SigSampFreq;
	SigSampFreq = mymalloc(SRAMIN,2048*4*2);//16k
	CplxFreqSampSynGen(SigIn,SigSampFreq,SampNum,SigLen);//35ms
	//向量点乘，IFFT
	float *CorrFFT;
	CorrFFT = mymalloc(SRAMIN,2048*4*2);//16k
	memset(&CorrFFT[0],0,2048*4*2);
    arm_cmplx_mult_cmplx_f32(SigSampFreq, SynFreqConj, CorrFFT, 2048);

	arm_cfft_f32(&arm_cfft_sR_f32_len2048,&CorrFFT[0],1,1); //4ms
	float *Corr;
	Corr = mymalloc(SRAMIN,2048*4);//16k
	memset(&Corr[0],0,2048*4);
	arm_cmplx_mag_f32(&CorrFFT[0],&Corr[0],2048);
	//求相关峰位置
	float Peak;
	arm_max_f32(&Corr[0],1024,&Peak,&Pose);
	myfree (SRAMIN,SigSampFreq);
	myfree (SRAMIN,CorrFFT);
	myfree (SRAMIN,Corr);
	
	if(Peak>0.25f)
	{
		g_TD = 1;
		if(g_TDMaster==0)
		{
			g_TDSlave = 1;
			g_TDPose = (Pose)*6;
//			SysAnswerCmd.TD_SLAVE_ANSWER[4] = g_TDPose/256;
//			SysAnswerCmd.TD_SLAVE_ANSWER[5] = g_TDPose%256;
//			printf("%d\r\n",1);
//			printf("%f\r\n",Peak);
//			Uart1_Puts(&SysAnswerCmd.TD_SLAVE_ANSWER[0],8);
		}
		else
		{
			g_TDPose = (Pose)*6;			
			g_TDSuccess = 1;
		}
		g_CommunicationMode = TD;
		g_TDSlaveNum = 1;
		
		g_IntMode = Idle;//测试
	}
	else
	{
//		return 0;
	}

}

u32 Syn2nd(float *data,u8 cont)
{
	float *SumResult;
	SumResult = mymalloc(SRAMIN,cont*4);
	memset(&SumResult[0],0,cont*4);	
	float SumResultTem = 0;
	float peak;
	u32 pos;
	for(u32 j=0;j<cont;j++)
	{
		for(u32 i=0;i<g_ReceiveBlock;i++)
		{
			SumResultTem = data[i+j]*g_SynHFM[i];
			SumResult[j] = SumResult[j]+SumResultTem;
		}
	}
	arm_max_f32(&SumResult[0],cont,&peak,&pos);
	myfree(SRAMIN,SumResult);
	return pos;
}

/**
  *	FunctionAim：滑动内存块以便于同步
*/
void g_BackupSlideStore(void)
{
	memmove(&g_Backup[0],&g_Backup[g_DataBagLen],g_DataBagLen*4);
	memmove(&g_Backup[g_DataBagLen],&g_Cachef32[0],g_DataBagLen*4);
}

void ModulateMaSS(u8 *UartHex,u32 UartHexLen,float *MaSS_Sig)
{
	u32 FrameNum;
//  u32 i,j;
  
	u8 *UartBIN;
	UartBIN = mymalloc(SRAMIN,g_TotalBitNumPerFarme);		
	memset(&UartBIN[0],0,g_TotalBitNumPerFarme);
	
  //【04】帧结构计算
	FrameStructGen(UartHex,UartHexLen,&FrameNum,UartBIN);
	
	u8 *FrameBin;
	FrameBin = mymalloc(SRAMIN,g_TotalBitNumPerFarme*FrameNum);		
	memset(&FrameBin[0],0,g_TotalBitNumPerFarme*FrameNum);
	
	u8 *TotalBitMaryPerFrameDec;
	TotalBitMaryPerFrameDec = mymalloc(SRAMIN,(g_InfoSymbolNumPerFrameMASS+2));		
	memset(&TotalBitMaryPerFrameDec[0],0,(g_InfoSymbolNumPerFrameMASS+2));
	
	u8 *InfoBitMaryPerFrameDec;
	InfoBitMaryPerFrameDec = mymalloc(SRAMIN,g_InfoSymbolNumPerFrameMASS);		
	memset(&InfoBitMaryPerFrameDec[0],0,g_InfoSymbolNumPerFrameMASS);
	
	float *SigModulated;
	SigModulated = mymalloc(SRAMEX,g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame*4);		
	memset(&SigModulated[0],0,g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame*4);
	
//  char *SpreadCode;
//  SpreadCode = osRtxMemoryAlloc(MemD1,g_M*g_SpreadCodeLen,0);		
//	memset(&SpreadCode[0],0,g_M*g_SpreadCodeLen);
//	
//	char *SpreadCodeCopy; //内存变化，SpreadCode复制后使用
//  SpreadCodeCopy = osRtxMemoryAlloc(MemD1,g_M*g_SpreadCodeLen,0);		
//	memset(&SpreadCodeCopy[0],0,g_M*g_SpreadCodeLen);
  
	memcpy(FrameBin,UartBIN,8*UartHexLen);
	
//	for(i=0;i<g_M;i++)
//  {
//    for(j=0;j<g_SpreadCodeLen;j++)
//    {
//      SpreadCode[g_SpreadCodeLen*i+j] = g_SpreadCode[g_M*j+i];
//    }
//  }
//	memcpy(SpreadCodeCopy,SpreadCode,g_M*g_SpreadCodeLen);
  
	for(u32 FrameIndex=0;FrameIndex<FrameNum;FrameIndex++)
	{
    //【05】信息映射
		InfoGen(FrameBin,FrameIndex,TotalBitMaryPerFrameDec,&InfoBitMaryPerFrameDec[0]);
    
    //【06】调制
    MaSSModulate(TotalBitMaryPerFrameDec,g_SpreadCode,SigModulated);   
    
    //【07】添加同步信号
    FormSigStructureMASS(SigModulated,g_ModuLen,MaSS_Sig);
	}
		
	myfree(SRAMIN,UartBIN);
	myfree(SRAMIN,FrameBin);
	myfree(SRAMIN,TotalBitMaryPerFrameDec);
	myfree(SRAMIN,InfoBitMaryPerFrameDec);
	myfree(SRAMEX,SigModulated);
//  myfree(SRAMEX,SpreadCode);
}

void DemodulateMaSS(float *SigReceived,u32 Pos)
{
	u8 *UratHex;
	UratHex = mymalloc(SRAMIN,g_InfoByteNumPerFrame+10);      
	memset(&UratHex[0],0,g_InfoByteNumPerFrame+10);

  //解码 
	MassDemodulate(&SigReceived[Pos+g_ReceiveBlock*4],g_ModuLen,&UratHex[0]);

	u8 DecodedLen;
	DecodedLen = UratHex[0];
//	Uart1_Puts(&UratHex[0],32);
	DecodeLogSave(&UratHex[0],15);
//	if(g_save == 1)
//	{
//		s32 *g_BackUps;
//		g_BackUps = mymalloc(SRAMEX,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
//		memset(&g_BackUps[0],0,g_ReceiveBlock*(g_ReceiveBlockNumber+2)*4);
//		for(u32 i=0;i<g_ReceiveBlock*(g_ReceiveBlockNumber+2);i++)
//		{
//			g_BackUps[i] = SigReceived[i]*21296;
//		}
//		DataSaveDecode(g_BackUps,g_ReceiveBlock*(g_ReceiveBlockNumber+2));
//		myfree(SRAMEX,g_BackUps);
//	}
	if(XORCaculate(&UratHex[0],DecodedLen+1) == UratHex[DecodedLen+1])
	{
		UratHex[0] = 0xFA;
		UratHex[DecodedLen+1] = 0xEF;
		UratHex[DecodedLen+2] = 0x0A;
		Uart1_Puts(&UratHex[0],DecodedLen+3);
	}
	myfree(SRAMEX,UratHex);
}	

void FrameStructGen(u8 *UartHex,u32 UartHexLen,u32 *FrameNum,u8 *UartBIN)
{
	u32 Remainder,UartBINLen;
	
	UartBINLen = UartHexLen*8;
	Remainder = UartBINLen%g_TotalBitNumPerFarme;
	
	*FrameNum = (Remainder==0)? UartBINLen/(g_TotalBitNumPerFarme):floor(UartBINLen/(g_TotalBitNumPerFarme))+1;
	
	HEXToBin(UartHex,UartHexLen,UartBIN);
	
}

void InfoGen(u8 *FrameBin,u32 FrameIndex,u8 *TotalBitMaryPerFrameDec,u8 *InfoBitMaryPerFrameDec)
{
  BinToM_Code(FrameBin,&InfoBitMaryPerFrameDec[0],g_r*g_TotalBitNumPerFarme,g_r);
  for(u32 i=0;i<g_TotalBitNumPerFarme;i++)
  {
    TotalBitMaryPerFrameDec[2+i] = InfoBitMaryPerFrameDec[i] + 1;
  }
  TotalBitMaryPerFrameDec[0] = 1;
  TotalBitMaryPerFrameDec[1] = 2;
}

void MaSSModulate(u8 *TotalBitMaryPerFrameDec,char *SpreadCode,float *Signal)
{
  float Max;
  u32 Index;
	u16 PulseShapeLen = 6*g_UpSamplingMultipleMASS+1;
  
	u8 *MaSS_Sequence;
  MaSS_Sequence = mymalloc(SRAMEX,g_SpreadCodeLen*g_TotalSymbolNumPerFrame);
	memset(&MaSS_Sequence[0],0,g_SpreadCodeLen*g_TotalSymbolNumPerFrame);
  
  float *UpSamping;
  UpSamping = mymalloc(SRAMEX,(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame+PulseShapeLen/2)*4);
	memset(&UpSamping[0],0,(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame+PulseShapeLen/2)*4);
  
  float *SigPulseShape;
  SigPulseShape = mymalloc(SRAMEX,(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame+PulseShapeLen/2)*4);
	memset(&SigPulseShape[0],0,(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame+PulseShapeLen/2)*4);
  
  float *SigModulated;
  SigModulated = mymalloc(SRAMEX,(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame)*4);
	memset(&SigModulated[0],0,(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame)*4);
  
  for(u32 num=0;num<g_TotalSymbolNumPerFrame;num++)
  {
    memcpy(&MaSS_Sequence[num*g_SpreadCodeLen],&SpreadCode[g_SpreadCodeLen*(TotalBitMaryPerFrameDec[num]-1)],g_SpreadCodeLen);
  }
  
  //插值
  for(u32 i=0;i<g_SpreadCodeLen*g_TotalSymbolNumPerFrame;i++)
  {
    UpSamping[g_UpSamplingMultipleMASS*i] = (float)MaSS_Sequence[i]-1;
  }
  
  //整形滤波
  FirFilter(UpSamping,g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame+PulseShapeLen/2,g_PulseShape,PulseShapeLen,SigPulseShape,PulseShapeLen/2);
  
  //调制
  for(u32 i=0;i<(g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame)/8;i++)
  {
    arm_mult_f32(&SigPulseShape[PulseShapeLen/2+8*i],&CarryCos[0],&SigModulated[8*i],8);
  }
  
  //归一化
  arm_max_f32(&SigModulated[0],g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame,&Max,&Index);
  arm_scale_f32(&SigModulated[0],1/Max,&Signal[0],g_UpSamplingMultipleMASS*g_SpreadCodeLen*g_TotalSymbolNumPerFrame);
  
  myfree(SRAMEX,MaSS_Sequence);
  myfree(SRAMEX,UpSamping);
  myfree(SRAMEX,SigPulseShape);
  myfree(SRAMEX,SigModulated);
}

void FormSigStructureMASS(float *SigInput,u32 SigLen,float *SigOutput)
{	
	memcpy(&SigOutput[0],g_SynHFM,g_ReceiveBlock*4);
	
	if(g_DopplerEstimation == 1)
	{
//		memcpy(&SigOutput[g_ReceiveBlock*2],SigInput,SigLen*4);
//		memcpy(&SigOutput[g_ReceiveBlock*3+SigLen],g_SynHFM,g_ReceiveBlock*4);		
		memcpy(&SigOutput[g_ReceiveBlock*2],g_SynHFM,g_ReceiveBlock*4);
		memcpy(&SigOutput[g_ReceiveBlock*4],SigInput,SigLen*4);		
	}
	else
	{
		memcpy(&SigOutput[g_ReceiveBlock*2],SigInput,SigLen*4);	
	}
}

void MassDemodulate(float *Sig,u32 SigLen,u8 *Decoded)
{
  u32 i,ii;
  float Max;

  float *SignalDemodulateReal;
  SignalDemodulateReal = mymalloc(SRAMEX,SigLen*4);
	memset(&SignalDemodulateReal[0],0,(SigLen+32)*4);
  
  float *SignalDemodulateImag;
  SignalDemodulateImag = mymalloc(SRAMEX,SigLen*4);
	memset(&SignalDemodulateImag[0],0,(SigLen+32)*4);
  
  float *BaseSigReal;
  BaseSigReal = mymalloc(SRAMEX,(SigLen+g_Delay)*4);
	memset(&BaseSigReal[0],0,(SigLen+g_Delay)*4);
  
  float *BaseSigImag;
  BaseSigImag = mymalloc(SRAMEX,(SigLen+g_Delay)*4);
	memset(&BaseSigImag[0],0,(SigLen+g_Delay)*4);
  
  float *SigDownResmaleReal;
  SigDownResmaleReal = mymalloc(SRAMIN,(SigLen/g_UpSamplingMultipleMASS)*4);
	memset(&SigDownResmaleReal[0],0,(SigLen/g_UpSamplingMultipleMASS)*4);
  
  float *SigDownResmaleImag;
  SigDownResmaleImag = mymalloc(SRAMIN,(SigLen/g_UpSamplingMultipleMASS)*4);
	memset(&SigDownResmaleImag[0],0,(SigLen/g_UpSamplingMultipleMASS)*4);
  
  float *DeSpreadCode;
  DeSpreadCode = mymalloc(SRAMIN,g_M*g_SpreadCodeLen*4);
	memset(&DeSpreadCode[0],0,g_M*g_SpreadCodeLen*4);
  
  float *PolitSig;
  PolitSig = mymalloc(SRAMIN,2*g_SpreadCodeLen*4);
	memset(&PolitSig[0],0,2*g_SpreadCodeLen*4);
  
  float *SigTemp;
  SigTemp = mymalloc(SRAMIN,2*192*4);
	memset(&SigTemp[0],0,2*192*4);
  
  float *DeSpreadCodeAddPolit;
  DeSpreadCodeAddPolit = mymalloc(SRAMIN,32*192*4);
	memset(&DeSpreadCodeAddPolit[0],0,32*192*4);
  
  float *DeSpreadCodeMatrix;
  DeSpreadCodeMatrix = mymalloc(SRAMIN,160*192*4);
	memset(&DeSpreadCodeMatrix[0],0,160*192*4);
  
  float *y;
  y = mymalloc(SRAMIN,160*2*4);
	memset(&y[0],0,160*2*4);
  
  float *sig_y;
  sig_y = mymalloc(SRAMIN,160*4);
	memset(&sig_y[0],0,160*4);
  
  u32 *Pose;
  Pose = mymalloc(SRAMIN,g_InfoSymbolNumPerFrameMASS*4);
	memset(&Pose[0],0,g_InfoSymbolNumPerFrameMASS*4);
  
  u32 *PosMary;
  PosMary = mymalloc(SRAMIN,g_InfoSymbolNumPerFrameMASS*4);
	memset(&PosMary[0],0,g_InfoSymbolNumPerFrameMASS*4);
  
  u8 *BinData;
  BinData = mymalloc(SRAMIN,g_TotalBitNumPerFarme);
	memset(&BinData[0],0,g_TotalBitNumPerFarme);
  
//  for(u32 i=0;i<SigLen/8;i++)
//  {
//    arm_mult_f32(&Sig[8*i],&CarryCos[0],&SignalDemodulateReal[8*i],8);
//    arm_mult_f32(&Sig[8*i],&CarrySin[0],&SignalDemodulateImag[8*i],8);
//  }
  for(i=0;i<SigLen;i++)
  {
		SignalDemodulateReal[i] = Sig[i]*CarryCos[i%8];
		SignalDemodulateImag[i] = Sig[i]*(-1)*CarrySin[i%8];
  }
  
  //低通滤波
	FirFilter(SignalDemodulateReal,(SigLen+g_Delay),g_LowPassCoeffDemodu,g_Order+1,BaseSigReal,g_Delay);
  FirFilter(SignalDemodulateImag,(SigLen+g_Delay),g_LowPassCoeffDemodu,g_Order+1,BaseSigImag,g_Delay);
  
  //降采样
  for(i=0;i<SigLen/g_UpSamplingMultipleMASS;i++)
  {
    arm_mean_f32(&BaseSigReal[g_Delay+g_UpSamplingMultipleMASS*i],g_UpSamplingMultipleMASS,&SigDownResmaleReal[i]);
    SigDownResmaleReal[i] = SigDownResmaleReal[i] * g_UpSamplingMultipleMASS;
    
    arm_mean_f32(&BaseSigImag[g_Delay+g_UpSamplingMultipleMASS*i],g_UpSamplingMultipleMASS,&SigDownResmaleImag[i]);
    SigDownResmaleImag[i] = SigDownResmaleImag[i] * g_UpSamplingMultipleMASS;
  }
  
  //MSJMD
  for(i=0;i<g_M*g_SpreadCodeLen;i++)
  {
//		printf("%d\r\n",i);
    DeSpreadCode[i] = g_SpreadCode[i]-1;
  }
  
  memcpy(PolitSig,DeSpreadCode,128*4);
  
  for(ii=g_PolitSymbolNumPerFrame+1;ii<g_TotalSymbolNumPerFrame+1;ii++)
  {
    for(i=0;i<192;i++)
    {
      SigTemp[2*i+0] = SigDownResmaleReal[(ii-1-g_PolitSymbolNumPerFrame)*g_SpreadCodeLen+i];
      SigTemp[2*i+1] = SigDownResmaleImag[(ii-1-g_PolitSymbolNumPerFrame)*g_SpreadCodeLen+i];  
    }
    
    for(i=0;i<g_M;i++)
    {
      memcpy(&DeSpreadCodeAddPolit[192*i],&PolitSig[0],128*4);
      memcpy(&DeSpreadCodeAddPolit[192*i+128],&DeSpreadCode[64*i],64*4);
    }
    
    DeSpreadCodeMatrixGen(DeSpreadCodeAddPolit,DeSpreadCodeMatrix);
    
    arm_real_mat_mult_cmplx_mat_f32(DeSpreadCodeMatrix,160,192,SigTemp,192,1,y,160,1);
    arm_cmplx_mag_squared_f32(y,sig_y,160);
    arm_max_f32(sig_y,160,&Max,&Pose[ii-(g_PolitSymbolNumPerFrame+1)]);
    PosMary[ii-(g_PolitSymbolNumPerFrame+1)] = Pose[ii-(g_PolitSymbolNumPerFrame+1)]%g_M;
    
    memmove(&PolitSig[0],&PolitSig[g_SpreadCodeLen],64*4);
    memcpy(&PolitSig[64],&DeSpreadCode[(PosMary[ii-(g_PolitSymbolNumPerFrame+1)])*64],64*4);
    
  }
  
  //进制转化
  M_CodeToBin(PosMary,BinData,g_InfoSymbolNumPerFrameMASS,g_r);
  BinToChar(BinData,Decoded,g_TotalBitNumPerFarme,8);
  
  myfree(SRAMEX,SignalDemodulateReal);
  myfree(SRAMEX,SignalDemodulateImag);
  myfree(SRAMEX,BaseSigReal);
  myfree(SRAMEX,BaseSigImag);
  myfree(SRAMIN,SigDownResmaleReal);
  myfree(SRAMIN,SigDownResmaleImag);
  myfree(SRAMIN,DeSpreadCode);
  myfree(SRAMIN,PolitSig);
  myfree(SRAMIN,SigTemp);
  myfree(SRAMIN,DeSpreadCodeAddPolit);
  myfree(SRAMIN,DeSpreadCodeMatrix);
  myfree(SRAMIN,y);
  myfree(SRAMIN,sig_y);
  myfree(SRAMIN,Pose);
  myfree(SRAMIN,PosMary);
  myfree(SRAMIN,BinData);
}

void DeSpreadCodeMatrixGen(float *DeSpreadCode,float *DeSpreadCodeMatrix)
{
  u32 t;
  
  float *Tmp;
  Tmp = mymalloc(SRAMIN,32*192*4);
	memset(&Tmp[0],0,32*192*4);
  
  float *Tmp1;
  Tmp1 = mymalloc(SRAMIN,32*2*4);
	memset(&Tmp1[0],0,32*2*4);
  
  float *Tmp2;
  Tmp2 = mymalloc(SRAMIN,32*192*4);
	memset(&Tmp2[0],0,32*192*4);
  
  arm_matrix_instance_f32 TMP;
  arm_matrix_instance_f32 TMP_TRANS;
  arm_matrix_instance_f32 TMPT;
  
  arm_mat_init_f32(&TMP,32,192,DeSpreadCode);
  arm_mat_init_f32(&TMP_TRANS,192,32,Tmp);
  arm_mat_init_f32(&TMPT,32,192,Tmp2);
  
  for(t=0;t<g_SlidingInterval;t++)
  {
    //先转置
    arm_mat_trans_f32(&TMP,&TMP_TRANS);
    
    //移动
    memcpy(Tmp1,&Tmp[32*(190+t)],(2-t)*32*4);
    memmove(&Tmp[(2-t)*32],&Tmp[0],(190+t)*32*4);
    memcpy(&Tmp[0],Tmp1,(2-t)*32*4);
    
    //再转置
    arm_mat_trans_f32(&TMP_TRANS,&TMPT);
    memcpy(&DeSpreadCodeMatrix[32*192*t],Tmp2,32*192*4);
  }
  
  memcpy(&DeSpreadCodeMatrix[2*32*192],DeSpreadCode,32*192*4);

  for(t=0;t<g_SlidingInterval;t++)
  {
    //先转置
    arm_mat_trans_f32(&TMP,&TMP_TRANS);
    
    //移动
    memcpy(Tmp1,&Tmp[0],(t+1)*32*4);
    memmove(&Tmp[0],&Tmp[32*(t+1)],(191-t)*32*4);
    memcpy(&Tmp[(191-t)*32],Tmp1,(t+1)*32*4);
    
    //再转置
    arm_mat_trans_f32(&TMP_TRANS,&TMPT);
    memcpy(&DeSpreadCodeMatrix[3*32*192+32*192*t],Tmp2,32*192*4);
  }
  
  myfree(SRAMIN,Tmp);
  myfree(SRAMIN,Tmp1);
  myfree(SRAMIN,Tmp2);
}

