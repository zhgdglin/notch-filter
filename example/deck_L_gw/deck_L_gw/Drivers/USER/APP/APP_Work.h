#ifndef _APP_Work_H_
#define _APP_Work_H_

//#define u8 unsigned char   // 提高变量定义的移植性
//#define u16 unsigned int   // 提高变量定义的移植性
#define  GPIO_HIN       GPIOD
#define  GPIO_BS_HIN    GPIO_BSRR_BS14   
#define  GPIO_BR_HIN    GPIO_BSRR_BR14    // PD14
#define  GPIO_ODR_HIN   GPIO_ODR_OD14    
#define  GPIO_LIN			  GPIOD
#define  GPIO_BR_LIN    GPIO_BSRR_BR12    // PD12
#define  GPIO_BS_LIN    GPIO_BSRR_BS12    // PD12
#define  GPIO_ODR_LIN   GPIO_ODR_OD12   



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
#include "max3221.h"  // RS232
#include "ds1302.h"   // RTC


#define u8 unsigned char   // 提高变量定义的移植性
#define u16 unsigned int  

#include "bsp_lcd12864.h"
#include "Key.h"
#include "UI.h"
#include "process.h"
#include "cmd.h"


#define ADC_FS  62500     // Hz
#define SINGAL_TIME  0.04  // S

#define LFM_LENGTH 2500  //   ADC_FS *  SINGAL_TIME

#define BUFF_SIZE (100)  // ADC的缓存数组  

#define ADC_PREAMP_30   // ADC前放倍数选择    ADC_PREAMP_2   ADC_PREAMP_3   ADC_PREAMP_11   ADC_PREAMP_30

#define DEVICE_ID  0x01



// 枚举和结构 
typedef struct 
{
	uint16_t prescaler;    // 分频数
	uint16_t period;			 // 计算值 
	uint16_t CH1_high;	 	 // 通道1 高电平时间， 占空比1 =  CH1_high / period
	uint16_t CH2_high;		 // 通道2 高电平时间， 占空比2 =  CH2_high / period
	float freq;						 // 对应频率值标识  KHz
}TIM_FREQUENCE;


typedef struct 
{
	uint8_t IDdata;   // 8位ID数据
	uint8_t CMDdata;  // 8位CMD数据 		 
}ID_CMD;          	// 通信帧数据


typedef enum
{
  MAIN_PAGE = 0,
	CONTROL_PAGE,
	SINGAL_SET_PAGE,
	DEVICE_INFO_PAGE,
	RUNNING_CMD_PAGE,
	MESSAGE_PAGE,
	CMD_CYCLE_PAGE,
	FINISH_CMD_PAGE,
} MENU;

typedef struct Menu
{
    uint8_t Current_Page ;  /*当前正在执行的页面*/
    int KeyEvent ;   /*当前触发的事件*/
} Menu;

typedef struct KEY
{
    /*当前保存中断键值*/ 
    uint8_t Current_Key_Value ;
//    /*当前触发的事件*/
//    int KeyEvent ;
} KEY ;

typedef struct DATE
{
	u8  date_num;
	u8  date_value[4];
	u8	full_status;
}Date;


extern KEY exit_key;
extern Menu menu;
extern Date ID;
extern Date CMD;

// 变量
extern	volatile bool  TEST_20ms_Flag;  
extern	volatile uint8_t TIM13_10ms_cnt;
extern  volatile uint8_t TIM13_100ms_cnt;
extern  volatile uint8_t TIM13_1s_cnt;
	
//extern  int32_t  AD7767_Ping[BUFF_SIZE];  
//extern  int32_t  AD7767_Pang[BUFF_SIZE];
extern  volatile  float    ad7767_data;
extern volatile  float     ad7767_data_16B;

//extern  int32_t  *p_sd;     			//    SD卡写入
//extern  int32_t  *p_ad_begin; 

extern volatile bool Ping_full_flag;
extern volatile bool Pang_full_flag;

extern  volatile uint16_t addata_cnt; 
extern  int32_t RTC_Time[6]; 

extern	uint16_t timer4_Prescaler ;
extern	uint16_t timer4_Period	  ;
extern	uint16_t timer4_oc1_Pulse ;
extern	uint16_t timer4_oc2_Pulse ;
extern  volatile uint8_t TIM13_10ms_Flag;

/*process*/
extern char RESPONSE_TEMP[][30];
/*cmd*/
extern volatile  bool TIM_20S_FLAG;
extern volatile  bool TIM_10S_FLAG;
extern volatile  bool order_respond;
extern volatile  bool fun_respond;
extern volatile  bool stop_cnt_flag;   
extern volatile  uint8_t time_mode;
extern float   recorded_time;


/* chirp.h */
extern const	uint8_t s1_data[LFM_LENGTH];
extern const	uint8_t s2_data[LFM_LENGTH];
extern volatile bool  TIM17_40ms_FLAG; 

// 函数
float Read_battery(uint8_t	vref);
void APP_Init(void);
void APP_Process (void);
void Send_response_cmd(void);
char* itoa(int num,char* str,int radix);

#endif
	
