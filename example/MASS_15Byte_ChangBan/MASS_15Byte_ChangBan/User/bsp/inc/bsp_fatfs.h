/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.2
Date: 	       // 2024/02/29
Description:   // 长板通信机驱动，MFSK
History:       // V.0、V.1
*******************************************************************/
#ifndef _BSP_FATFS_H
#define _BSP_FATFS_H

#include "bsp.h"
#include "ff.h"				/* FatFS文件系统模块*/

extern FATFS fs;
extern FIL file;
//extern char FsReadBuf[128];
//extern char FsWriteBuf[128];
//extern uint8_t g_TestBuf[BUF_SIZE];
extern const char * FR_Table[];
extern DIR DirInf;
extern FILINFO FileInf;
extern char DiskPath[4]; /* SD卡逻辑驱动路径，比盘符0，就是"0:/" */


void FatFsInit(void);
void DataSave(void);
void DataSaveDecode(s32 *data,u32 len);
void DataSaveDelete(void);
void DecodeLogSave(u8 *Deode, u32 Len);
void DecodeLogRead(void);
void DecodeLogDelete(void);

#endif
/***************************** 会上树的猪(END OF FILE) *********************************/
