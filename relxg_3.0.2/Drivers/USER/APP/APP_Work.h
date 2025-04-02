#ifndef _APP_Work_H_
#define _APP_Work_H_


#include <stdbool.h>  //bool type
#include <stdio.h>
#include <string.h>   

#include "stm32h7xx_hal.h"
#include "gpio.h"
#include "fatfs.h"
#include "sdmmc.h"
#include "adc.h"

#include "SDTest.h"
#include "ad7767.h"
#include "mpu6050.h"
#include "max3221.h"  // RS232
#include "ds1302.h"   // RTC
#include "demodu.h"



#define BUFF_SIZE (2500)  // 缓存数组的大小   



enum {   // 唤醒状态参数
			  Normal = 0,
				Wake_Up = 1,
				Wake_Work = 2,
};


static const char  * SystemTable[] = 
{
		"Normal",     /* 0 正常工作 */
		"Wake_Up",    /* 1 唤醒 */
		"Wake_Work"		/* 2 唤醒后工作 */
};

typedef struct 
{
	uint8_t IDdata;   // 8位ID数据
	uint8_t CMDdata;  // 8位CMD数据 		 
}ID_CMD;          	// 通信帧数据


// 变量
extern  volatile uint8_t  System_State;
extern	volatile uint32_t Wakeup_signal_cnt;     
extern	volatile uint32_t Wakeup_signal_Low_cnt;
	
extern  float  AD7767_Ping[BUFF_SIZE];  
extern  float  AD7767_Pang[BUFF_SIZE];
extern  float  *p_sd;     			//    SD卡写入
extern  float  *p_ad_begin; 

extern volatile bool Ping_full_flag;
extern volatile bool Pang_full_flag;

extern  volatile uint16_t addata_cnt; 
extern  int32_t RTC_Time[6]; 
//extern char RTC_Time[6]; 

extern uint16_t Timer4_Prescaler;
extern uint16_t Timer4_Period;
extern uint16_t Timer4_oc1_Pulse;
extern uint16_t Timer4_oc2_Pulse;

extern bool Minute_5_start; 
//extern bool volatile TIM7_1s_Flag;   
//extern uint16_t last_cmd;   



// 函数
void Power_ON(void); 
void Power_OFF(void);
float Read_battery(uint8_t	vref);
void Motor_rotate(uint8_t rotation);
void Enter_Standby_Mode(void);
void APP_Init(void);
void APP_Process (void);
void Scan_limit(void);
void Send_signel(void);
ID_CMD Demodulation(void);
void Send_response_cmd(void);
char* itoa(int num,char* str,int radix);

void Delay_S(uint16_t cnt);
void Cmd_delay(float time);
#endif
	
