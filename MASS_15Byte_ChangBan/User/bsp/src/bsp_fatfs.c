/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.2
Date: 	       // 2024/02/29
Description:   // 长板通信机驱动，MFSK
History:       // V.0、V.1
*******************************************************************/
#include "bsp.h"

#include "ff.h"				/* FatFS文件系统模块*/
#include "ff_gen_drv.h"
#include "sd_diskio_dma.h"

#define TxtName            "Test01.txt"    
#define FloatInfoNum        142803
#define ReadBuffSize_test   6144*24*4 
#define SingleWriteNum      6144*5 

u32 IdleCount=0;

RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;

FATFS fs;
FIL file;
FRESULT result1;
uint32_t bw1;
//ALIGN_32BYTES(char FsReadBuf[128]);
//ALIGN_32BYTES(char FsWriteBuf[128]) = {"FatFS Write Demo \r\n www.armfly.com \r\n"};
//ALIGN_32BYTES(uint8_t g_TestBuf[BUF_SIZE]);

DIR DirInf;
FILINFO FileInf;
char DiskPath[4]; /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */

/* FatFs API的返回值 */
static const char * FR_Table[]= 
{
	"FR_OK：成功",				                             /* (0) Succeeded */
	"FR_DISK_ERR：底层硬件错误",			                 /* (1) A hard error occurred in the low level disk I/O layer */
	"FR_INT_ERR：断言失败",				                     /* (2) Assertion failed */
	"FR_NOT_READY：物理驱动没有工作",			             /* (3) The physical drive cannot work */
	"FR_NO_FILE：文件不存在",				                 /* (4) Could not find the file */
	"FR_NO_PATH：路径不存在",				                 /* (5) Could not find the path */
	"FR_INVALID_NAME：无效文件名",		                     /* (6) The path name format is invalid */
	"FR_DENIED：由于禁止访问或者目录已满访问被拒绝",         /* (7) Access denied due to prohibited access or directory full */
	"FR_EXIST：文件已经存在",			                     /* (8) Access denied due to prohibited access */
	"FR_INVALID_OBJECT：文件或者目录对象无效",		         /* (9) The file/directory object is invalid */
	"FR_WRITE_PROTECTED：物理驱动被写保护",		             /* (10) The physical drive is write protected */
	"FR_INVALID_DRIVE：逻辑驱动号无效",		                 /* (11) The logical drive number is invalid */
	"FR_NOT_ENABLED：卷中无工作区",			                 /* (12) The volume has no work area */
	"FR_NO_FILESYSTEM：没有有效的FAT卷",		             /* (13) There is no valid FAT volume */
	"FR_MKFS_ABORTED：由于参数错误f_mkfs()被终止",	         /* (14) The f_mkfs() aborted due to any parameter error */
	"FR_TIMEOUT：在规定的时间内无法获得访问卷的许可",		 /* (15) Could not get a grant to access the volume within defined period */
	"FR_LOCKED：由于文件共享策略操作被拒绝",				 /* (16) The operation is rejected according to the file sharing policy */
	"FR_NOT_ENOUGH_CORE：无法分配长文件名工作区",		     /* (17) LFN working buffer could not be allocated */
	"FR_TOO_MANY_OPEN_FILES：当前打开的文件数大于_FS_SHARE", /* (18) Number of open files > _FS_SHARE */
	"FR_INVALID_PARAMETER：参数无效"	                     /* (19) Given parameter is invalid */
};

/*
*********************************************************************************************************
*	函 数 名: DemoFatFS
*	功能说明: FatFS文件系统演示主程序
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void FatFsInit(void)
{
  uint8_t State;
	/* 打印命令列表，用户可以通过串口操作指令 */
//	DispMenu();
	
	/* 注册SD卡驱动 */
	State = FATFS_LinkDriver(&SD_Driver, DiskPath);
	if(State == 0)
	{
//		printf("\r\n");
//		printf("SD卡挂载成功!\r\n");
	}
	else 
	{
		printf("\r\n");
		printf("SD卡挂载失败!\r\n");
	}
}

void DataSave(void)
{
	u32 i = 0;
  FRESULT result;
	uint32_t bw;
	uint32_t FileCnt = 0;
  
  char DataName[32];
  char DataName1[32];
  char FileNameIsValid = 0;
	u8  FileNameIndex[2] = {0x00,0x00};
	u16 FileNameIndexD = 0;
  
  /* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
  
  if(result == FR_OK)
  {
//    printf("挂载文件系统成功 (%s)\r\n", FR_Table[result]);
  }
  else if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}
  
  while(f_opendir(&DirInf,"0:/RECORDER"))
  {
    f_mkdir("0:/RECORDER");																//创建该目录 
		break;
  }
  
  while(FileNameIndexD<0xFFFF)
  {
    sprintf((char*)DataName1,"%s/RECORDER/%d.txt",DiskPath,FileNameIndexD);   
    result = f_open(&file,DataName1,FA_READ);	//尝试打开这个文件
    if(result == FR_NO_FILE)
    {
      FileNameIsValid = 1;
			FileNameIndex[0] = FileNameIndexD/256;
			FileNameIndex[1] = FileNameIndexD%256;
			Uart1_Puts(&FileNameIndex[0],2);
			f_close(&file);
      break;	
    }
  
    FileNameIndexD++;
		f_close(&file);
  }
  
  while(1)
  {
    if(FileNameIsValid==1)
    {	
      sprintf((char*)DataName,"%s/RECORDER/%d.txt",DiskPath,FileNameIndexD);
      result = f_open(&file, DataName, FA_CREATE_ALWAYS|FA_WRITE);
    if (result != FR_OK)
	{
		printf("文件创建失败 (%s)\r\n", FR_Table[result]);
	}
      if (result == FR_OK)
      {
//        printf("%d.txt 文件打开成功\r\n",FileNameIndexD);
        
				while(FileCnt<500)//0.064*3*100=19.2秒，每次存储19.2s数据,7200kB。
				{
					if(g_StorageDataOK==0)						
					{
						for(i=0;i<g_StoreBlockNum;i++)
						{
							result = f_write(&file,(const void*)&g_StoreDataBuf[i*g_ReceiveBlock],g_ReceiveBlock*4,&bw);
							if (result != FR_OK)
							{
								printf("数据写入失败 (%s)\r\n", FR_Table[result]);
							}
						}					
						memset(&g_StoreDataBuf[0],0,g_ReceiveBlock*g_StoreBlockNum*4);
						FileCnt++;						
				        g_StorageDataOK=1;
					}
					else
					{
						IdleCount++;
						if(IdleCount>1000000)IdleCount=0;
					}
				}
        
        FileNameIndexD++;
        FileNameIsValid = 0;      
      }
      
//      printf("存储结束\r\n");      
    }
    
    f_close(&file);
    break;
  }
  /* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
    printf("\nSD Save Data Finish");
}

void DataSaveDecode(s32 *data,u32 len)
{
  FRESULT result;
	uint32_t bw;
  
  char DataName[32];
  char DataName1[32];
  char FileNameIsValid = 0;
	u8  FileNameIndex[2] = {0x00,0x00};
	u16 FileNameIndexD = 0;
  
  /* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
  
  if(result == FR_OK)
  {
//    printf("挂载文件系统成功 (%s)\r\n", FR_Table[result]);
  }
  else if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}
  
  while(f_opendir(&DirInf,"0:/RECORDER"))
  {
    f_mkdir("0:/RECORDER");																//创建该目录 
		break;
  }
  
  while(FileNameIndexD<0xFFFF)
  {
    sprintf((char*)DataName1,"%s/RECORDER/%d.txt",DiskPath,FileNameIndexD);
    
    result = f_open(&file,DataName1,FA_READ);	//尝试打开这个文件
    if(result == FR_NO_FILE)
    {
      FileNameIsValid = 1;
			f_close(&file);
      break;	
    }
  
    FileNameIndexD++;
		f_close(&file);
  }
  
  while(1)
  {
    if(FileNameIsValid==1)
    {	
      sprintf((char*)DataName,"%s/RECORDER/%d.txt",DiskPath,FileNameIndexD);
      result = f_open(&file, DataName, FA_CREATE_ALWAYS|FA_WRITE);
      
      if (result == FR_OK)
      {
//        printf("%d.txt 文件打开成功\r\n",FileNameIndexD);      													
				result = f_write(&file,(const void*)data,len*4,&bw);																														
        FileNameIsValid = 0;    
				FileNameIndex[0] = FileNameIndexD/256;
				FileNameIndex[1] = FileNameIndexD%256;
				Uart1_Puts(&FileNameIndex[0],2);				
      }    
//      printf("存储结束\r\n");      
    }
    
    f_close(&file);
    break;
  }
  /* 卸载文件系统 */
	f_mount(NULL, DiskPath, 0);
}

//数据存储删除
void DataSaveDelete(void)
{
  FRESULT result;
  
  char DataName[32];
	u16 FileNameIndexD = 1;
	u8  FileNameIndex[2] = {0x00,0x00};	
	
  /* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
  
  if(result == FR_OK)
  {
//    printf("挂载文件系统成功 (%s)\r\n", FR_Table[result]);
  }
  else if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}
  
  while(f_opendir(&DirInf,"0:/RECORDER"))
  {
    f_mkdir("0:/RECORDER");																//创建该目录 
		break;
  }	
	
	while(1)
	{
    sprintf((char*)DataName,"%s/RECORDER/%d.txt",DiskPath,FileNameIndexD);
    result = f_open(&file,DataName,FA_READ);	//尝试打开这个文件
    if(result == FR_NO_FILE)
    {
      break;	
    }
		f_close(&file);
		f_unlink(DataName);
		FileNameIndex[0] = FileNameIndexD/256;
		FileNameIndex[1] = FileNameIndexD%256;
		Uart1_Puts(&FileNameIndex[0],2);		
//		printf("\r\n");		
		FileNameIndexD++;
		
	}
}

//解码日志存储——自动存储
void DecodeLogSave(u8 *Deode, u32 Len)
{
	u32 i;
	char DataName[32];
	u8 	State;
	u8	NameIsValid;
	
  FRESULT result;
  
  /* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
  
  if(result == FR_OK)
  {
//    printf("挂载文件系统成功 (%s)\r\n", FR_Table[result]);
  }
  else if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}	
	
	//检测文件夹，若没检测到，创建文件夹
	while(f_opendir(&DirInf,"0:/DecodeFile"))								//打开日志文件夹
	{	
		f_mkdir("0:/DecodeFile");															//创建该目录 
		break;
	}
	
	while(1)
	{
		sprintf((char*)DataName,"0:/DecodeFile/Decode.txt");
		State = f_open(&file,(const TCHAR*)DataName,FA_READ);	//尝试打开这个文件
		
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
	
	f_close(&file);
	if(NameIsValid==1)
	{
		State = f_open(&file,(const TCHAR*)DataName, FA_WRITE);
		
		if(State==FR_OK)
		{		
			//先获取RTC时间，进行日志时间记录
			HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);		
			HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
      
      f_lseek(&file, f_size(&file));
      
			f_printf(&file,"20%02d-%02d-%02d-%02d.%02d.%02d: ",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date,RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
			
			//再存储日志信息
			for(i=0;i<Len;i++)		
			{
				f_printf(&file,"%02X ",Deode[i]);
			}
			f_printf(&file,"\r\n");
		}
	}
	else if(NameIsValid==2)
	{
		State = f_open(&file,(const TCHAR*)DataName, FA_CREATE_ALWAYS | FA_WRITE);
		
		if(State==FR_OK)
		{		
			//先获取RTC时间，进行日志时间记录
			HAL_RTC_GetTime(&RTC_Handler,&RTC_TimeStruct,RTC_FORMAT_BIN);		
			HAL_RTC_GetDate(&RTC_Handler,&RTC_DateStruct,RTC_FORMAT_BIN);
      
      f_lseek(&file, f_size(&file));
      
			f_printf(&file,"20%02d-%02d-%02d-%02d.%02d.%02d: ",RTC_DateStruct.Year,RTC_DateStruct.Month,RTC_DateStruct.Date,RTC_TimeStruct.Hours,RTC_TimeStruct.Minutes,RTC_TimeStruct.Seconds);
			
			//再存储日志信息
			for(i=0;i<Len;i++)		
			{
				f_printf(&file,"%02X ",Deode[i]);
			}
			f_printf(&file,"\r\n");
		}
	}

	f_close(&file);
}

//解码日志读取
void DecodeLogRead(void)
{
	u32 bw;
	char DataName[32];
	
	u8 	State;
  u32 DataSize = 0;
	
  FRESULT result;
  
  /* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
  
  if(result == FR_OK)
  {
//    printf("挂载文件系统成功 (%s)\r\n", FR_Table[result]);
  }
  else if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}		
	//检测文件夹，若没检测到，创建文件夹
	while(f_opendir(&DirInf,"0:/DecodeFile"))								//打开日志文件夹
	{	
		f_mkdir("0:/DecodeFile");															//创建该目录 
		break;
	}
	
	sprintf((char*)DataName,"0:/DecodeFile/Decode.txt");
	State = f_open(&file,(const TCHAR*)DataName,FA_READ);	//尝试打开这个文件 
	if(State == FR_NO_FILE)
	{
		u8 NoTxtInfo[10] = {0x4e,0x6f,0x20,0x54,0x78,0x74,0x20,0x21,0x21,0x21};   //No Txt !!!
		for(u8 i=0;i<10;i++)    
    {
      printf("%c",NoTxtInfo[i]);
    }
	}
	else if(State == FR_OK)
	{
		f_lseek(&file,f_size(&file));
		DataSize = f_tell(&file);	
		f_close(&file);
  
		char *Buff;
		Buff = mymalloc(SRAMEX,DataSize);
		memset(Buff,0,DataSize);
		State = f_open(&file,(const TCHAR*)DataName,FA_READ);	//尝试打开这个文件   
    State = f_read(&file,Buff,DataSize,&bw);  
    
    for(u32 j=0;j<DataSize;j++)    
    {
      printf("%c",Buff[j]);
			IWDG_Feed();//喂狗
    }
		myfree(SRAMEX,Buff);
	}
	else{}
  
  f_close(&file);
}

//解码日志删除
void DecodeLogDelete(void)
{
    FRESULT result;
	char DataName[32];
  
  /* 挂载文件系统 */
	result = f_mount(&fs, DiskPath, 0);			/* Mount a logical drive */
  
  if(result == FR_OK)
  {
//    printf("挂载文件系统成功 (%s)\r\n", FR_Table[result]);
  }
  else if (result != FR_OK)
	{
		printf("挂载文件系统失败 (%s)\r\n", FR_Table[result]);
	}		
	
	sprintf((char*)DataName,"0:/DecodeFile/Decode.txt");
	f_unlink((const TCHAR*)DataName);

}


/***************************** 会上树的猪(END OF FILE) *********************************/
