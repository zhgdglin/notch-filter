#ifndef __Demodu_H__
#define __Demodu_H__

#include "usart.h"


#define MAX_COUNT 80
//#define HEX_SIZE (MAX_COUNT / 4)  // ʮ����������Ĵ�С
#define HEX_SIZE (MAX_COUNT / 8)  // ʮ����������Ĵ�С

typedef struct {
    float uc, us;    // ����Ȩ��
    float omega;     // ��Ƶ��
} AdaptiveNotchFilter;


// ״̬����
typedef enum {
    SIGNAL_IDLE,     // ��ʼ״̬�����ź�
    SIGNAL_ACTIVE,   // �źż����
    SIGNAL_ENDED     // �źŽ���
} SignalState;



void process_buffer_and_sum(float *input_buffer, int buffer_size);
void frm_sych(void);
void demodu_process(void);
void check_da_timeout(void);

extern uint32_t da_index;
extern uint8_t da[MAX_COUNT];
extern uint8_t hex[HEX_SIZE];
extern uint16_t TIM7_cnt;

#endif


