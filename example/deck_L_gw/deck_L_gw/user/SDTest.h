#ifndef _SDTest_H_
#define _SDTest_H_

#include "APP_Work.h"




void Get_SD_information(void);															
void SD_Test(void);
void Check_SD_Fatfs (void);
void SD_Write_Dat(void);
void Fatfs_SD_Init (void);
void Close_Fat_Unmount (void );
void SD_Write_Process (void);			


#endif 
