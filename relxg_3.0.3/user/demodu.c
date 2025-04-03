#include "demodu.h"
#include "usart.h"



//滤波器参数
#define PI 3.14159265358979323846
#define SAMPLING_RATE 62500  // 采样率
#define LEARNING_RATE 0.02   // 学习率 mu


// 目标频率
#define FREQ1 9000.0   // 第一个目标频率 9kHz
#define FREQ2 10000.0  // 第二个目标频率 10kHz
#define FREQ3 11000.0  // 第三个目标频率 11kHz
#define FREQ4 12000.0  // 第三个目标频率 11kHz

//// 用于计算瞬时频率的全局变量
//float prev_phase = 0.0f;


// 用于计算各滤波器总输出的全局变量
float y1_sum = 0.0f; 
float last_y1_sum = 0.0f; // 上一次 y1_sum 的值
uint32_t last_send_time = 0; // 上一次发送的时间

float y2_sum = 0.0f; 
float last_y2_sum = 0.0f; // 上一次 y1_sum 的值

float y3_sum = 0.0f; 
float last_y3_sum = 0.0f; // 上一次 y1_sum 的值

float last_y4_sum = 0.0f; // 上一次 y1_sum 的值
float y4_sum = 0.0f; 

#define THRESHOLD 0.5  // 触发滤波器输出的阈值
#define MIN_SEND_INTERVAL 17 // 最小发送间隔（单位：ms）

#define MAX_COUNT 80  // 数组最大存储数量
#define HEX_SIZE (MAX_COUNT / 4)  // 十六进制数组的大小
int da[80];  // 用于存储 8 个数字
// 转换后的十六进制数组
int hex[HEX_SIZE] = {0};
int da_index = 0;  // 当前数据插入位置




//// 将二进制数组（int 类型）转换为十六进制数组
//void binary_to_hex(const int* binary, unsigned char* hex, int binary_size) {
//    for (int i = 0; i < binary_size; i += 4) {
//        // 每 4 个二进制位转换为 1 个十六进制字符
//        unsigned char value = 0;
//        for (int j = 0; j < 4; j++) {
//            if (i + j < binary_size) {  // 防止数组越界
//                value |= (binary[i + j] << (3 - j)); // 将二进制位组合成一个十六进制字符
//            }
//        }
//        // 将值转换为十六进制字符
//        hex[i / 4] = (value < 10) ? ('0' + value) : ('A' + (value - 10));
//    }
//}

//将二进制数组（int 类型）转换为十六进制数组（int）
void binary_to_hex(int binary_array[MAX_COUNT], int hex_array[HEX_SIZE]) {
    for (int i = 0; i < HEX_SIZE; i++) {
        hex_array[i] = 0;  // 初始化为 0
        for (int j = 0; j < 4; j++) {
            hex_array[i] = (hex_array[i] << 1) | binary_array[i * 4 + j];  // 依次左移并添加位
        }
    }
}






// 初始化陷波滤波器
void init_notch_filter(AdaptiveNotchFilter *filter, float freq) {
    filter->uc = 0;
    filter->us = 0;
    filter->omega = 2.0 * PI * freq / SAMPLING_RATE;
}

// 自适应陷波滤波器的处理函数
float process_notch_filter(AdaptiveNotchFilter *filter, float desired_signal, int k) {
    float xc = cos(filter->omega * k);  // 正交参考信号 - 余弦分量
    float xs = sin(filter->omega * k);  // 正交参考信号 - 正弦分量

    // 滤波器输出
    float y = filter->uc * xc + filter->us * xs;

    // 计算误差
    float e = desired_signal - y;

    // 更新权重
    filter->uc += LEARNING_RATE * e * xc;
    filter->us += LEARNING_RATE * e * xs;

    return y;  // 返回当前通道的滤波输出
}

//// 主处理函数
//float multiChannelNotchFilter(float input_sample, int k) {
//    static AdaptiveNotchFilter filter1, filter2, filter3;
//    static int initialized = 0;

//    if (!initialized) {
//        init_notch_filter(&filter1, FREQ1);  // 初始化 9kHz 通道
//        init_notch_filter(&filter2, FREQ2);  // 初始化 10kHz 通道
//        init_notch_filter(&filter3, FREQ3);  // 初始化 11kHz 通道
//        initialized = 1;
//    }

//    // 并行滤波器处理
//    float y1 = process_notch_filter(&filter1, input_sample, k);
//    float y2 = process_notch_filter(&filter2, input_sample, k);
//    float y3 = process_notch_filter(&filter3, input_sample, k);

//    // 总残差输出
//    float output = y1 + y2 + y3;

//    return output;  // 返回多通道滤波器的输出
//}


// 主处理函数
float multiChannelNotchFilter1(float input_sample, int k) {
    static AdaptiveNotchFilter filter1;
    static int initialized = 0;

    if (!initialized) {
        init_notch_filter(&filter1, FREQ1);  // 初始化 9kHz 通道
        initialized = 1;
    }

    // 并行滤波器处理
    float y1 = process_notch_filter(&filter1, input_sample, k);


    return y1;  // 返回多通道滤波器的输出
}



// 主处理函数
float multiChannelNotchFilter2(float input_sample, int k) {
    static AdaptiveNotchFilter filter2;
    static int initialized = 0;

    if (!initialized) {
        init_notch_filter(&filter2, FREQ2);  // 初始化 9kHz 通道
        initialized = 1;
    }

    // 并行滤波器处理
    float y2 = process_notch_filter(&filter2, input_sample, k);


    return y2;  // 返回多通道滤波器的输出
}


// 主处理函数
float multiChannelNotchFilter3(float input_sample, int k) {
    static AdaptiveNotchFilter filter3;
    static int initialized = 0;

    if (!initialized) {
        init_notch_filter(&filter3, FREQ3);  // 初始化 9kHz 通道
        initialized = 1;
    }

    // 并行滤波器处理
    float y3 = process_notch_filter(&filter3, input_sample, k);


    return y3;  // 返回多通道滤波器的输出
}


// 主处理函数
float multiChannelNotchFilter4(float input_sample, int k) {
    static AdaptiveNotchFilter filter4;
    static int initialized = 0;

    if (!initialized) {
        init_notch_filter(&filter4, FREQ4);  // 初始化 9kHz 通道
        initialized = 1;
    }

    // 并行滤波器处理
    float y4 = process_notch_filter(&filter4, input_sample, k);


    return y4;  // 返回多通道滤波器的输出
}



//// 批量处理缓冲区数据
//void process_buffer(float *input_buffer, float *output_buffer, int buffer_size) {
//    for (int i = 0; i < buffer_size; i++) {
//        output_buffer[i] = multiChannelNotchFilter(input_buffer[i], i);
//    }
//}


// 处理缓冲区数据并求和
void process_buffer_and_sum1(float *input_buffer, int buffer_size) {
    

 for (int i = 0; i < buffer_size; i++) {
        // 对每个数据点应用滤波，并累加结果
        float filtered_value = multiChannelNotchFilter1(input_buffer[i], i);
        float fv = fabs(filtered_value);

        // 过滤低振幅信号
        if (fv <= THRESHOLD) {
            fv = 0;
        }
        y1_sum += fv;  // 累加绝对值

        // 检查 y1_sum 是否增大
        if (y1_sum > last_y1_sum) {
            uint32_t current_time = HAL_GetTick(); // 获取当前时间
            if ((current_time - last_send_time) >= MIN_SEND_INTERVAL) {
				
//				printf("00");
                

				  
                        da[da_index++] = 0;  // 这里可以替换成实际数据
                        da[da_index++] = 0;  // 这里可以替换成实际数据
                   
               
						// 检查数组是否满了，满了则存入 SD 卡并清空数组
						if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							 // 打印十六进制数据
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							
							// 清空数组和重置索引
							memset(da, 0, sizeof(da));  // 清空数组
							da_index = 0;  // 重置索引
						}
                // 更新发送时间
                last_send_time = current_time;
            }
        }

        // 更新 last_y1_sum
        last_y1_sum = y1_sum;
    }

    return ; // 返回总和
}


// 处理缓冲区数据并求和
void process_buffer_and_sum2(float *input_buffer, int buffer_size) {
    

 for (int i = 0; i < buffer_size; i++) {
        // 对每个数据点应用滤波，并累加结果
        float filtered_value = multiChannelNotchFilter2(input_buffer[i], i);
        float fv = fabs(filtered_value);

        // 过滤低振幅信号
        if (fv <= THRESHOLD) {
            fv = 0;
        }
        y2_sum += fv;  // 累加绝对值

        // 检查 y2_sum 是否增大
        if (y2_sum > last_y2_sum) {
            uint32_t current_time = HAL_GetTick(); // 获取当前时间
            if ((current_time - last_send_time) >= MIN_SEND_INTERVAL) {
				
//				printf("01");
                

				  
                        da[da_index++] = 0;  // 这里可以替换成实际数据
                        da[da_index++] = 1;  // 这里可以替换成实际数据
                   
               
						// 检查数组是否满了，满了则存入 SD 卡并清空数组
						if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							 // 打印十六进制数据
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							// 清空数组和重置索引
							memset(da, 0, sizeof(da));  // 清空数组
							da_index = 0;  // 重置索引

						}
                // 更新发送时间
                last_send_time = current_time;
            }
        }

        // 更新 last_y2_sum
        last_y2_sum = y2_sum;
    }

    return ; // 返回总和
}



// 处理缓冲区数据并求和
void process_buffer_and_sum3(float *input_buffer, int buffer_size) {
    

 for (int i = 0; i < buffer_size; i++) {
        // 对每个数据点应用滤波，并累加结果
        float filtered_value = multiChannelNotchFilter3(input_buffer[i], i);
        float fv = fabs(filtered_value);

        // 过滤低振幅信号
        if (fv <= THRESHOLD) {
            fv = 0;
        }
        y3_sum += fv;  // 累加绝对值

        // 检查 y3_sum 是否增大
        if (y3_sum > last_y3_sum) {
            uint32_t current_time = HAL_GetTick(); // 获取当前时间
            if ((current_time - last_send_time) >= MIN_SEND_INTERVAL) {
				
//				printf("10");
				
						da[da_index++] = 1;  // 这里可以替换成实际数据
                        da[da_index++] = 0;  // 这里可以替换成实际数据
						// 检查数组是否满了，满了则存入 SD 卡并清空数组
						if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							// 打印十六进制数据
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							// 清空数组和重置索引
							memset(da, 0, sizeof(da));  // 清空数组
							da_index = 0;  // 重置索引

						}
                // 更新发送时间
                last_send_time = current_time;
            }
        }

        // 更新 last_y3_sum
        last_y3_sum = y3_sum;
    }

    return ; // 返回总和
}



// 处理缓冲区数据并求和
void process_buffer_and_sum4(float *input_buffer, int buffer_size) {
    

 for (int i = 0; i < buffer_size; i++) {
        // 对每个数据点应用滤波，并累加结果
        float filtered_value = multiChannelNotchFilter4(input_buffer[i], i);
        float fv = fabs(filtered_value);

        // 过滤低振幅信号
        if (fv <= THRESHOLD) {
            fv = 0;
        }
        y4_sum += fv;  // 累加绝对值

        // 检查 y4_sum 是否增大
        if (y4_sum > last_y4_sum) {
            uint32_t current_time = HAL_GetTick(); // 获取当前时间
            if ((current_time - last_send_time) >= MIN_SEND_INTERVAL) {
                // 向串口发送数据
//				printf("11\n");

				   da[da_index++] = 1;  // 这里可以替换成实际数据
                        da[da_index++] = 1;  // 这里可以替换成实际数据
						// 检查数组是否满了，满了则存入 SD 卡并清空数组
						if (da_index >= MAX_COUNT) {
							binary_to_hex(da,hex);
							// 打印十六进制数据
							for (int j = 0; j < HEX_SIZE; j++) {  // 每个字节转换为2个十六进制字符
								printf("%X", hex[j]);
							}
							// 清空数组和重置索引
							
							memset(da, 0, sizeof(da));  // 清空数组
							da_index = 0;  // 重置索引
//							da[da_index++] = 1;  // 这里可以替换成实际数据
//                        da[da_index++] = 1;  // 这里可以替换成实际数据
						}
                // 更新发送时间
                last_send_time = current_time;
            }
        }

        // 更新 last_y4_sum
        last_y4_sum = y4_sum;
    }

    return ; 
}






//// 非阻塞发送数据
//void send_data_non_blocking(uint8_t *data, uint16_t size)
//{
//    HAL_UART_Transmit_IT(&huart3, data, size);  // 发送数据
//	return ;
//}


//// 串口发送完成回调
//void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
//{
//    if (huart->Instance == USART3) {
//        // 数据传输完成后，可以执行其他操作，如发送完成标志等
//        printf("Data transmission complete.\n");
//    }
//}




//// 处理缓冲区数据并求和
//void process_buffer_and_sum(float *input_buffer, int buffer_size) {
//    

//    for (int i = 0; i < buffer_size; i++) {
//        // 对每个数据点应用滤波，并累加结果
//		float filtered_value = multiChannelNotchFilter1(input_buffer[i], i);
//		float filtered_value2 = multiChannelNotchFilter2(input_buffer[i], i);
//		float filtered_value3 = multiChannelNotchFilter3(input_buffer[i], i);
//		float fv = fabs(filtered_value);
//		float fv2 = fabs(filtered_value2);
//		float fv3 = fabs(filtered_value3);
//		if(fv<=0.5){
//		fv=0;
//		}
//		if(fv2<=0.5){
//		fv2=0;
//		}
//		if(fv3<=0.5){
//		fv3=0;
//		}
//        y1_sum += fv;  // 累加绝对值
//		y2_sum += fv2;  // 累加绝对值
//		y3_sum += fv3;  // 累加绝对值
//  }

//    return ; // 返回总和
//}






//// 计算瞬时频率
//float calculate_instantaneous_frequency(float signal_value) {
//    // 使用 atan2 或 atan 计算当前采样点的相位
//    float phase = atan(signal_value);  // 适用于单通道信号

//    // 计算相位差（避免跨越问题）
//    float delta_phase = phase - prev_phase;

//    // 处理相位跨越
//    if (delta_phase > PI) {
//        delta_phase -= 2 * PI;
//    } else if (delta_phase < -PI) {
//        delta_phase += 2 * PI;
//    }

//    // 更新上一个相位值
//    prev_phase = phase;
//	
//    // 确保频率为正
//    float frequency = (delta_phase * SAMPLING_RATE) / (2 * PI);
//    if (frequency < 0) {
//        frequency = -frequency;  // 或根据需要设置为零 frequency = 0;
//    }

//    return frequency;
//}





//// 处理整个缓冲区的瞬时频率
//void process_fre(float *input_buffer, float *output_buffer, int buffer_size) {
//    for (int i = 0; i < buffer_size; i++) {
//        output_buffer[i] = calculate_instantaneous_frequency(input_buffer[i]);
//    }
//}




