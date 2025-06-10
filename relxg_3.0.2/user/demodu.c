#include "demodu.h"


#define PI 3.14159265358979323846
#define SAMPLING_RATE 62500
#define LEARNING_RATE 0.02

#define FREQ1 9000.0
#define FREQ2 10000.0
#define FREQ3 11000.0
#define FREQ4 12000.0

#define THRESHOLD 1.2
#define MIN_HIGH_SAMPLES 10
#define MIN_LOW_SAMPLES 200

uint8_t da[MAX_COUNT];
uint8_t hex[HEX_SIZE] = {0};
uint16_t da_index = 0;
uint16_t TIM7_cnt = 0;
uint8_t StartT = 0;
ID_CMD message;

AdaptiveNotchFilter filter1 = {0, 0, 2.0 * PI * 9000 / 62500};
AdaptiveNotchFilter filter2 = {0, 0, 2.0 * PI * 10000 / 62500};
AdaptiveNotchFilter filter3 = {0, 0, 2.0 * PI * 11000 / 62500};
AdaptiveNotchFilter filter4 = {0, 0, 2.0 * PI * 12000 / 62500};


////将二进制数组（int 类型）转换为十六进制数组（int）
//void binary_to_hex(uint8_t binary_array[MAX_COUNT], uint8_t hex_array[HEX_SIZE]) {
//    for (int i = 0; i < HEX_SIZE; i++) {
//        hex_array[i] = 0;  // 初始化为 0
//        for (int j = 0; j < 4; j++) {
//            hex_array[i] = (hex_array[i] << 1) | binary_array[i * 4 + j];  // 依次左移并添加位
//        }
//    }
//}



//
void binary_to_hex(uint8_t binary_array[MAX_COUNT], uint8_t hex_array[HEX_SIZE]) {
    for (int i = 0; i < HEX_SIZE; i++) {
        hex_array[i] = 0;  // 初始化
        for (int j = 0; j < 8; j++) {
            hex_array[i] = (hex_array[i] << 1) | binary_array[i * 8 + j];
        }
    }
}

//计算异或
uint8_t calculate_checksum(uint8_t *data, int len) {
    uint8_t checksum = data[0];
    for (int i = 1; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}


//辅助校验
bool parse_frame(uint8_t *hex) {
    if (hex[9] != calculate_checksum(hex, 9)) {
        return false;  // 校验失败
    }
    return true;
}





float multiChannelNotchFilter(float desired_signal, int k, AdaptiveNotchFilter *filter) {
    float xc = cos(filter->omega * k);
    float xs = sin(filter->omega * k);
    float y = filter->uc * xc + filter->us * xs;
    float e = desired_signal - y;
    filter->uc += LEARNING_RATE * e * xc;
    filter->us += LEARNING_RATE * e * xs;
    return y;
}

SignalState envelope_detection_channel(float filtered_value, SignalState *channel_state, int *high_count_ch, int *low_count_ch) {
    float fv = fabs(filtered_value);
    SignalState result_state = *channel_state;

    switch (*channel_state) {
        case SIGNAL_IDLE:
            if (fv >= THRESHOLD) {
                (*high_count_ch)++;
                if (*high_count_ch >= MIN_HIGH_SAMPLES) {
                    *channel_state = SIGNAL_ACTIVE;
                }
            }
            break;

        case SIGNAL_ACTIVE:
            if (fv < THRESHOLD) {
                (*low_count_ch)++;
                if (*low_count_ch >= MIN_LOW_SAMPLES) {
                    *channel_state = SIGNAL_ENDED;
                    result_state = SIGNAL_ENDED;
//									printf("%d",*low_count_ch);
                }
            }
            break;

        case SIGNAL_ENDED:
            *channel_state = SIGNAL_IDLE;
            *high_count_ch = 0;
            *low_count_ch = 0;
            break;
    }

    return result_state;
}

SignalState state1 = SIGNAL_IDLE, state2 = SIGNAL_IDLE, state3 = SIGNAL_IDLE, state4 = SIGNAL_IDLE;
int high_count1 = 0, low_count1 = 0;
int high_count2 = 0, low_count2 = 0;
int high_count3 = 0, low_count3 = 0;
int high_count4 = 0, low_count4 = 0;

void process_buffer_and_sum(float *input_buffer, int buffer_size) {
	uint8_t ihex[HEX_SIZE] = {0x1B,0x1B,0x1B,0x1B,0x1b,0x1b,0x1b,0x1b,0x1b,0x1b};
	
    for (int i = 0; i < buffer_size; i++) {
        float y1 = multiChannelNotchFilter(input_buffer[i], i, &filter1);
        float y2 = multiChannelNotchFilter(input_buffer[i], i, &filter2);
        float y3 = multiChannelNotchFilter(input_buffer[i], i, &filter3);
        float y4 = multiChannelNotchFilter(input_buffer[i], i, &filter4);
				
			envelope_detection_channel(y1, &state1, &high_count1, &low_count1);
			envelope_detection_channel(y2, &state2, &high_count2, &low_count2);
			envelope_detection_channel(y3, &state3, &high_count3, &low_count3);
			envelope_detection_channel(y4, &state4, &high_count4, &low_count4);
			
        if (state1== SIGNAL_ENDED){
            da[da_index++] = 0;
					  da[da_index++] = 0;
            if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							for (int j = 0; j < HEX_SIZE; j++){  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							
//							if(memcmp(hex,ihex,HEX_SIZE)==0){
//								StartT = 1;
////							HAL_Delay(5000);
////							communication_process(Demodulation());
//							}
							
							if (parse_frame(hex)) {
											// 解帧成功，可使用 frame.IDdata 和 frame.CMDdata
											StartT = 1;
									} else {
											// 校验失败，可记录错误次数或重传
									}
							
							
							TIM7_cnt = 0;
							da_index = 0;
							memset(da, 0, sizeof(da));  // 清空数组
							
						}
						}
        if (state2 == SIGNAL_ENDED) {
            da[da_index++] = 0;
						da[da_index++] = 1;			
            if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							
//							if(memcmp(hex,ihex,HEX_SIZE)==0){
//								StartT = 1;
////							HAL_Delay(5000);
////							communication_process(Demodulation());
//							}
							
							
							if (parse_frame(hex)) {
												// 解帧成功，可使用 frame.IDdata 和 frame.CMDdata
												StartT = 1;
										} else {
												// 校验失败，可记录错误次数或重传
										}
							
							TIM7_cnt = 0;
							da_index = 0;
							memset(da, 0, sizeof(da));  // 清空数组
							
						}
				}
        if (state3 == SIGNAL_ENDED) {
            da[da_index++] = 1;
						da[da_index++] = 0;
            if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							
//							if(memcmp(hex,ihex,HEX_SIZE)==0){
//								StartT = 1;
////							HAL_Delay(5000);
////							communication_process(Demodulation());
//							}
							
							
							if (parse_frame(hex)) {
										// 解帧成功，可使用 frame.IDdata 和 frame.CMDdata
										StartT = 1;
								} else {
										// 校验失败，可记录错误次数或重传
								}
							
							TIM7_cnt = 0;
							da_index = 0;
							memset(da, 0, sizeof(da));  // 清空数组
							
						}
				}
        if (state4 == SIGNAL_ENDED) {
            da[da_index++] = 1;
					  da[da_index++] = 1;
            if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							
//							if(memcmp(hex,ihex,HEX_SIZE)==0){
//								StartT = 1;
////							HAL_Delay(5000);
////							communication_process(Demodulation());
//							}
							
							
							if (parse_frame(hex)) {
											// 解帧成功，可使用 frame.IDdata 和 frame.CMDdata
											StartT = 1;
									} else {
											// 校验失败，可记录错误次数或重传
									}
							
							
							
							TIM7_cnt = 0;
							da_index = 0;
							memset(da, 0, sizeof(da));  // 清空数组
							
						}
				}
    }
}
