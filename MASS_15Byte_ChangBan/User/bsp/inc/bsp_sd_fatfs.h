#ifndef _BSP_SD_FATFS_H
#define _BSP_SD_FATFS_H
#include "bsp.h"

/* 供外部调用的函数声明 */
void bsp_InitFatfs(void);
void DispMenu(void);
void ViewRootDir(void);
void CreateNewFile(void);
void ReadFileData(void);
void CreateDir(void);
void DeleteDirFile(void);
void WriteFileTest(void);
void FliesLoad(void);

#endif

/***************************** 会上树的猪 (END OF FILE) *********************************/
