#ifndef __Demodu_H__
#define __Demodu_H__

#include "usart.h"


#define MAX_COUNT 80
//#define HEX_SIZE (MAX_COUNT / 4)  // 十六进制数组的大小
#define HEX_SIZE (MAX_COUNT / 8)  // 十六进制数组的大小

typedef struct {
    float uc, us;    // 正交权重
    float omega;     // 角频率
} AdaptiveNotchFilter;


// 状态定义
typedef enum {
    SIGNAL_IDLE,     // 初始状态，无信号
    SIGNAL_ACTIVE,   // 信号检测中
    SIGNAL_ENDED     // 信号结束
} SignalState;



void process_buffer_and_sum(float *input_buffer, int buffer_size);
void frm_sych(void);
void demodu_process(void);

extern uint32_t da_index;
extern uint8_t da[MAX_COUNT];
extern uint8_t hex[HEX_SIZE];
extern uint16_t TIM7_cnt;

#endif


