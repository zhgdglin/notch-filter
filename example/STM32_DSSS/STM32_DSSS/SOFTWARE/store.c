#include "system.h"

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
	
void DataRecv(char *Buff,u32 Len,u8 filenum)
{
	FIL *SigRecord = 0;
	DIR ReadDir;
	u8  *FileName = 0;
	
	u8 	State;
	//变量名内存申请
	FileName = mymalloc(SRAMIN,30);
	SigRecord = (FIL *)mymalloc(SRAMIN,sizeof(FIL));						//开辟FIL字节的内存区域
	
	while(f_opendir(&ReadDir,"0:/RECORDER"))										//打开文件夹
	{	
		break;
	}
	
	sprintf((char*)FileName,"0:RECORDER/data%d.txt",filenum);
	State = f_open(SigRecord,(const TCHAR*)FileName, FA_OPEN_EXISTING | FA_READ);
	
	if(State != FR_OK)
	{
		printf("没有此文件！\r\n");
	}
	else
	if(State == FR_OK)
	{													
		State = f_read(SigRecord,Buff,Len,&bw);
		if(bw > 0)
		{
			Buff[bw] = 0;
		}
		
		f_close(SigRecord);
		myfree(SRAMIN,FileName);
		myfree(SRAMIN,SigRecord);
	}
}


void DataSave(void)
{
  FIL *pRecv=0;
	u8  *DataName = 0;
	DIR RecDir;

	u8 	State;
	u8	Name_valid_flag;
	u32 FileCnt = 0;
	u8  FileNameIndex[2] = {0x00,0x00};
	u16 FileNameIndexD = 0;
	
	//变量名内存申请
	DataName = mymalloc(SRAMIN,30);
	pRecv = (FIL *)mymalloc(SRAMIN,sizeof(FIL));						//开辟FIL字节的内存区域
	
	while(f_opendir(&RecDir,"0:/RECORDER"))									//打开录音文件夹
	{	
		f_mkdir("0:/RECORDER");																//创建该目录 
		break;
	}
	
	while(1)
	{
		while(FileNameIndexD<0xFFFE)
		{	
			sprintf((char*)DataName,"0:RECORDER/%d.txt",FileNameIndexD);
			State = f_open(ftemp,(const TCHAR*)DataName,FA_READ);	//尝试打开这个文件
			if(State == FR_NO_FILE)
			{
				Name_valid_flag = 1;
				FileNameIndex[0] = FileNameIndexD/256;
				FileNameIndex[1] = FileNameIndexD%256;
				Uart1_Puts(&FileNameIndex[0],2);
				break;	
			}
			FileNameIndexD++;
		}
		
		while(1)
		{
			if(Name_valid_flag)
			{
				State = f_open(pRecv,(const TCHAR*)DataName, FA_CREATE_ALWAYS | FA_WRITE);
				
				if(State==0)
				{
					while(FileCnt<31)//12888*15/96000=2秒，每次存储60s数据。
					{
						while(g_StorageDataOK==0);						
						g_StorageDataOK = 0;
						FileCnt++;														
						State = f_write(pRecv,(const void*)g_StoreDataBuf,g_ReceiveBlock*15*4,&bw);																														
					}
					
					f_close(pRecv);
          
					Name_valid_flag = 0;
					FileCnt = 0;
          
					myfree(SRAMIN,DataName);
					myfree(SRAMIN,pRecv);
				}
			}
			break;
		}		
		break;
	}
}

void DataSaveDecode(s32 *data,u32 len)
{
  FIL *pRecv=0;
	u8  *DataName = 0;
	DIR RecDir;

	u8 	State;
	u8	Name_valid_flag;
	u8  FileNameIndex[2] = {0x00,0x00};
	u16 FileNameIndexD = 0;
	
	//变量名内存申请
	DataName = mymalloc(SRAMIN,30);
	pRecv = (FIL *)mymalloc(SRAMIN,sizeof(FIL));						//开辟FIL字节的内存区域
	
	while(f_opendir(&RecDir,"0:/RECORDER"))									//打开录音文件夹
	{	
		f_mkdir("0:/RECORDER");																//创建该目录 
		break;
	}
	
	while(1)
	{
		while(FileNameIndexD<0xFFFE)
		{	
			sprintf((char*)DataName,"0:RECORDER/%d.txt",FileNameIndexD);
			State = f_open(ftemp,(const TCHAR*)DataName,FA_READ);	//尝试打开这个文件
			if(State == FR_NO_FILE)
			{
				Name_valid_flag = 1;
				break;	
			}
			FileNameIndexD++;
		}
		
		while(1)
		{
			if(Name_valid_flag)
			{
				State = f_open(pRecv,(const TCHAR*)DataName, FA_CREATE_ALWAYS | FA_WRITE);
				
				if(State==0)
				{												
					State = f_write(pRecv,(const void*)data,len*4,&bw);																														

					f_close(pRecv);
          
					Name_valid_flag = 0;
					
					FileNameIndex[0] = FileNameIndexD/256;
					FileNameIndex[1] = FileNameIndexD%256;
	      	Uart1_Puts(&FileNameIndex[0],2);
					myfree(SRAMIN,DataName);
					myfree(SRAMIN,pRecv);
				}
			}
			break;
		}		
		break;
	}
}


//数据存储删除
void DataSaveDelete(void)
{
	u8  *FileName = 0;	
	FileName = mymalloc(SRAMIN,30);
	u8  FileNameIndex = 0;
	u8 	State;
	
	while(1)
	{
		sprintf((char*)FileName,"0:RECORDER/%d.txt",FileNameIndex);
		State = f_open(ftemp,(const TCHAR*)FileName,FA_READ);	//尝试打开这个文件
		if(State == FR_NO_FILE)
		{
			break;	
		}
		f_unlink((const TCHAR*)FileName);
		FileNameIndex++;
	}
	
	myfree(SRAMIN,FileName);
}

//解码日志存储——自动存储
void DecodeLogSave(u8 *Deode, u32 Len)
{
	u32 i;
	DIR RecDir;
	FIL *pRecv=0;
	u8  *FileName = 0;
	u8 	State;
	u8	NameIsValid;
  
	//变量名内存申请
	FileName = mymalloc(SRAMIN,30);
	pRecv = (FIL *)mymalloc(SRAMIN,sizeof(FIL));						//开辟FIL字节的内存区域
	
	//检测文件夹，若没检测到，创建文件夹
	while(f_opendir(&RecDir,"0:/DecodeFile"))								//打开日志文件夹
	{	
		f_mkdir("0:/DecodeFile");															//创建该目录 
		break;
	}
	
	while(1)
	{
		sprintf((char*)FileName,"0:DecodeFile/Decode.txt");
		State = f_open(ftemp,(const TCHAR*)FileName,FA_READ);	//尝试打开这个文件
		
		if(State ==FR_OK)
		{
			NameIsValid = 1;
			break;
		}
		else if(State == FR_NO_FILE)
		{
			NameIsValid = 2;
			break;	
		}
	}
	
	if(NameIsValid==1)
	{
		State = f_open(pRecv,(const TCHAR*)FileName, FA_WRITE);
		
		if(State==FR_OK)
		{		
			//先获取RTC时间，进行日志时间记录
			HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);		
			HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
      
      f_lseek(pRecv, f_size(pRecv));
      
			f_printf(pRecv,"20%02d-%02d-%02d-%02d.%02d.%02d: ",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date,RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
			
			//再存储日志信息
			for(i=0;i<Len;i++)		
			{
				f_printf(pRecv,"%02X ",Deode[i]);
			}
			f_printf(pRecv,"\r\n");
		}
	}
	else if(NameIsValid==2)
	{
		State = f_open(pRecv,(const TCHAR*)FileName, FA_CREATE_ALWAYS | FA_WRITE);
		
		if(State==FR_OK)
		{		
			//先获取RTC时间，进行日志时间记录
			HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);		
			HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
      
      f_lseek(pRecv, f_size(pRecv));
      
			f_printf(pRecv,"20%02d-%02d-%02d-%02d.%02d.%02d: ",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date,RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
			
			//再存储日志信息
			for(i=0;i<Len;i++)		
			{
				f_printf(pRecv,"%02X ",Deode[i]);
			}
			f_printf(pRecv,"\r\n");
		}
	}

	f_close(pRecv);
	myfree(SRAMIN,FileName);
	myfree(SRAMIN,pRecv);
}

//解码日志读取
void DecodeLogRead(void)
{
	FIL *pRecv=0;
	u8  *FileName = 0; 
  char *Buff;
	
	u8 	State;
  u32 DataSize = 0;
  //变量名内存申请
	FileName = mymalloc(SRAMIN,30);
	pRecv = (FIL *)mymalloc(SRAMIN,sizeof(FIL));
  
  sprintf((char*)FileName,"0:DecodeFile/Decode.txt");
	State = f_open(pRecv,(const TCHAR*)FileName,FA_READ);
	if(State == FR_NO_FILE)
	{
		u8 NoTxtInfo[10] = {0x4e,0x6f,0x20,0x54,0x78,0x74,0x20,0x21,0x21,0x21};   //No Txt !!!
		for(u8 i=0;i<10;i++)    
    {
      printf("%c",NoTxtInfo[i]);
    }
	}

  f_lseek(pRecv,f_size(pRecv));
  DataSize = f_tell(pRecv);	
  
  f_close(pRecv);
  
  sprintf((char*)FileName,"0:DecodeFile/Decode.txt");
	State = f_open(pRecv,(const TCHAR*)FileName,FA_READ);
  
  Buff = mymalloc(SRAMIN,DataSize);
  
	if(State == FR_OK)
	{
    State = f_read(pRecv,Buff,DataSize,&bw);  
    
    for(u32 j=0;j<DataSize;j++)    
    {
      printf("%c",Buff[j]);
    }
	}
  
  f_close(pRecv);
  myfree(SRAMIN,pRecv);
  myfree(SRAMIN,FileName);
  myfree(SRAMIN,Buff);
}

//解码日志删除
void DecodeLogDelete(void)
{
	u8  *FileName = 0;	
	FileName = mymalloc(SRAMIN,30);
	
	sprintf((char*)FileName,"0:DecodeFile/Decode.txt");
	f_unlink((const TCHAR*)FileName);
	
	myfree(SRAMIN,FileName);
}
