#ifndef  __DS1302_H
#define  __DS1302_H
#include "APP_Work.h"


/* DS1302  RTC时钟芯片   */  




/* 寄存器地址 */   
//  B7   B6    B5   B4   B3   B2   B1   B0       // 位数
//  1    R/C   A4   A3   A2   A1   A0   R/W  		 
//  高位在前               // 读取地址（写入函数也可直接调用，内部已转换）
#define ADDR_1302_Second  					0x81
#define ADDR_1302_Minute  					0x83
#define ADDR_1302_Hour   						0x85
#define ADDR_1302_Date   						0x87
#define ADDR_1302_Month  						0x89
#define ADDR_1302_Day    						0x8B
#define ADDR_1302_Year   						0x8D
#define ADDR_1302_Write_Lock    		0x8F
#define ADDR_1302_TCS   						0x91

void RTC_Init(void);
void RTC_Write(void);
void RTC_Set_24Hour(void);

//void RTC_Read(void);
char* RTC_Read(void);  // RTC_Read

#endif
