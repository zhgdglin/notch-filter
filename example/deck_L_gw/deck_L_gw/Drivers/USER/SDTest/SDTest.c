#include "SDTest.h"
#include "sdmmc.h"
/**
  ******************************************************************************
  *                              定义变量
  ******************************************************************************
  */
//char SDPath[4]; /* SD逻辑驱动器路径 */
static FATFS fs;				/* FatFs文件系统对象 */
static FIL fnew;				/* 文件对象 */
static FRESULT res_sd;           /* 文件操作结果 */
UINT fnum;            			  /* 文件成功读写数量 */
BYTE ReadBuffer[1024]={0};        /* 读缓冲区 */
BYTE WriteBuffer[] =              /* 写缓冲区*/
"This is new have SD + fatfs.\r\n";  
BYTE word[_MAX_SS]={0}; 

char file_name[6]; 

//extern char * Tx_buf_begin ;
///********外部标志位*********/
//extern volatile bool Toggle_Pi_Po_Flag;
//extern volatile bool Pi_Buf_Full_Flag;
//extern volatile bool Po_Buf_Full_Flag;
//extern volatile uint16_t Package_Total_Num;
//extern volatile char *g_writeAdr;
//extern volatile bool Pi_Buf_Send_Package_Finish_Flag;
//extern volatile bool Po_Buf_Send_Package_Finish_Flag;
/***************************/
//extern FATFS flash_fs;
//extern Diskio_drvTypeDef  SD_Driver;
extern SD_HandleTypeDef hsd1;

void Fatfs_SD_Init (void)
{ 
	res_sd = f_mount(&fs,  (TCHAR const*) SDPath, 1);
	if(res_sd != FR_OK){
		printf("SD mount error \r\n");
		while(1);
	}
	res_sd = f_open( &fnew, "0:DATA_14_07.dat", FA_CREATE_ALWAYS | FA_WRITE ); //新建文件
	if(res_sd != FR_OK){
		printf(".dat file create or open error \r\n");
		while(1);
	}
}
/*
  * @brief SD_Card进程函数
  * @param None
  * @retval None
*/
void SD_Write_Process (void)
{
//	if((Pi_Buf_Full_Flag == true)||(Po_Buf_Full_Flag == true)){
//		/*判断是否SD卡传输成功*/
//		if(SD_GetCardState()==SD_TRANSFER_OK)
//		{
//			LED_G_ON();
//			SD_Write_Dat();
//			LED_G_OFF();	
//		}
//	}
}

void SD_Write_Dat(void)
{
//	/*Pi_BUF 满,且以太网发送完成*/
//	if((Pi_Buf_Full_Flag == true)&&(g_writeAdr!=NULL))//&&(Pi_Buf_Send_Package_Finish_Flag == true)
//	{
//		res_sd = f_write(&fnew,g_writeAdr, Buff_size, &fnum);
//		if(res_sd != FR_OK){
//			debug("Pi_Buf write error \r\n");
//			Error_Handler();
//		}
//		/*通过总包数判断是否为第一次写入*/
//		if(Package_Total_Num>=2*Buff_Package_Num){
//			/*不是第一次写入*/
//		}else{
//			/*第一次写入*/
//			res_sd = f_lseek(&fnew,f_size(&fnew));
//			if(res_sd != FR_OK){
//			debug("Pi_Buf first write f_lseek error \r\n");
//			Error_Handler();
//			}
//		}
////		memset(g_writeAdr,0,Buff_Use_Size); // 清空缓存区
//		f_sync(&fnew);
//		g_writeAdr = NULL;
//		Pi_Buf_Full_Flag = false;
//	}
//	/*Po_BUF 满,且以太网发送完成*/
//	else if((Po_Buf_Full_Flag == true)&&(g_writeAdr!=NULL))//&&(Po_Buf_Send_Package_Finish_Flag == true)
//	{
//		res_sd = f_write(&fnew,g_writeAdr, Buff_size, &fnum);
//		if(res_sd != FR_OK){
//			debug("Po_Buf write error \r\n");
//			Error_Handler();
//		}
////		memset(g_writeAdr,0,Buff_Use_Size); // 清空缓存区
//		f_sync(&fnew);
//		g_writeAdr = NULL;	
//		Po_Buf_Full_Flag = false;
//	}
}

void Close_Fat_Unmount(void)
{
	f_close(&fnew);	
	f_mount(NULL,"0:",1);
}

void Check_SD_capacity(void)  //用来测试SD卡无文件系统
{
/*读取SD卡信息*/	
  if(HAL_SD_Init (&hsd1)!=HAL_OK)
	  Error_Handler();
  printf ("/**get SDcard capacity**/\r\n");
  HAL_SD_CardInfoTypeDef  SDCardInfo;         //SD卡信息结构体
    //得到卡的信息并输出到串口1
  if(HAL_SD_GetCardInfo(&hsd1, &SDCardInfo)!=HAL_OK)
	  Error_Handler();
  //HAL_SD_CardInfoTypeDef *pCardInfo);//获取SD卡信息
  printf("Card Type:%d\r\n",SDCardInfo.CardType);   //卡类型
  printf("Card CardVersion:%d\r\n",SDCardInfo.CardVersion);	     //CardVersion
  printf("Card RCA:%d\r\n",SDCardInfo.RelCardAdd);	//卡相对地址
  printf("Card Capacity:%d MB\r\n",(uint32_t)(SDCardInfo.LogBlockNbr)>>11);	//显示容量
  printf("Card BlockSize:%d\r\n\r\n",SDCardInfo.LogBlockSize);//显示块大小 
}

void Check_SD_Fatfs (void)
{
   res_sd = f_mount(&fs,  (TCHAR const*) SDPath, 1); 
    /*----------------------- 格式化测试 ---------------------------*/
    /* 如果没有文件系统就格式化创建创建文件系统 */
    if(res_sd == FR_NO_FILESYSTEM)
    {
      printf("》SD卡还没有文件系统，即将进行格式化...\r\n");
      /* 格式化 */
//      res_sd=f_mkfs((TCHAR const*) SDPath,FM_FAT32, 0, word,sizeof (word));			
    res_sd = f_mkfs((TCHAR const*)SDPath, FM_FAT32, 0, word, _MAX_SS);
      if(res_sd == FR_OK)
      {
        printf("》SD卡已成功格式化文件系统。\r\n");
        /* 格式化后，先取消挂载 */
        res_sd = f_mount(NULL,"0:",1);
        /* 重新挂载	*/			
        res_sd = f_mount(&fs,"0:",1);
      }
      else
      {
        printf("《《格式化失败。》》\r\n");
        while(1);
      }
    }
    else if(res_sd!=FR_OK)
    {
      printf("！！SD卡挂载文件系统失败。(%d)\r\n",res_sd);
      printf("！！可能原因：SD卡初始化不成功。\r\n");
      while(1);
    }
    else
    {
      printf("Fatfs mount successed\r\n");
    }
}
void SD_Test(void)
{

	
		Check_SD_Fatfs();


    /*----------------------- 文件系统测试：写测试 -----------------------------*/
    /* 打开文件，如果文件不存在则创建它 */
    printf("\r\n****** 即将进行文件写入测试... ******\r\n");	
    res_sd = f_open(&fnew, "0:date.txt",FA_CREATE_ALWAYS | FA_WRITE );
    if ( res_sd == FR_OK )
    {
      printf("》打开/创建文件成功，向文件写入数据。\r\n");
      /* 将指定存储区内容写入到文件内 */
      res_sd=f_write(&fnew,WriteBuffer,sizeof(WriteBuffer),&fnum);
      if(res_sd==FR_OK)
      {
        printf("》文件写入成功，写入字节数据：%d\n",fnum);
        printf("》向文件写入的数据为：\r\n%s\r\n",WriteBuffer);
      }
      else
      {
        printf("！！文件写入失败：(%d)\n",res_sd);
      }    
      /* 不再读写，关闭文件 */
      f_close(&fnew);
    }
    else
    {	
      printf("！！打开/创建文件失败。\r\n");
    }
    
  /*------------------- 文件系统测试：读测试 ------------------------------------*/
    printf("****** 即将进行文件读取测试... ******\r\n");
    res_sd = f_open(&fnew, "0:date.txt", FA_OPEN_EXISTING | FA_READ); 	 
    if(res_sd == FR_OK)
    {
      printf("》打开文件成功。\r\n");
      res_sd = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
      if(res_sd==FR_OK)
      {
        printf("》文件读取成功,读到字节数据：%d\r\n",fnum);
        printf("》读取得的文件数据为：\r\n%s \r\n", ReadBuffer);	
      }
      else
      {
        printf("！！文件读取失败：(%d)\n",res_sd);
      }		
    } 
    else
    {
      printf("！！打开文件失败。\r\n");
    }
    /* 不再读写，关闭文件 */
    f_close(&fnew);	
    
    /* 不再使用文件系统，取消挂载文件系统 */
    f_mount(NULL,"0:",1);
}


