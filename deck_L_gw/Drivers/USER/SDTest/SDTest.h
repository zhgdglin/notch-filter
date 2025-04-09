#ifndef _SDTest_H_
#define _SDTest_H_

#include "gpio.h"
#include "fatfs.h"
#include <stdio.h>


void Check_SD_capacity(void);
void SD_Test(void);
void Check_SD_Fatfs (void);
void SD_Write_Dat(void);
void Fatfs_SD_Init (void);
void Close_Fat_Unmount (void );
void SD_Write_Process (void);
#endif /*_SDTest_H_*/
