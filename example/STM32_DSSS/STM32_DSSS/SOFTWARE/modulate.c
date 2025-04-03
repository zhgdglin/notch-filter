#include "system.h"

void DSSSModulate(u8 *data,u32 data_len,float *DsssSig)
{
	u32 BinLen;
  
	char *MCodeInital;
  MCodeInital = mymalloc(SRAMEX,g_MOrder);
	memset(&MCodeInital[0],0,g_MOrder);
  
	BinLen = data_len*8;
	
	u8 *UartBin;
	UartBin = mymalloc(SRAMEX,data_len*8);
	memset(&UartBin[0],0,data_len*8);
  
	signed char *DSSSBinData;
  DSSSBinData = mymalloc(SRAMEX,data_len*8);
	memset(&DSSSBinData[0],0,data_len*8);
  
	signed char *DiffCode;
	DiffCode = mymalloc(SRAMEX,BinLen+1);
	memset(&DiffCode[0],0,BinLen+1);
	
	signed char *MCode;
	MCode = mymalloc(SRAMEX,(1<<g_MOrder));
	memset(&MCode[0],0,(1<<g_MOrder));
	
	signed char *MapSig;
	MapSig = mymalloc(SRAMEX,(BinLen+1)*(1<<g_MOrder));
	memset(&MapSig[0],0,(BinLen+1)*(1<<g_MOrder));
	
	signed char *ReModulateSig;
	ReModulateSig = mymalloc(SRAMEX,(BinLen+1)*(1<<g_MOrder)*g_Nsap);
	memset(&ReModulateSig[0],0,(BinLen+1)*(1<<g_MOrder)*g_Nsap);
	
	CharToBinStream(&data[0],&UartBin[0],data_len,8);
	
	for(u32 i=0;i<BinLen;i++)
	{
		DSSSBinData[i] = 2*UartBin[i] - 1;
	}
  
	DiffCode[0] = 1;
	DSSSDiffCode(DSSSBinData,BinLen,DiffCode);

	memcpy(&MCodeInital[0],&g_MCodeInital[0],g_MOrder);
	MCodeGen(&MCodeInital[0],MCode);
	
	DSSSKron(&DiffCode[0],BinLen+1,&MCode[0],(1<<g_MOrder),&MapSig[0]);

	Rectpulse(&MapSig[0],(BinLen+1)*(1<<g_MOrder),g_Nsap,&ReModulateSig[0]);
	
	for(u32 i=0;i<(BinLen+1)*(1<<g_MOrder)*g_Nsap;i++)
	{
		double x;
		x = 2*PI*i*g_f0/g_fs;
		DsssSig[g_ReceiveBlock*4+i] = ReModulateSig[i]*cos(x);
	}
	
	memcpy(&DsssSig[0],&g_SynHFM[0],g_ReceiveBlock*4);
	memcpy(&DsssSig[g_ReceiveBlock*2],&g_SynHFM[0],g_ReceiveBlock*4);
  
  myfree(SRAMEX,MCodeInital);
  myfree(SRAMEX,DSSSBinData);
	myfree(SRAMEX,UartBin);
	myfree(SRAMEX,DiffCode);
	myfree(SRAMEX,MCode);
	myfree(SRAMEX,MapSig);
	myfree(SRAMEX,ReModulateSig);
}
