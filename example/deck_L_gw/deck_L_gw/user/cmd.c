#include "cmd.h"
//#include "tim.h"
#include <math.h>

//  cmd  包含 和水下单元的通信过程   发射信号  接收应答信号  和 命令执行



/* 发射信号组成 */
/* 12K 1s  唤醒 + 0.5s +  9K-13K 40ms 线性调频 + 0.5s  + 码元16个 20ms 20ms */

/* 接收信号*/
/* 判断9K  9.5K是否有效即可 */


/* 命令表 */
// 0X55  释放
// 0X49  测距
// 0X48  查询水下单元电池电压
// 0X47  查询姿态

#ifdef  ADC_PREAMP_2    // 根据前放倍数调整识别阈值 
		#define  threshold_ack    17246978048  // 17179869184
#endif
#ifdef  ADC_PREAMP_3
		#define  threshold_9K     25870467072  //25769803776   //   17179869184 * 1.5
#endif
#ifdef  ADC_PREAMP_11
		#define  threshold_9K    94858379264  //94489280512      // 17179869184 * 5.5
#endif
#ifdef  ADC_PREAMP_30
		#define  threshold_9K    258704670720   // 17179869184 * 15
#endif

#define ACK_SIGNAL_TABLE_NUM 100




volatile  bool TIM_20S_FLAG  = 0;
volatile  bool TIM_10S_FLAG  = 0;
volatile  bool order_respond = 0;
volatile  bool fun_respond 	 = 0;
volatile  bool stop_cnt_flag = 0;
volatile  uint8_t time_mode	 = 0;   // 选择 倒计时10s，倒计时20s ，正计时


/*----------------------------------------------------------------------------------------------------------  发射信号  ------------------------------------*/

////  数据说明： 用于 OC模式定时器 的频率改变  ，四舍五入 （ 定时器240M ），占空比35%，
////  OC模式  计数值 = 定时器频率/2/目标频率  
//    如 10.2KHz = 240 000 000/ 2 /10200 = 11764.7 取11765  11765*0.35=4118.8 取4118   11765-4118=7647
const static TIM_FREQUENCE Single_Freq_Data0 [8]=   // bit0对应的频率,占空比35%，
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

const static TIM_FREQUENCE Single_Freq_Data1 [8]=   // bit1对应的频率,占空比35%，
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

const static uint8_t PN[8] = {1,6,7,3,8,2,4,5};  // 随机序列


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
{0,9230-1,923-1,8307-1,13},						// f5	 	13K
{0,10000-1,1000-1,9000-1,12},						// f4	 	12K
};



void Delay_10ms(uint16_t cnt)   // 定时器13 延时10ms
{
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        // 清除T13标志
	__HAL_TIM_SET_COUNTER(&htim13, 0);   								 // 清零
	HAL_TIM_Base_Start_IT (&htim13 );                    // 允许T13中断.
	
uint32_t timeout = 0xFFFF;  // 超时阈值
	while (cnt) {
			timeout = 0xFFFF;
			while (TIM13_10ms_Flag == 0 && timeout--) {
					// 可选：插入 __NOP() 或轻量级任务
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
	timer4_Prescaler 		= freq.prescaler;
	timer4_Period   	 	= freq.period;
	timer4_oc1_Pulse 		= freq.CH1_high;
	timer4_oc2_Pulse 		= freq.CH2_high;
	MX_TIM4_Init();   // 信号发射  PWM
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


 
void Send_LFM(void)   // 发送  线性调频 信号
{
		printf("Send LFM\r\n");
	
		/* 不使用PWM模式，将引脚配置成输出模式 */
		GPIO_InitTypeDef GPIO_InitStruct = {0};
		
	  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

		// 端口初值
		if(s1_data[0])
			GPIO_HIN->BSRR |= GPIO_BS_HIN; 
		else
			GPIO_HIN->BSRR |= GPIO_BR_HIN;
		
		if(s2_data[0])
			GPIO_LIN->BSRR |= GPIO_BS_LIN;
		else
			GPIO_LIN->BSRR |= GPIO_BR_LIN;
		
	  // 初始化定时器
		MX_TIM17_Init();

		__HAL_TIM_CLEAR_IT (&htim17 ,TIM_IT_UPDATE ); 
		TIM17->CNT = 0;		
		HAL_TIM_Base_Start_IT(&htim17);
		
		while(TIM17_40ms_FLAG == 0);  //等待40ms结束
		TIM17_40ms_FLAG = 0;
		
		GPIO_HIN->BSRR |= GPIO_BR_HIN;
		GPIO_LIN->BSRR |= GPIO_BR_LIN;
}



// 发送跳频调制信号
// 传送一个16位拼接好ID和CMD的数组
void Send_single_frequency(bool* order_data)  
{	
 		uint8_t i = 0;
		uint8_t PN_index = 0;               // 随机序列索引
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
			
				Delay_10ms(2);  // 信号持续20ms

				HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
				HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1);  

				Delay_10ms(2);  // 空闲20ms
		}
}


// 发送一帧4个频点单频信号
void Send_aframe1(void)  // 发送 唤醒信号
{
  Time4_change_freq(Single_Freq_Data3[0]);   // 9K ,35%
	
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
	
	Delay_10ms(20);  // 发送20ms
		
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
	
	Delay_10ms(30);  // 空闲30ms
	
	Time4_change_freq(Single_Freq_Data3[1]);   // 10K ,35%
	
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
	
	Delay_10ms(20);  // 发送20ms
		
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
	
	Delay_10ms(30);  // 空闲30ms
	
	  Time4_change_freq(Single_Freq_Data3[2]);   // 11K ,35%
	
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
	
	Delay_10ms(20);  // 发送20ms
		
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
	
	Delay_10ms(30);  // 空闲30ms
	
	  Time4_change_freq(Single_Freq_Data3[3]);   // 12K ,35%
	
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);   
	
	Delay_10ms(20);  // 发送20ms
		
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1); 
	
	Delay_10ms(30);  // 空闲30ms
}





void Deck_Send_frame(bool* order_data)   // 甲板单元发送一帧数据 ：唤醒 + 线性调频 + 调制信号*/
{
	Set_Pin(POWER_CAP);   // 开启发射
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);  // 关闭接收
  Reset_Pin(IR2110S_SD);     //开启输出，低有效  // 装上变压器以后开启
	
	Send_wakeup();   // 12K唤醒信号  持续1秒
	Set_Pin(IR2110S_SD);   // 结束输出
	Delay_10ms(50);  // 空闲500ms
	
	Reset_Pin(IR2110S_SD);  
	Send_LFM();      // 线性调频信号  持续40ms
	Set_Pin(IR2110S_SD);   // 结束输出
	Delay_10ms(50);  // 空闲500ms
	
	Reset_Pin(IR2110S_SD);  
	Send_single_frequency(order_data);  // 发送16个数据 
	Set_Pin(IR2110S_SD);   // 结束输出
  Delay_10ms(200);  // 连续调用测试用
	
	Reset_Pin(POWER_CAP);   // 关闭发射
}



void Serial_Send_frame(void)   // 甲板单元发送一帧数据 ：唤醒 + 线性调频 + 调制信号*/
{
	Set_Pin(POWER_CAP);   // 开启发射
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);  // 关闭接收
  Reset_Pin(IR2110S_SD);     //开启输出，低有效  // 装上变压器以后开启
	
	Send_aframe1();   // 9，10，11，12k单频信号 
	Set_Pin(IR2110S_SD);   // 结束输出
	Delay_10ms(50);  // 空闲500ms
	

	Reset_Pin(POWER_CAP);   // 关闭发射
}



#define SYMBOL_COUNT 40  // 10字节 * 4个2-bit 符号



void Send_frame_from_hex(uint8_t hex_array[10]) {
    TIM_FREQUENCE freq_sequence[SYMBOL_COUNT];
    int symbol_idx = 0;

    // Step 1: 将10字节数据拆成40段2-bit并映射为频率配置
    for (int byte_idx = 0; byte_idx < 10; byte_idx++) {
        uint8_t byte = hex_array[byte_idx];
        for (int i = 0; i < 4; i++) {
            uint8_t two_bits = (byte >> (6 - i * 2)) & 0x03;
            freq_sequence[symbol_idx++] = Single_Freq_Data3[two_bits];
        }
    }

    // Step 2: 依次发送每个频率信号
    for (int i = 0; i < SYMBOL_COUNT; i++) {
        Time4_change_freq(freq_sequence[i]);

        HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_3);
        HAL_TIM_OC_Start(&htim4, TIM_CHANNEL_1);

        Delay_10ms(2);  // 发送20ms

        HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_3);
        HAL_TIM_OC_Stop(&htim4, TIM_CHANNEL_1);

        Delay_10ms(3);  // 空闲30ms
    }
}






/*----------------------------------------------------------- 接收并判断 应答信号  ------------------------------------*/
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
	
	
	void clear_receive(void)  // 变量清零， 20S时间达到 以及 正确接收到信息后
{
		correct_cnt = 0;
		addata_cnt  = 0;
		index_100 = 0;
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); // 关闭ADC
} 

 
uint8_t receive_deal_9K(void)   // 识别9K 指令应答
{
		static uint16_t 	enter_cnt = 0;
//		static uint8_t  index_100 = 0;
//	static uint16_t  correct_cnt = 0;
	
	if(enter_cnt == 0)
	{		
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);  // 打开ADC接收
		enter_cnt = 1;
	}

		ack_9K_temp  = ad7767_data_16B;
		ack_9K_temp *= ack_9K_tlabe[addata_cnt];   // 乘积
		ack_9K_sum  += ack_9K_temp;   // 累加

		
	/* 计算 100次 */  
	if (addata_cnt > 100){  		
			addata_cnt = 0;
			Receive_9K[index_100] = ack_9K_sum;
			ack_9K_sum = 0;
			if(Receive_9K[index_100] > threshold_ack)
				correct_cnt++;
			
			index_100++;
			if(index_100 > 100)
				index_100 = 0;
		}
	

//	{
//			index_100 = 0;
////			for(int i = 0 ; i<50 ;i++)
////				final_temp += Receive_9K[i];
////			final_temp /= 100;
////			printf("平均大小 = %llu",final_temp);
//	}
	
	if(correct_cnt > 50)      // 正确次数需要根据 和水下单元一起测试修改
	{ 
		correct_cnt = 0;
		addata_cnt  = 0;
		enter_cnt = 0;
		index_100 = 0;
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn); // 关闭ADC
		return 1;
	}
	else 
		return 0;
	
}

//	volatile uint16_t 	enter_cnt1 = 0;   // 测试用  总共满足触发阈值的次数
uint8_t receive_deal_9_5K(void)    // 识别9.5K 功能应答
{
	static uint16_t 	enter_cnt = 0;

//	static uint16_t   correct_cnt = 0;
	
	if(enter_cnt == 0)
	{		
		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);   // 打开ADC接收
		enter_cnt = 1;
	}

		ack_9_5K_temp  = ad7767_data_16B;
		ack_9_5K_temp *= ack_9_5K_tlabe[addata_cnt];     // 乘积
		ack_9_5K_sum  += ack_9_5K_temp;   						   // 累加

		
	/* 计算 100次 */  
	if (addata_cnt > 100){  		
			addata_cnt = 0;
//		  enter_cnt1++;
			Receive_9_5K[index_100] = ack_9_5K_sum;
			ack_9_5K_sum = 0;
			if(Receive_9_5K[index_100] > threshold_ack)
			{	
				correct_cnt++;
	
			}
			index_100++;
			if(index_100 > 100)
				index_100 = 0;
		}
	
	
	if(correct_cnt > 6)    // 正确次数需要根据 和水下单元一起测试修改
	{
		clear_receive();
		enter_cnt = 0;

//		printf("正确");
//		TEST_20ms_Flag = 1; // 跳出循环

		return 1;
	}
	else 
		return 0;
	
}






///*-----------------------------------------------------------   命令UI 应用层  ------------------------------------*/
bool order_data[16] = {0};

bool*  order_convert(uint8_t CMD_data)   // 数组转8位
{
	  uint8_t i = 0;
//	ID_CMD ID_CMD1;
	uint8_t ID_data  = 0x5A;    //之后放外面
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
//	printf(" 合成数据为数组\r\n");
//	for(i=0;i<16;i++)
//		printf(" %d \n",order_data[i]);
	return order_data;
}

void append_xor_checksum(uint8_t ihex[10]) {
    uint8_t checksum = ihex[0];
    for (int i = 1; i < 9; i++) {
        checksum ^= ihex[i];
    }
    ihex[9] = checksum;
}






// 时间  逻辑 都没测
void CMD_55(void)   /* 释放指令 */
{
	//构造指令通信帧
	uint8_t ihex[10] = {DEVICE_ID,0x55,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
	append_xor_checksum(ihex);
	
	//lcd_DisStr(4,0,"释放：");
	printf("释放55\r\n");
	lcd_DisStr(4,0,"释放55：");  //验证命令

//	Deck_Send_frame(order_convert(0x55));
	Send_frame_from_hex(ihex);

	/* 开始计时20S*/
		adc7767_init();  //  ADC初始化 开始接收
	time_mode = 2000;
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
	HAL_TIM_Base_Start_IT (&htim13 );                    
	
	lcd_clear_row(4,3);
	lcd_DisStr(4,3,"等待应答");
	printf("等待应答\r\n");


	
	while(TIM_20S_FLAG == 0)  
	{ 
//  		judge_receive(9);   // 判别指令应答 
			
			// lcd_clear_row(4,3);
			// lcd_DisStr(4,3,"等待应答");
			// printf("等待应答\r\n");
			if( 1 == receive_deal_9_5K() )                         // if(order_respond == 1)
			{
				  stop_cnt_flag = 1;  // 停止计时

//					order_respond = 0;  /* 是否在这清零未知 */
					lcd_clear_row(2,3);
					lcd_DisStr(2,3,RESPONSE_TEMP[1]);   
					printf("收到应答\r\n");
				
					/* 转换成10S计时 */
					time_mode = 1000;
					stop_cnt_flag = 0;   // 再次计时
					while(TIM_10S_FLAG == 0)
					{
						if( 1 == receive_deal_9_5K() )               // if(fun_respond == 1)
						{
							stop_cnt_flag = 1;  // 停止计时
//							fun_respond = 0;  /* 是否在这清零未知 */
							lcd_clear_row(4,3);
			        lcd_DisStr(4,3,"释放成功");
					    printf("释放成功\r\n");
							time_mode = 0;
							stop_cnt_flag = 0; 
							return;
						}
					}
					if(TIM_10S_FLAG == 1 ) //&& fun_respond == 0 )
					{
					    TIM_10S_FLAG = 0;
							time_mode = 0;
							stop_cnt_flag = 0;  
							lcd_clear_row(4,3);
			        lcd_DisStr(4,3,"释放失败");
							clear_receive(); 
							HAL_TIM_Base_Stop_IT (&htim13 ); 
					    printf("55释放失败\r\n");
							return;
					}
			}
	}
	if(TIM_20S_FLAG == 1 )//&& order_respond == 0 )
	{
		  TIM_20S_FLAG = 0;
			time_mode = 0;
			stop_cnt_flag = 0; 
			lcd_clear_row(4,3);
	    lcd_DisStr(4,3,"通信失败");
			clear_receive();
			HAL_TIM_Base_Stop_IT (&htim13 ); 
	    printf("55通信失败\r\n");
			return;
	}

}



void CMD_49(void)  /* 测距命令 */
{
	//构造指令通信帧
	uint8_t ihex[10] = {DEVICE_ID,0x49,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
	append_xor_checksum(ihex);
	
	float distance_temp = 0;
	char  distance[4]={0};

		//lcd_DisStr(4,0,"释放：");
	printf("测距49\r\n");
	lcd_DisStr(4,0,"测距49：");  //验证命令

//	Deck_Send_frame(order_convert(0x49));
	Send_frame_from_hex(ihex);

	/* 开始计时20S*/
		adc7767_init();  //  ADC初始化 开始接收
	time_mode = 20;
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
	HAL_TIM_Base_Start_IT (&htim13 );
	lcd_clear_row(4,3);
	lcd_DisStr(4,3,"等待应答");
	printf("等待应答\r\n");
	
	while(TIM_20S_FLAG == 0)  
	{
			// lcd_clear_row(4,3);
			// lcd_DisStr(4,3,"等待应答");
			// printf("等待应答\r\n");
		if(1 == receive_deal_9K())  //	if(order_respond == 1)
			{
					recorded_time =  (float)(TIM13_1s_cnt*100 + TIM13_100ms_cnt*10 + TIM13_10ms_cnt) / 100;  // 单位 S
				  stop_cnt_flag = 1;  // 停止计时
				
//					order_respond = 0;  /* 是否在这清零未知 */
		
					lcd_clear_row(4,3);
					lcd_DisStr(4,3,"收到应答");
					printf("49收到应答\r\n");
			
					printf("测距时间 = %0.1f S\r\n",recorded_time );

					/* 得到距离 */
						distance_temp = recorded_time *1500 /2;      //  信号来回 故除以2
					 itoa( (int)(distance_temp) , distance, 10); 
					 lcd_clear_row(3,3);
					 lcd_DisStr(3,3,distance);
					 clear_receive();  
					 time_mode = 0;
					 stop_cnt_flag = 0;
					 HAL_TIM_Base_Stop_IT (&htim13 );
					 recorded_time = 0;				
					 printf("测距距离 = %s \r\n",distance );
					 return;
			}
	}
	if(TIM_20S_FLAG == 1) //&& order_respond == 0 )
	{
		  TIM_20S_FLAG = 0;
			recorded_time = 0;
			time_mode = 0;
			stop_cnt_flag = 0;  
			clear_receive(); 
			HAL_TIM_Base_Stop_IT (&htim13 );
			lcd_clear_row(4,3);
	    lcd_DisStr(4,3,"通信失败");
	    printf("49通信失败\r\n");
			return;
	}
}
 
void CMD_48(void)  /* 查询电池电压 */
{
	//构造指令通信帧
	uint8_t ihex[10] = {DEVICE_ID,0x48,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
	append_xor_checksum(ihex);
	
	float voltage_temp = 0;
	char  Battery_voltage[4]={0};

			//lcd_DisStr(4,0,"释放：");
	printf("电压48\r\n");
	lcd_DisStr(4,0,"电压48：");  //验证命令

	Deck_Send_frame(order_convert(0x48));

	/* 开始计时20S*/
		adc7767_init();  //  ADC初始化 开始接收
	time_mode = 20;
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
	HAL_TIM_Base_Start_IT (&htim13 );                    
	
	lcd_clear_row(4,3);
	lcd_DisStr(4,3,"等待应答");
	printf("等待应答\r\n");
	
	while(TIM_20S_FLAG == 0)  
	{

			if(1 == receive_deal_9K() )//			if(order_respond == 1)
			{
//				  order_respond = 0;  /* 是否在这清零未知 */
				  stop_cnt_flag = 1;  // 停止计时
					lcd_clear_row(2,3);
					lcd_DisStr(2,3,RESPONSE_TEMP[1]);   
					printf("48收到应答\r\n");
				
					time_mode = 55;
					stop_cnt_flag = 0;   // 转换模式再次计时
				
					/* 等待功能应答 */
					//while(fun_respond == 0);
				  //					fun_respond = 0;  /* 是否在这清零未知 */
					while(0 == receive_deal_9_5K());
					recorded_time =  (float)(TIM13_1s_cnt*100 + TIM13_100ms_cnt*10 + TIM13_10ms_cnt) / 100;  // 单位 S
				  stop_cnt_flag = 1;  // 停止计时
					lcd_clear_row(4,3);
					lcd_DisStr(4,3,"功能应答");
					printf("48功能应答\r\n");
				
					printf("电压时间 = %0.1f S\r\n",recorded_time );  // 正常范围 0-3.3S
					if(recorded_time > 3.3)
					{
							printf("电压超出范围\r\n");	
					}
					/* 转换成水下单元的电池电压表示  /1V */
					 voltage_temp = recorded_time*2.72; // 转成电压值 
					 itoa( (int)(voltage_temp) , Battery_voltage, 10); 
					 lcd_clear_row(3,3);
					 lcd_DisStr(3,3,Battery_voltage);
					 time_mode = 0;
					 stop_cnt_flag = 0;
					 clear_receive(); 
					 HAL_TIM_Base_Stop_IT (&htim13 ); 
					 printf("电池电压 = %s \r\n",Battery_voltage );
					 return;
			}
	}
	if(TIM_20S_FLAG == 1 )//&& order_respond == 0 )
	{
		  TIM_20S_FLAG = 0;
		  recorded_time = 0;
			time_mode = 0;
		  stop_cnt_flag = 0;  
			lcd_clear_row(4,3);
	    lcd_DisStr(4,3,"通信失败");
			clear_receive(); 
			HAL_TIM_Base_Stop_IT (&htim13 ); 
	    printf("通信失败\r\n");
			return;
	}
}

void CMD_47(void)   /* 查询姿态 */
{
	//构造指令通信帧
	uint8_t ihex[10] = {DEVICE_ID,0x47,0x11,0x11,0x11,0x11,0x11,0x11,0x11};
	append_xor_checksum(ihex);
	
	float posture_temp = 0;
	char  posture[4]={0};

				//lcd_DisStr(4,0,"释放：");
	printf("姿态47\r\n");
	lcd_DisStr(4,0,"姿态47：");  

	Deck_Send_frame(order_convert(0x47));

	/* 开始计时20S*/
	adc7767_init();  //  ADC初始化 开始接收
	time_mode = 20;
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        
	HAL_TIM_Base_Start_IT (&htim13 );                    
	
	lcd_clear_row(4,3);
	lcd_DisStr(4,3,"等待应答");
	printf("等待应答\r\n");
	
	while(TIM_20S_FLAG == 0)  
	{
			// lcd_clear_row(4,3);
			// lcd_DisStr(4,3,"等待应答");
			// printf("等待应答\r\n");
			if(1 == receive_deal_9K())  //if(order_respond == 1)
			{
//				  order_respond = 0;  /* 是否在这清零未知 */
					stop_cnt_flag = 1;  // 停止计时
					lcd_clear_row(2,3);
					lcd_DisStr(2,3,RESPONSE_TEMP[1]);   
					printf("47收到应答\r\n");
				
					time_mode = 55;
					stop_cnt_flag = 0;   // 转换模式再次计时
					
					/* 等待功能应答 */
//					while(fun_respond == 0);
//					fun_respond = 0;  /* 是否在这清零未知 */
					while(0 == receive_deal_9_5K());
					recorded_time =  (float)(TIM13_1s_cnt*100 + TIM13_100ms_cnt*10 + TIM13_10ms_cnt) / 100;  // 单位 S
				  stop_cnt_flag = 1;  // 停止计时
					
					lcd_clear_row(4,3);
					lcd_DisStr(4,3,"功能应答");
					printf("47功能应答\r\n");
				
					printf("姿态时间 = %0.1f S\r\n",recorded_time );  //  正常范围 0-9S
					if(recorded_time > 9)
					{
						printf("姿态超出范围\r\n");	
					}
								
					posture_temp = recorded_time/0.05; // 转成角度
					itoa( (int)(posture_temp) , posture, 10); 
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
	    lcd_DisStr(4,3,"通信失败");
			clear_receive(); 
			HAL_TIM_Base_Stop_IT (&htim13 ); 
	    printf("通信失败\r\n");
			return;	
	}
}



void switch_cmd_do(void)  /* 命令选取 */
{
	if(CMD.date_value[2]==5 && CMD.date_value[3]== 5)  // 按键输入55
			CMD_55();    

	else if(CMD.date_value[2]==4){
		switch (CMD.date_value[3]){
			case 7:CMD_47();break;    // 输入47
			case 8:CMD_48();break;  	// 输入48
			case 9:CMD_49();break;    // 输入49
			default:printf("unknow cmd \r\n");break;
		}
	 }
}











