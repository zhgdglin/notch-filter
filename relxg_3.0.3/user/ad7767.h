#ifndef __AD7767_H__
#define __AD7767_H__

#include "APP_Work.h"
#include "tim.h"   // 用于使用us延时
#include "spi.h"





/* RS2253 ADC采集前置放大器 倍数调整 */
#define RS2253_X  Set_Pin(RS2253_A);Reset_Pin(RS2253_B);Reset_Pin(RS2253_C)
#define RS2253_Y  Reset_Pin(RS2253_A);Set_Pin(RS2253_B);Reset_Pin(RS2253_C)
#define RS2253_Z  Reset_Pin(RS2253_A);Reset_Pin(RS2253_B);Set_Pin(RS2253_C)

// 前置放大倍数   （无法叠加） 
#define PREAMP_2    Reset_Pin(RS2253_A);Reset_Pin(RS2253_B);Reset_Pin(RS2253_C)      // 默认2倍放大  对应电阻R99 = 20K
#define PREAMP_3    RS2253_X																												 // 3倍放大			 对应电阻R100 = 30K
#define PREAMP_11   RS2253_Y																												 // 11倍放大		 对应电阻R101 = 110K
#define PREAMP_30   RS2253_Z																												 // 30倍放大		 对应电阻R102 = 300K
/* RS2253 ADC采集前置放大器 倍数调整 */




int32_t S24toS32(int32_t input);
void adc7767_init(void);
void get_adc(void);
void change_buff(void);

#endif

