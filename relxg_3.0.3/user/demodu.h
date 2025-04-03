#ifndef __Demodu_H__
#define __Demodu_H__




typedef struct {
    float uc, us;    // 正交权重
    float omega;     // 角频率
} AdaptiveNotchFilter;

void init_notch_filter(AdaptiveNotchFilter *filter, float freq);
float process_notch_filter(AdaptiveNotchFilter *filter, float desired_signal, int k);
float multiChannelNotchFilter(float input_sample, int k);
float calculate_instantaneous_frequency(float signal_value);
void process_buffer(float *input_buffer, float *output_buffer, int buffer_size);


void frm_sych(void);
void demodu_process(void);



#endif


