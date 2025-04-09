#include "bsp.h"

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
