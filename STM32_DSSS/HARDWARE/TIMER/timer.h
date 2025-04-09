#ifndef __TIMER_H
#define __TIMER_H

#include "system.h"

extern TIM_HandleTypeDef TIM3_Handler;      //定时器3PWM句柄 
extern TIM_OC_InitTypeDef TIM3_CH4Handler; 	//定时器3通道4句柄

void TIM3_Init(u16 arr,u16 psc);    		//定时器初始化
void TIM3_PWM_Init(u16 arr,u16 psc);
void TIM_SetTIM3Compare4(u32 compare);
u32 TIM_GetTIM3Capture4(void);
void TIM5_CH1_Cap_Init(u32 arr,u16 psc);

#endif /*__TIMER_H*/

