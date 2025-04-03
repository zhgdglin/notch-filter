#include "SDTest.h"
#include <string.h>  

 
extern SD_HandleTypeDef hsd1;


static FATFS fs;				// FatFs文件系统对象 
static FIL fnew;				// 文件对象 
static FRESULT res_sd;  // 文件操作结果 
static FILINFO fno;     // 文件目录对象
UINT fnum;              // 文件成功读写数量 



uint32_t SD_cnt = 0;  // SD卡写入次数计数



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




/* 读取SD卡信息 */	
void Get_SD_information(void)  
{
  HAL_SD_CardInfoTypeDef  SDCardInfo = {0};         // SD卡信息结构体

	uint32_t SD_Blocksize   = 0;
	uint32_t SD_Capacity    = 0;
	uint32_t SD_Blocknumber = 0;
	
	HAL_SD_DeInit(&hsd1);  // 为稳定先复位
	
  if(HAL_SD_Init (&hsd1) != HAL_OK)    
	{	
		printf(" SD卡初始化失败\r\n");
		Error_Handler();
	}
	printf("\r\nSD卡初始化成功\r\n");
  printf ("信息打印如下：\r\n");
	
  if(HAL_SD_GetCardInfo(&hsd1, &SDCardInfo)!=HAL_OK)
	  Error_Handler();
	
	SD_Blocknumber = SDCardInfo.LogBlockNbr;
	SD_Blocksize   = SDCardInfo.LogBlockSize;
	SD_Capacity =  (SD_Blocknumber >> 10) * 512;
	
  printf(" Card Type:%d\r\n",SDCardInfo.CardType);   												// 卡类型
  printf(" Card CardVersion:%d\r\n",SDCardInfo.CardVersion);	     						// 卡版本
	printf(" Class :%d\r\n",SDCardInfo.Class);   															// 最低写入速度
  printf(" RelCardAdd :%d\r\n",SDCardInfo.RelCardAdd);												// 卡相对地址
  printf(" LogBlockNumber :%d\r\n",SD_Blocknumber);													// 逻辑块数量
  printf(" LogBlockSize :%dByte\r\n",SD_Blocksize);							 						// 逻辑块大小 
	printf(" Capacity :%dGB\r\n",SD_Capacity>>20);															// SD卡容量    由于计算机以1024计数，故实际大小比标称值小
//	printf(" Speed:%d\r\n\r\n",SDCardInfo.CardSpeed);        									// 速度
}  




void Fatfs_SD_Init (void)
{ 
	
		printf("\r\n文件系统初始化\r\n");
	
		res_sd = f_mount(&fs,  (TCHAR const*) SDPath, 1);
		printf("挂载系统 %s \r\n", FR_Table[res_sd]);
		if(res_sd != FR_OK)
			{
				printf("f_mount error \r\n");
				Error_Handler();
			}
	
		//  FA_OPEN_ALWAYS   不会覆盖写入，需要每次都格式化
		//  FA_CREATE_ALWAYS 不需要每次都格式化
//		res_sd = f_open( &fnew, "data.dat", FA_CREATE_ALWAYS | FA_WRITE ); //新建文件
//		printf("新建dat文件 %s \r\n", FR_Table[res_sd]);
//		if(res_sd != FR_OK)
//			{
//				printf("f_open error \r\n");
//				Error_Handler();
//			}
}



/*
  * @brief SD_Card进程函数
  * @param None
  * @retval None
*/
void SD_Write_Process (void)
{
	if((Ping_full_flag == 1)||(Pang_full_flag == 1)){
		/*判断是否SD卡传输成功*/
		if(SD_GetCardState() == SD_TRANSFER_OK)
		{
			Set_Pin(LED1);
			SD_Write_Dat();
			Reset_Pin(LED1);
		}
	}
}


	/* 以RTC时间命名dat文件 重名自动后面补数字 */
void SD_ctreate_new_file(void)
{

	
	static char FILE_FORMAT[6] = ".dat";
	static uint8_t same_name_cnt;
	char FILE_NAME[50] = "";
	char *p_file_name = NULL;
	char *p_file_add_name = NULL;

	/* RTC 读取*/
	p_file_name = RTC_Read();

	strcat(FILE_NAME, p_file_name);
	strcat(FILE_NAME, FILE_FORMAT);
	printf("\r\n文件名 = %s\r\n", FILE_NAME);
	
	
	/* 查找是否有相同名称的文件 */
	if( f_stat(FILE_NAME,&fno) == FR_OK)
	{
		same_name_cnt++;                           // 文件重名次数
		memset(FILE_NAME, 0 , sizeof(FILE_NAME));  // 清空原数组
		strcat(FILE_NAME, p_file_name);
		strcat(FILE_NAME, "_");
		itoa(same_name_cnt, p_file_add_name, 10);
		strcat(FILE_NAME, p_file_add_name);
		strcat(FILE_NAME, FILE_FORMAT);
		printf("重名文件 = %s\r\n", FILE_NAME);
	}
	else 
			same_name_cnt = 0;
	
	/*新建文件*/
		res_sd = f_open( &fnew, FILE_NAME , FA_CREATE_ALWAYS | FA_WRITE ); //新建文件
		printf("新建dat文件 %s \r\n", FR_Table[res_sd]);
		if(res_sd != FR_OK)
			{
				printf("f_open error \r\n");
				Error_Handler();
			}
}

void SD_Write_Dat(void)
{

	/* 乒乓写入数据 */		
	if(Ping_full_flag == 1 && p_sd!=NULL)
	{

		SD_cnt++;
		if(SD_cnt >= 10 + 1) // 测试1000个
		{
			return;
		}
		else if(SD_cnt == 1)
		{
			SD_ctreate_new_file();
		}
//				Enter_Standby_Mode();
		printf("\r\nPing  %d\r\n",SD_cnt);    // 控制写入量 方便测试
		
		//SD_ctreate_new_file();
		
		//f_lseek(&fnew,f_size(&fnew));  // 调整写入指针位置
		//res_sd = f_write(&fnew, AD7767_Ping , BUFF_SIZE*4, &fnum);   // 第三个变量是写入的字节数  4096 x 4   修改 BUFF_SIZE
		//res_sd = f_write(&fnew, AD7767_Ping, BUFF_SIZE*4, &fnum);	
		printf("f_write  Ping %s \r\n", FR_Table[res_sd]);
		if(res_sd != FR_OK)
			{
			 printf("SD write Ping error \r\n");
			 Error_Handler();
			}
			
		res_sd = f_lseek(&fnew,f_size(&fnew));  // 调整写入指针位置
		printf("f_lseek %s \r\n", FR_Table[res_sd]);
		if(res_sd != FR_OK)
			{
				printf("f_lseek Ping  error \r\n");
				Error_Handler();
			}
				
		f_sync(&fnew);  // 关闭文件，保持写入状态
		//f_close(&fnew);
		Ping_full_flag = 0;
 }	

	else if((Pang_full_flag == 1) && (p_sd!=NULL))//&&(Po_Buf_Send_Package_Finish_Flag == true)
	{
		SD_cnt++;
		if(SD_cnt >= 10 + 1) // 测试1000个
		{
			return;
		}
		else if(SD_cnt == 1)
		{
			SD_ctreate_new_file();
		}
		printf("Pang  %d\r\n",SD_cnt);   
//	  SD_ctreate_new_file();
//		RTC_Read();
//			res_sd = f_write(&fnew, RTC_Time , 6*4, &fnum);   
//			printf("f_write  RTC %s \r\n", FR_Table[res_sd]);
//			if(res_sd != FR_OK)
//			{
//			 printf("SD RTC write  error \r\n");
//			 Error_Handler();
//			}
		//	f_lseek(&fnew,f_size(&fnew));  // 调整写入指针位置
		res_sd = f_write(&fnew, AD7767_Pang, BUFF_SIZE*4, &fnum);
		//res_sd = f_write(&fnew, output_Pang, BUFF_SIZE*4, &fnum);		
		printf("f_write  Pang %s \r\n", FR_Table[res_sd]);
		if(res_sd != FR_OK)
			{
			 printf("SD write Pang error \r\n");
			 Error_Handler();
			}

		f_sync(&fnew);
//	f_close(&fnew);
		Pang_full_flag = 0;
	}
	
}



void Close_Fat_Unmount(void)
{
	f_close(&fnew);	
	f_mount(NULL,"0:",1);
}




BYTE ReadBuffer[1024] = {0}; 
BYTE WriteBuffer[] =              // 写缓冲区
"This is new have SD + fatfs.\r\n";  
BYTE word[_MAX_SS]={0}; 




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


