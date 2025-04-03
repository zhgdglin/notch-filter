#ifndef __FUNCTION_H
#define __FUNCTION_H

#include "system.h"
void ParameterInit(void);
void ParameterFree(void);
void DownConversion_SYN(float *Sig,u32 SigLen,float *BaseSig);
void EvenOddCombine(float *pSrcA,float *pSrcB,float *pDst,u32	blockSize);
void HFMSignalGen(float T,float *SynSig);
void LFMSignalGen(float SigTime,float *SynSig);
void S32ToF32(s32 *DataIn,u32 Len,float *DataOut);
void DownSamplingCplx(float *Sig,u32 SigLen,u32 Q,float *SigDownSamp);;
void HFMCopyGen(float *HFMSig,u32 HFMSigLen,float *HFMFreqConj,float *BaseBandSig);
u8   Syn1st(float *Sig,u32 SigLen,float *RefCopy,u32 *Pose1st);
u8   XORCaculate(u8 *Data,u32 Len);
u8   CheckSum(u8 *Data,u32 Len);

void DSSSDiffCode(signed char *Code,u32 CodeLen,signed char *DiffCode);
void MCodeGen(char *InitCode,signed char *Mcode);
void DecToBin(int pSrc, char *pDst, u32	blockSize);
void DSSSKron(signed char *DataA,u32 DataALen,signed char *DataB,u32 DataBLen,signed char *DSCode);
void Rectpulse(signed char *DataA,u32 DataALen,u32 DataB,signed char *DataC);
u32  DotProduct(char *DataA,char *DataB,u32 DataSize);
void DSSSBandPassFilter(float *pSrc,u32 SrcLen,float *Coeff,u32 CoeffLen,float *pDst,u32 blockSize);
void DiffEnergyDector(float *Sig,u32 SigLen,signed char *Decode);
void EvenOddRank(float *EvenSig,float *OddSig,u32 SigLen,float *CplxSig);
void SignedBinToChar (signed char *pSrc, u8 *pDst, u32 pSrc_len,u32 bits_len);
void ScaleTo1(float *data, u32 data_len, float *data_normal);
void CharToBinStream(u8 *pSrc, u8 *pDst, u32 pSrc_len,u32 bits_len);

u32  DopplerEstimation(float *data);
void Resample(float *input, u32 inputlen, float *output, u32 outputlen, float frac);
#endif /*__FUNCTION_H*/




