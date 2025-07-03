#include "cmd.h"
//#include "tim.h"
#include <math.h>

//  cmd  指令相关参数说明   包括发送流程、响应流程、数据处理等

/* 响应信号时间 */
/* 12K 1s 唤醒 + 0.5s +  9K-13K 40ms 调频 + 0.5s  + 单元16位 20ms 20ms */

/* 响应信号 */
/* 判断9K  9.5K是否有效响应 */

/* 指令说明 */
// 0X55  释放
// 0X49  量程
// 0X48  查询水位单元电压
// 0X47  查询姿态

#ifdef  ADC_PREAMP_2    // ADC前置放大2倍
        #define  threshold_ack    17246978048  // 17179869184
#endif
#ifdef  ADC_PREAMP_3
        #define  threshold_ack     25870467072  //25769803776   //   17179869184 * 1.5
#endif
#ifdef  ADC_PREAMP_11
        #define  threshold_ack    94858379264  //94489280512      // 17179869184 * 5.5
#endif
#ifdef  ADC_PREAMP_30
        #define  threshold_9K    258704670720   // 17179869184 * 15
#endif

#define ACK_SIGNAL_TABLE_NUM 100

volatile  bool TIM_20S_FLAG  = 0;
volatile  bool TIM_10S_FLAG  = 0;
volatile  bool order_respond = 0;
volatile  bool fun_respond   = 0;
volatile  bool stop_cnt_flag = 0;
volatile  uint8_t time_mode  = 0;   // 模式选择 10s/20s 响应超时

extern uint8_t StartT;

uint8_t id_hex;


/*----------------------------------------------------------------------------------------------------------  频率参数  ------------------------------------*/

////  输出PWM的OC参数说明  以35%占空比为例  时钟240M  计算如下：
//  OC参数  周期 = 时钟频率/2/目标频率  
//    如 10.2KHz = 240 000 000/ 2 /10200 = 11764.7 取11765  11765*0.35=4118.8 取4118   11765-4118=7647
const static TIM_FREQUENCE Single_Freq_Data0 [8]=   // bit0频率参数，占空比35%
{		             		   
        {0,  12000-1, 4200-1, 7800-1, 10   },   	// f1  10K     
        {0,  11538-1, 4038-1, 7500-1, 10.4 },   	// f2  10.4K
        {0,  11111-1, 3889-1, 7222-1, 10.8 }, 		// f3	 10.8K
        {0,  10714-1, 3750-1, 6964-1, 11.2 }, 		// f4  11.2K 
        {0,  10345-1, 3621-1, 6724-1, 11.6 }, 		// f5  11.6K 	
        {0,  10000-1, 3500-1, 6500-1, 12 	 }, 		// f6	 12K 		
        {0,  9677-1,  3387-1, 6290-1, 12.4 }, 		// f7  12.4K 	
        {0,  9375-1,  3281-1, 6094-1, 12.8 }, 		// f8	 12.8K 	
};		

const static TIM_FREQUENCE Single_Freq_Data1 [8]=   // bit1频率参数，占空比35%
{		 									
        {0,  11765-1, 4118-1, 7647-1, 10.2},  	// f1  	10.2K
        {0,  11321-1, 3962-1, 7359-1, 10.6},  	// f2  	10.6K
        {0,  10909-1, 3818-1, 7091-1, 11  },		// f3	 	11K
        {0,  10526-1, 3684-1, 6842-1, 11.4},		// f4   11.4K
        {0,  10169-1, 3560-1, 6609-1, 11.8},		// f5   11.8K
        {0,  9836-1,  3443-1, 6393-1, 12.2},		// f6	 	12.2K
        {0,  9524-1,  3333-1, 6191-1, 12.6},		// f7   12.6K
        {0,  9231-1,  3231-1, 6000-1, 13	},		// f8	  13K		
};		

const static uint8_t PN[8] = {1,6,7,3,8,2,4,5};  // PN序列

//占空比35%
const static TIM_FREQUENCE Single_Freq_Data2[5] = {
{0,13333-1,4667-1,8666-1,12},					// f1  9K 
{0,  12000-1, 4200-1, 7800-1, 10},   // f2  10K 
{0,  10909-1, 3818-1, 7091-1, 11},		// f3	 	11K
{0,10000-1,3500-1,6500-1,12},						// f4	 	12K
{0,9230-1,3230-1,6000-1,13},						// f5	 	13K
};

//占空比10%
const static TIM_FREQUENCE Single_Freq_Data3[5] = {
{0,13333-1,1333-1,8666-1,9},					// f1  9K 
{0,  12000-1, 1200-1, 10800-1, 10},   // f2  10K 
{0,  10909-1, 1090-1, 9819-1, 11},		// f3	 	11K
{0,10000-1,1000-1,9000-1,12},						// f4	 	12K
{0,9230-1,923-1,8307-1,13},						// f5	 	13K
};

//占空比8%
const static TIM_FREQUENCE Single_Freq_Data_8[5] = {
    {0, 13333-1, 1066-1, 12266-1, 9},    // 9 kHz (Pulse = round(13333 * 0.08) = 1066)
    {0, 12000-1, 960-1, 11040-1, 10},    // 10 kHz (Pulse = round(12000 * 0.08) = 960)
    {0, 10909-1, 873-1, 10036-1, 11},    // 11 kHz (Pulse = round(10909 * 0.08) = 873)
    {0, 10000-1, 800-1, 9200-1, 12},     // 12 kHz (Pulse = round(10000 * 0.08) = 800)
    {0,  9230-1, 738-1, 8492-1, 13},     // 13 kHz (Pulse = round(9230 * 0.08) = 738)
};

//占空比9%
const static TIM_FREQUENCE Single_Freq_Data_9[5] = {
    // Format: {0, ARR, Pulse, OffTime, Freq (kHz)}
    {0, 13333-1, 1200-1, 12133-1, 9},    // 9 kHz (Pulse = round(13333 * 0.09) = 1200)
    {0, 12000-1, 1080-1, 10920-1, 10},   // 10 kHz (Pulse = round(12000 * 0.09) = 1080)
    {0, 10909-1, 982-1, 9927-1, 11},     // 11 kHz (Pulse = round(10909 * 0.09) = 982)
    {0, 10000-1, 900-1, 9100-1, 12},     // 12 kHz (Pulse = round(10000 * 0.09) = 900)
    {0,  9230-1, 831-1, 8399-1, 13},     // 13 kHz (Pulse = round(9230 * 0.09) = 831)
};

//占空比5%
const static TIM_FREQUENCE Single_Freq_Data4[5] = {
{0,13333-1,666-1,12667-1,9},					// f1  9K 
{0,  12000-1, 600-1, 11400-1, 10},   // f2  10K 
{0,  10909-1, 545-1, 10364-1, 11},		// f3	 	11K'
{0,10000-1,500-1,9500-1,12},						// f4	 	12K
{0,9230-1,923-1,8307-1,13},						// f5	 	13K
};

//占空比2%
TIM_FREQUENCE Single_Freq_Data[5] = {
{0,13333-1,266-1,13067-1,9},					// f1  9K 
{0,  12000-1, 240-1, 11760-1, 10},   // f2  10K 
{0,  10909-1, 218-1, 10691-1, 11},		// f3	 	11K'
{0,10000-1,200-1,9800-1,12},						// f4	 	12K
{0,9230-1,923-1,8307-1,13},						// f5	 	13K
};

void Delay_10ms(uint16_t cnt)   // 定时器13 延时10ms
{
    __HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        // 清T13标志
    __HAL_TIM_SET_COUNTER(&htim13, 0);    // 计数器清零
    HAL_TIM_Base_Start_IT (&htim13 );                    // 开启T13中断.
    
    uint32_t timeout = 0xFFFF;  // 超时值
    while (cnt) {
            timeout = 0xFFFF;
            while (TIM13_10ms_Flag == 0 && timeout--) {
                    // 可选用 __NOP() 降低功耗
            }
            if (timeout == 0) {
                    break;  // 超时退出
            }
            cnt--;
            TIM13_10ms_Flag = 0;
    }
    
    HAL_TIM_Base_Stop_IT (&htim13 );              //关闭中断      
}	

static void Time4_change_freq(TIM_FREQUENCE freq)  /**改变T4频率**/
{
    timer4_Prescaler   = freq.prescaler;
    timer4_Period      = freq.period;
    timer4_oc1_Pulse   = freq.CH1_high;
    timer4_oc2_Pulse   = freq.CH2_high;
    MX_TIM4_Init();   // 信号输出  PWM
}

void Send_wakeup(void)  // 发送 唤醒信号
{
  Time4_change_freq(Single_Freq_Data3[3]);   // 12K ,10%
    
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
    
    Delay_10ms(80);  // 800ms
        
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
}

void Send_LFM(void)   // 发送线性调频信号
{
        printf("Send LFM\r\n");
    
        /* 配置PWM相关IO为输出 */
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        
      GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

        // 输出高低电平
        if(s1_data[0])
            GPIO_HIN->BSRR |= GPIO_BS_HIN; 
        else
            GPIO_HIN->BSRR |= GPIO_BR_HIN;
        
        if(s2_data[0])
            GPIO_LIN->BSRR |= GPIO_BS_LIN;
        else
            GPIO_LIN->BSRR |= GPIO_BR_LIN;
        
      // 启动定时器
        MX_TIM17_Init();

        __HAL_TIM_CLEAR_IT (&htim17 ,TIM_IT_UPDATE ); 
        TIM17->CNT = 0;   
        HAL_TIM_Base_Start_IT(&htim17);
        
        while(TIM17_40ms_FLAG == 0);  //等待40ms
        TIM17_40ms_FLAG = 0;
        
        GPIO_HIN->BSRR |= GPIO_BR_HIN;
        GPIO_LIN->BSRR |= GPIO_BR_LIN;
}

// 发送单频信号
// 拼接16位ID和CMD指令
void Send_single_frequency(bool* order_data)  
{	
         uint8_t i = 0;
        uint8_t PN_index = 0;               // PN序列  用于中间值存储
        TIM_FREQUENCE TIM_freq = {0};     // 频率

//		printf("\r频率打印\r\n");	
        for(i=0; i<16; i++)
        {
                PN_index = i%8;

                if((*order_data) == 0) {
                    TIM_freq = Single_Freq_Data0[(PN[PN_index])-1];
//					printf("0 freq %0.1f\r\n",TIM_freq.freq);	
                }
                else 
                { TIM_freq = Single_Freq_Data1[(PN[PN_index])-1];
//					printf("1 freq %0.1f\r\n",TIM_freq.freq);	
                }
                order_data++;
                
                Time4_change_freq(TIM_freq);		

                HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
                HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1); 
            
                Delay_10ms(20);  // 高电平20ms

                HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
                HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1);  

                Delay_10ms(20);  // 低电平20ms
        }
}

// 发送4种频率的测试帧
void Send_aframe1(void)  // 发送测试帧
{
  Time4_change_freq(Single_Freq_Data_9[0]);   // 9K ,35%
    
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
    
    Delay_10ms(20);  // 高电平20ms
        
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
    
    Delay_10ms(30);  // 低电平30ms
    
    Time4_change_freq(Single_Freq_Data_9[1]);   // 10K ,35%
    
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
    
    Delay_10ms(20);  // 高电平20ms
        
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
    
    Delay_10ms(30);  // 低电平30ms
    
      Time4_change_freq(Single_Freq_Data_9[2]);   // 11K ,35%
    
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
    
    Delay_10ms(20);  // 高电平20ms
        
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
    
    Delay_10ms(30);  // 低电平30ms
    
      Time4_change_freq(Single_Freq_Data_9[3]);   // 12K ,35%
    
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
    
    Delay_10ms(20);  // 高电平20ms
        
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
    HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
    
    Delay_10ms(30);  // 低电平30ms
}

#define SYMBOL_COUNT 40  // 10字节 * 4个2-bit符号

// 发送10字节数据帧
void Send_frame_from_hex(uint8_t hex_array[10]) {
    TIM_FREQUENCE freq_sequence[SYMBOL_COUNT];
    int symbol_idx = 0;

    // Step 1: 将10字节数据拆成40个2-bit符号，查表生成频率序列
    for (int byte_idx = 0; byte_idx < 10; byte_idx++) {
        uint8_t byte = hex_array[byte_idx];
        for (int i = 0; i < 4; i++) {
            uint8_t two_bits = (byte >> (6 - i * 2)) & 0x03;
            freq_sequence[symbol_idx++] = Single_Freq_Data[two_bits];
        }
    }

    // Step 2: 依次输出每个频率
    for (int i = 0; i < SYMBOL_COUNT; i++) {
        Time4_change_freq(freq_sequence[i]);

        HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_3);
        HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_1);

        Delay_10ms(20);  // 高电平20ms

        HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_3);
        HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_1);

        Delay_10ms(30);  // 低电平30ms
    }
}

void Deck_Send_frame(bool* order_data)   // 单元完整发送流程 唤醒 + LFM + 指令
{
    Set_Pin(POWER_CAP);   // 上电
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);  // 关闭ADC中断
    
    Reset_Pin(IR2110S_SD);     // 关闭功放  // 防止误发
    Send_wakeup();   // 12K唤醒信号  持续1秒
    Set_Pin(IR2110S_SD);   // 打开功放
    Delay_10ms(50);  // 延时500ms
    
    Reset_Pin(IR2110S_SD);  
    Send_LFM();      // 发送LFM信号  持续40ms
    Set_Pin(IR2110S_SD);   // 打开功放
    Delay_10ms(50);  // 延时500ms
    
    Reset_Pin(IR2110S_SD);  
    Send_single_frequency(order_data);  // 发送16位指令 
    Set_Pin(IR2110S_SD);   // 打开功放
    Delay_10ms(200);  // 延时200ms
    
    Reset_Pin(POWER_CAP);   // 断电
}

void Deck_Send(uint8_t hex_array[10])   // 单元完整发送流程 唤醒 + LFM + 数据帧
{
    Set_Pin(POWER_CAP);   // 上电
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);  // 关闭ADC中断
    
    Reset_Pin(IR2110S_SD);     // 关闭功放  // 防止误发
    Send_frame_from_hex(hex_array);   // 9K10K11K12k数据帧 
    Set_Pin(IR2110S_SD);   // 打开功放
    Delay_10ms(50);  // 延时500ms

    Reset_Pin(POWER_CAP);   // 断电
}

/*----------------------------------------------------------- 响应信号处理  ------------------------------------*/
const  int16_t ack_9K_tlabe[ACK_SIGNAL_TABLE_NUM] = { 
-2675,-1669,573,2360,2273,415,-1826,-2694,-1558,714,2407,2204,284,-1919,-2674,
-1454,834,2464,2135,140,-2021,-2648,-1317,962,2469,2044,33,-2085,-2640,-1235,
1087,2532,1977,-108,-2183,-2619,-1107,1212,2558,1888,-240,-2264,-2578,-984,
1325,2573,1796,-378,-2329,-2522,-856,1432,2576,1707,-505,-2407,-2480,-725,
1550,2585,1597,-645,-2452,-2411,-614,1643,2590,1505,-771,-2512,-2355,-467,
1753,2558,1374,-888,-2532,-2282,-354,1842,2557,1267,-1024,-2586,-2201,-213,
1925,2535,1162,-1149,-2622,-2119,-81,2004,2498,1052,-1255,-2650,-2050,49,};

const  int16_t ack_9_5K_tlabe[ACK_SIGNAL_TABLE_NUM] = 
{ 
    -2345,448,-853,-2872,-1918,579,2543,2297,101,-2266,-2749,-978,1592,2759,
    1531,-1039,-2784,-2200,159,2360,2503,505,-2012,-2857,-1348,1255,2744,1858,
    -629,-2657,-2458,-266,2130,2657,904,-1679,-2882,-1697,860,2650,2123,-210,
    -2428,-2639,-700,1800,2734,1300,-1295,-2832,-2011,440,2477,2339,217,-2165,
    -2751,-1085,1466,2727,1627,-893,-2727,-2286,13,2288,2550,621,-1897,-2838,
    -1454,1099,2702,1961,-491,-2595,-2518,-402,2020,2671,1036,-1531,-2854,-1821,
    693,2587,2225,-50,-2352,-2695,-839,1706,2754,1416,-1175,-2821,-2118,295,2436,2442
};

 int64_t  Receive_9K  [ACK_SIGNAL_TABLE_NUM] = {0};  
 int64_t  Receive_9_5K[ACK_SIGNAL_TABLE_NUM] = {0};

static  int32_t  ack_9K_temp = 0;
static  int32_t  ack_9_5K_temp = 0;
static  int64_t  ack_9K_sum = 0;
static  int64_t  ack_9_5K_sum = 0;

volatile uint16_t   correct_cnt = 0;
volatile uint8_t    index_100 = 0;

void clear_receive(void)  // 清除接收缓存，20S超时自动清除接收信息
{
    correct_cnt = 0;
    addata_cnt  = 0;
    index_100 = 0;
    HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); // 关闭ADC
} 

/*-----------------------------------------------------------   指令UI 相关  ------------------------------------*/
bool order_data[16] = {0};

bool*  order_convert(uint8_t CMD_data)   // 指令转8位
{
    uint8_t i = 0;
    uint8_t ID_data  = 0x5A;    // 固定值
    volatile bool id_data[8] = {0};
    volatile bool cmd_data[8] = {0};

    for(i=0;i<8;i++)
    {
        id_data[i] =  (ID_data<<i) & 0x80;
        cmd_data[i] = (CMD_data<<i) & 0x80;
    }
    for(i=0;i<8;i++)
    {
        order_data[i] = id_data[i];
        order_data[i+8] = cmd_data[i];
    }
    return order_data;
}

//校验
void append_xor_checksum(uint8_t ihex[10]) {
    uint8_t checksum = ihex[0];
    for (int i = 1; i < 9; i++) {
        checksum ^= ihex[i];
    }
    ihex[9] = checksum;
}

//16进制转float
float hex_bytes_to_float(uint8_t hex[10]) {
    uint32_t temp = ((uint32_t)hex[0] << 24) |
                    ((uint32_t)hex[1] << 16) |
                    ((uint32_t)hex[2] << 8) |
                    (uint32_t)hex[3];
    float value;
    memcpy(&value, &temp, sizeof(float));
    return value;
}

/* 定时  逻辑 未用 */
void CMD_55(void)   /* 释放指令 */
{
    //指令发送帧
    uint8_t ihex[10] = {id_hex,0x55,0x11,0x11,0x11,0x11,0x11,0x11,0x11};

    append_xor_checksum(ihex);
    
    printf("释放55\r\n");
    lcd_DisStr(4,0,"释放55:");  //验证显示

    Deck_Send(ihex);

    /* 启动定时20S*/
    adc7767_init();  //  ADC初始化  ADC启动采集
    time_mode = 20;
    __HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
    HAL_TIM_Base_Start_IT (&htim13 );                    
    
    lcd_clear_row(4,3);
    lcd_DisStr(4,3,"等待响应");
    printf("等待响应\r\n");

    while(TIM_20S_FLAG == 0)  
    { 
        if(1 == StartT )
        {
            stop_cnt_flag = 1;

            lcd_clear_row(2,3);
            lcd_DisStr(2,3,RESPONSE_TEMP[1]);   
            printf("收到响应\r\n");
        
            /* 再等10S */
            time_mode = 10;
            stop_cnt_flag = 0;
            while(TIM_10S_FLAG == 0)
            {
                if(1 == StartT )
                {
                    StartT = 0;
                    stop_cnt_flag = 1;
                    lcd_clear_row(4,3);
                    lcd_DisStr(4,3,"功能响应");
                    printf("功能响应\r\n");
                    time_mode = 0;
                    stop_cnt_flag = 0;
                    return;
                }
            }
            if(TIM_10S_FLAG == 1 )
            {
                TIM_10S_FLAG = 0;
                time_mode = 0;
                stop_cnt_flag = 0;  
                lcd_clear_row(4,3);
                lcd_DisStr(4,3,"功能超时");
                clear_receive(); 
                HAL_TIM_Base_Stop_IT (&htim13 ); 
                printf("55功能超时\r\n");
                return;
            }
        }
    }
    if(TIM_20S_FLAG == 1 )
    {
        TIM_20S_FLAG = 0;
        time_mode = 0;
        stop_cnt_flag = 0; 
        lcd_clear_row(4,3);
        lcd_DisStr(4,3,"指令超时");
        clear_receive();
        HAL_TIM_Base_Stop_IT (&htim13 ); 
        printf("55指令超时\r\n");
        return;
    }
}

void CMD_49(void)  /* 量程指令 */
{
    uint8_t ihex[10] = {id_hex,0x49,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    append_xor_checksum(ihex);

    float distance_temp = 0;
    char  distance[4]={0};

    printf("发送49\r\n");
    lcd_DisStr(4,0,"发送49指令");  //显示

    Deck_Send(ihex);

    /* 启动定时20S*/
    adc7767_init();  //  ADC初始化  ADC启动采集
    time_mode = 20;
    __HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
    HAL_TIM_Base_Start_IT (&htim13 );
    lcd_clear_row(4,3);
    lcd_DisStr(4,3,"等待响应");
    printf("等待响应\r\n");
    
    while(TIM_20S_FLAG == 0)  
    {
        if(1 == StartT )
        {
            StartT = 0;
            recorded_time =  (float)(TIM13_1s_cnt*100 + TIM13_100ms_cnt*10 + TIM13_10ms_cnt) / 100;  // 单位 S
            stop_cnt_flag = 1;
            lcd_clear_row(4,3);
            lcd_DisStr(4,3,"收到响应");
            printf("49收到响应\r\n");
            printf("响应时间 = %0.1f S\r\n",recorded_time );

            /* 距离 = 时间 * 声速 /2 */
            distance_temp = recorded_time *1500 /2;
            itoa( (int)(distance_temp) , distance, 10); 
            lcd_clear_row(3,3);
            lcd_DisStr(3,3,distance);
            clear_receive();  
            time_mode = 0;
            stop_cnt_flag = 0;
            HAL_TIM_Base_Stop_IT (&htim13 );
            recorded_time = 0;				
            printf("测距结果 = %s \r\n",distance );
            return;
        }
    }
    if(TIM_20S_FLAG == 1)
    {
        TIM_20S_FLAG = 0;
        recorded_time = 0;
        time_mode = 0;
        stop_cnt_flag = 0;  
        lcd_clear_row(4,3);
        lcd_DisStr(4,3,"指令超时");
        clear_receive(); 
        HAL_TIM_Base_Stop_IT (&htim13 ); 
        printf("指令超时\r\n");
        return;
    }
}

void CMD_48(void)  /* 查询电压指令 */
{
    uint8_t ihex[10] = {id_hex,0x48,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    append_xor_checksum(ihex);

    float voltage_temp = 0;
    char  Battery_voltage[10]={0};

    printf("发送48\r\n");
    lcd_DisStr(4,0,"发送48指令");  //显示

    Deck_Send(ihex);

    /* 启动定时20S*/
    adc7767_init();  //  ADC初始化  ADC启动采集
    time_mode = 20;
    __HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
    HAL_TIM_Base_Start_IT (&htim13 );                    
    
    lcd_clear_row(4,3);
    lcd_DisStr(4,3,"等待响应");
    printf("等待响应\r\n");
    
    while(TIM_20S_FLAG == 0)  
    {
        if(1 == StartT )
        {
            stop_cnt_flag = 1;
            lcd_clear_row(2,3);
            lcd_DisStr(2,3,RESPONSE_TEMP[1]);   
            printf("48收到响应\r\n");
        
            time_mode = 55;
            stop_cnt_flag = 0;

            voltage_temp = hex_bytes_to_float(hex);
            stop_cnt_flag = 1;
            lcd_clear_row(4,3);
            lcd_DisStr(4,3,"功能响应");
            printf("48功能响应\r\n");

            StartT = 0;

            sprintf(Battery_voltage, "%.2fV", voltage_temp);  // 保留2位小数+单位"V"
            lcd_clear_row(3,3);
            lcd_DisStr(3,3,Battery_voltage);
            time_mode = 0;
            stop_cnt_flag = 0;
            clear_receive(); 
            HAL_TIM_Base_Stop_IT (&htim13 ); 
            printf("测量电压 = %s \r\n",Battery_voltage);
            return;
        }
    }
    if(TIM_20S_FLAG == 1 )
    {
        TIM_20S_FLAG = 0;
        recorded_time = 0;
        time_mode = 0;
        stop_cnt_flag = 0;  
        lcd_clear_row(4,3);
        lcd_DisStr(4,3,"指令超时");
        clear_receive(); 
        HAL_TIM_Base_Stop_IT (&htim13 ); 
        printf("指令超时\r\n");
        return;
    }
}

void CMD_47(void)   /* 查询姿态指令 */
{
    uint8_t ihex[10] = {id_hex,0x47,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
    append_xor_checksum(ihex);

    float posture_temp = 0;
    char  posture[10]={0};

    printf("发送47\r\n");
    lcd_DisStr(4,0,"发送47指令");  

    Deck_Send(ihex);

    /* 启动定时20S*/
    adc7767_init();  //  ADC初始化  ADC启动采集
    time_mode = 20;
    __HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
    HAL_TIM_Base_Start_IT (&htim13 );                    
    
    lcd_clear_row(4,3);
    lcd_DisStr(4,3,"等待响应");
    printf("等待响应\r\n");
    
    while(TIM_20S_FLAG == 0)  
    {
        if(1 == StartT )
        {
            StartT = 0;
            stop_cnt_flag = 1;
            lcd_clear_row(2,3);
            lcd_DisStr(2,3,RESPONSE_TEMP[1]);   
            printf("47收到响应\r\n");
        
            time_mode = 55;
            stop_cnt_flag = 0;

            posture_temp = hex_bytes_to_float(hex);
            stop_cnt_flag = 1;
            
            lcd_clear_row(4,3);
            lcd_DisStr(4,3,"功能响应");
            printf("47功能响应\r\n");

            printf("响应时间 = %0.1f S\r\n",recorded_time );  //  范围 0-9S
            if(recorded_time > 9)
            {
                printf("超出响应范围\r\n");	
            }
                        
            sprintf(posture, "%.2f度", posture_temp);  // 保留2位小数+单位"度"
            lcd_clear_row(3,3);
            lcd_DisStr(3,3,posture);
            clear_receive();
            time_mode = 0;
            stop_cnt_flag = 0;	
            recorded_time = 0;
            HAL_TIM_Base_Stop_IT (&htim13 ); 
            printf("姿态角度 = %s \r\n",posture );	
            return;	
        }
    }
    if(TIM_20S_FLAG == 1 && order_respond == 0 )
    {
        TIM_20S_FLAG = 0;
        recorded_time = 0;
        time_mode = 0;
        stop_cnt_flag = 0; 
        lcd_clear_row(4,3);
        lcd_DisStr(4,3,"指令超时");
        clear_receive(); 
        HAL_TIM_Base_Stop_IT (&htim13 ); 
        printf("指令超时\r\n");
        return;
    }
}

void switch_cmd_do(void)  /* 指令分发 */
{
    id_hex = ((CMD.date_value[0] & 0x0F) << 4) | (CMD.date_value[1] & 0x0F);
    if(CMD.date_value[2]==5 && CMD.date_value[3]== 5)  // 释放指令55
            CMD_55();    

    else if(CMD.date_value[2]==4){
        switch (CMD.date_value[3]){
            case 7:CMD_47();break;    // 查询姿态47
            case 8:CMD_48();break;  	// 查询电压48
            case 9:CMD_49();break;    // 量程49
            default:printf("unknow cmd \r\n");break;
        }
     }
}