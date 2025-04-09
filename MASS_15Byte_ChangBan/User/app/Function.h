/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.1
Date: 	       // 2023/02/06
Description:   // 简版通信机驱动，无任何算法应用
History:       // V.0
*******************************************************************/
#ifndef _FUNCTION_H
#define _FUNCTION_H

#include "bsp.h"

/****************************                   博哥程序遗留函数                     ******************************/
void SynPickUp(void);
void Soft_Reset(void);
void int31_to_float(int32_t *sound,uint32_t sound_len, float *data);
void MemUsed(void);

void HFMSignalGen(float T,float *SynSig);
void LFMSignalGen(float T,float *SynSig);
u8   XORCaculate(u8 *Data,u32 Len);
u32  DopplerEstimation(float *data);
u32 Resample(float *input, u32 inputlen, float *output, u32 outputlen, float frac);
void BinToChar (u8 *pSrc, u8 *pDst, u32 pSrc_len,u32 bits_len);
void FirFilter(float *pSrc,u32 SrcLen,float *Coeff,u32 CoeffLen,float *pDst,u32 blockSize);
void BinToM_Code(u8 *pSrc, u8 *pDst, u32 SrcLen,u32 M_Code);
void arm_real_mat_mult_cmplx_mat_f32(float *pSrcA,u32 RowA,u32 ColumnA,float *pSrcB,u32 RowB,u32 ColumnB,float *pDst,u32 RowC,u32 ColumnC);
void M_CodeToBin (u32 *M_Code, u8 *Bin, u32 M_CodeLen,u32 M); 

void HEXToBin(u8 *HEX,u32 HEXLEN,u8 *BIN);
void RealImagCombine(float *CplxSig,float *RealSig,float *ImagSig,u32 SigLen);
void DDCF(float *SigIn,float *SigOut,u32 SigLen);
void CplxDownSamp(float *SigIn,float *SigOut,u8 SampNum,u32 SigLen);
void LfmGen(float *LFM);
void CplxFreqSampSynGen(float *LfmIn,float *FreqLfmOut,u8 SampNum,u32 SigLen);

void LocalSynRefGen_HFM(float *SigOut);
void LocalSynRefGen_LFM(float *SigOut);
u8 SynDetect(float *SigIn,float *SynFreqConj,u8 SampNum,u32 SigLen,u32 *synpose);
void SynDetectTD(float *SigIn,float *SynFreqConj,u8 SampNum,u32 SigLen,u32 *synpose);
u32 Syn2nd(float *data,u8 cont);
void g_BackupSlideStore(void);

void ModulateMaSS(u8 *UartHex,u32 UartHexLen,float *MaSS_Sig);
void DemodulateMaSS(float *SigReceived,u32 Pos);
void FrameStructGen(u8 *UartHex,u32 UartHexLen,u32 *FrameNum,u8 *FrameBin);
void InfoGen(u8 *FrameBin,u32 FrameIndex,u8 *TotalBitMaryPerFrameDec,u8 *InfoBitMaryPerFrameDec);
void MaSSModulate(u8 *TotalBitMaryPerFrameDec,char *SpreadCode,float *Signal);
void FormSigStructureMASS(float *SigInput,u32 SigLen,float *SigOutput);
void MassDemodulate(float *Sig,u32 SigLen,u8 *Decoded);
void DeSpreadCodeMatrixGen(float *DeSpreadCode,float *DeSpreadCodeMatrix);


#endif
/***************************** 声威海洋科技 (END OF FILE) *********************************/
