#ifndef __Demodu_H__
#define __Demodu_H__





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




void frm_sych(void);
void demodu_process(void);



#endif


