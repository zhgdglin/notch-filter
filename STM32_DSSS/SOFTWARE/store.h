#ifndef __STORE_H
#define __STORE_H

#include "system.h"
#include "ftl.h"		//Fatfs头文件
#include "ff.h"			//Fatfs头文件
#include "exfuns.h"	//Fatfs头文件

extern void DataRecv(char *Buff,u32 Len,u8 filenum);
extern void DataSave(void);
extern void DecodeLogSave(u8 *Deode, u32 Len);
extern void DecodeLogRead(void);
extern void DecodeLogDelete(void);
extern void DataSaveDelete(void);
extern void DataSaveDecode(s32 *data,u32 len);

#endif /*__STORE_H*/







