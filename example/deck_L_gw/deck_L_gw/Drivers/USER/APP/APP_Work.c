#include "APP_Work.h"




char Project_name[30] = {"deck I3 3.0\r\n"};


//键控CMD相关
KEY exit_key;
Menu menu ;
Date ID;
Date CMD;

extern uint8_t StartT;


// 频率变量
volatile uint8_t  TIM13_10ms_Flag = 0;
float   recorded_time = 0;

volatile bool  TIM17_40ms_FLAG = 0; 


/*  数值 转换成 字符串显示
// num    输入数(可输入负数)
// str		目标字符
// radix  进制            */

char* itoa(int num,char* str,int radix)
{
    char index[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    unsigned unum;
    int i = 0,j,k;
 
    //获取要转换的整数的绝对值
    if(radix == 10 && num < 0)
    {
        unum = (unsigned) - num;
        str[i++] = '-';
    }
    else unum = (unsigned)num;//若是num为正，直接赋值给unum
 
    //转换部分，逆序
    do
    {
        str[i++]=index[unum%(unsigned)radix];//取unum的最后一位，并设置为str对应位，指示索引加1
        unum/=radix;
 
    }while(unum);
 
    str[i]='\0';// '\0'字符
 
    //顺序调整
    if(str[0]=='-') k=1;
    else k=0;
 
    char temp;
    for(j=k;j<=(i-1)/2;j++)//头尾一一对称交换，i其实就是字符串的长度，索引最大值比长度少1
    {
        temp=str[j];
        str[j]=str[i-1+k-j];
        str[i-1+k-j]=temp;
    }
	
	
    return str;//返回转换后的字符串
 
}



/*---------------------------------------------------------  外设基本功能   ----------------------------------------- */

	

float Read_battery(uint8_t	vref)  // 电池电压读取 最大3V
{
	static uint32_t  ADC_Internal_Data = 0;     // 内部ADC值  16位
	static float     Battery_Voltage   = 0;     // 电池电压
	
	ADC_Internal_Data = HAL_ADC_GetValue(&hadc3);
	Battery_Voltage = (ADC_Internal_Data*3.3)/65535;  
	
	if(vref == 3)
		return Battery_Voltage;
	else if(vref == 9)
		return Battery_Voltage*2.72;  // 2.72= 9/3.3  ，转换成电池电压
	else 
		return 0;
}




void APP_Init(void)
{
	
//  printf(Project_name);
	Reset_Pin(POWER_CAP);  /* 关闭48V电源 */

 /* LCD */
	InitDis();
  draw_main_page();
	
	/* RS232 */
//	 RS232_Init();
  
	/* 电池电压 */
	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3, 0xff);
	printf("电池电压 = %0.3fV\r\n",Read_battery(9));  	 // 以9V输出电压	

	
	/* 按键 */
		__HAL_TIM_CLEAR_IT (&htim7 ,TIM_IT_UPDATE );        // 清除T7标志
	HAL_TIM_Base_Start_IT (&htim7 );                    // 允许T7中断
	
	/* RTC */
	RTC_Init();
	
// 如果需要重新写入使能下面语句，提前编译完成，大约提前5s下载
//	RTC_Write();    // 写入时间   
//	printf("\r\n RTC写入时间 \r\n");
//	RTC_Read();			// 读取写入后的时间


	/* SD卡测试 */
//	Get_SD_information();
//	Check_SD_Fatfs();
//	Fatfs_SD_Init();
	
	/* ADC */
//	PREAMP_2;        // ADC 前放倍数选择


/* 发射测试 */
	Set_Pin(LED1); // 绿灯
  CMD_55();
//  HAL_Delay(10);
//	CMD_49();
//	HAL_Delay(10);
//	CMD_48();
//	HAL_Delay(10);
//	CMD_47();
	
//receive_deal_9K();

/*ADC*/
//adc7767_init();
//HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);  // 打开ADC7767 触发



//测试应答
// Deck_Send_frame(order_convert(0x55));

// Deck_Send_frame(order_convert(0x48));

/* 测试ADC中断触发频率 : 正常 16us进一次*/
#if 0
	adc7767_init();
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        // 清除T7标志
	HAL_TIM_Base_Start_IT (&htim13 );                    // 允许T7中断
	addata_cnt = 0;
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	
	while(TIM13_10ms_Flag == 0) {};

		printf("ADC 10ms 中断次数 = %d" , addata_cnt);
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		HAL_TIM_Base_Stop_IT (&htim13 );  
	#endif
	
		/* 测试接收信号的辨析度 20ms内*/
#if 0
	__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        // 清除T13标志
	HAL_TIM_Base_Start_IT (&htim13 );                    // 允许T13中断
	correct_cnt = 0;
	
	while(TEST_20ms_Flag == 0) {receive_deal_9_5K();};
		TEST_20ms_Flag = 0;
		printf("20ms 正确次数 = %d\r\n" , correct_cnt);
	  printf("20ms 判断次数 = %d" , enter_cnt1); 
		HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);
		HAL_TIM_Base_Stop_IT (&htim13 );  
	#endif
}


////16进制转float
//float hex_bytes_to_float(uint8_t hex[4]) {
//    uint32_t temp = ((uint32_t)hex[0] << 24) |
//                    ((uint32_t)hex[1] << 16) |
//                    ((uint32_t)hex[2] << 8) |
//                    (uint32_t)hex[3];
//    float value;
//    memcpy(&value, &temp, sizeof(float));
//    return value;
//}















void APP_Process (void)
{
//	printf("检测变量及数组清空成功\r\n");
//  get_adc();   // 单独测试ADC用
	Matrix_Key_funtcion(Matrix_Key_Scan());
	HOME_Scan();
	if(StartT == 1)
		{
			StartT = 0;
			printf("收到了\r\n");
//			hex_bytes_to_float(hex);
		}
	
}

	volatile uint8_t TIM13_10ms_cnt = 0;
  volatile uint8_t TIM13_100ms_cnt = 0;
  volatile uint8_t TIM13_1s_cnt = 0;

// 定时器溢出中断回调
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */

//	static uint8_t TIM13_10ms_cnt = 0;
//  static uint8_t TIM13_100ms_cnt = 0;
//  static uint8_t TIM13_1s_cnt = 0;
  static uint16_t TIM17_16us_cnt = 0;

  if(htim->Instance == TIM7){        /*  T7  1ms */
			Key_Scan_flag = 1;  // 开启HOME键扫描
  
	if(da_index>0)
		{
//			TIM7_cnt++;
//			if(TIM7_cnt == 3)
//			{
//				TIM7_cnt = 0;
//				da_index = 0;
//				memset(da, 0, sizeof(da));  // 清空数组
//				memset(hex, 0, sizeof(hex));  // 清空数组
//				printf("\r\n 检测变量及数组清空成功\r\n");
//			}
		}
	}
	
	  if(htim->Instance == TIM13)    /*  T13  10ms  */
  {
		TIM13_10ms_Flag = 1;
    if( (time_mode == 10 || time_mode == 20 || time_mode == 55) && stop_cnt_flag == 0)
    {
        TIM13_10ms_cnt++;
        if(TIM13_10ms_cnt>=10)
        {
          TIM13_100ms_cnt++;
          TIM13_10ms_cnt = 0;
        }
        if(TIM13_100ms_cnt>=10)
        {
          TIM13_1s_cnt++;
          TIM13_100ms_cnt = 0;
        }
        if(TIM13_1s_cnt>= 10 && time_mode ==10)  // 10S时间到达
         {
          TIM_10S_FLAG = 1;
          stop_cnt_flag = 1;
         }
        else if(TIM13_1s_cnt>=20 && time_mode == 20)  // 20S时间到达
         {
          TIM_20S_FLAG = 1;
          stop_cnt_flag = 1;
         }
    }
    if(stop_cnt_flag == 1)  // 停止计时后清零计数值
    {
//        recorded_time =  (TIM13_1s_cnt*100 + 
//                          TIM13_100ms_cnt*10 + 
//                          TIM13_10ms_cnt) / 100;  // 单位 S
        TIM13_10ms_cnt = 0;
        TIM13_100ms_cnt = 0;
        TIM13_1s_cnt = 0;
    }
  }	
	


		if(htim->Instance == TIM17)    /*  T17  16us  */
		{
			  TIM17_16us_cnt++;
				if(TIM17_16us_cnt > 2500)
				{
					TIM17_40ms_FLAG = 1;
					TIM17_16us_cnt = 0;
					HAL_TIM_Base_Stop(&htim17);  // 关闭定时器
				}	
				else 
				{
								if(s1_data[TIM17_16us_cnt])
									GPIO_HIN->BSRR |= GPIO_BS_HIN; 
								else
									GPIO_HIN->BSRR |= GPIO_BR_HIN;
								
								if(s2_data[TIM17_16us_cnt])
									GPIO_LIN->BSRR |= GPIO_BS_LIN;
								else
									GPIO_LIN->BSRR |= GPIO_BR_LIN;
				}
		}
		
  /* NOTE : This function should not be modified, when the callback is needed,
            the HAL_TIM_PeriodElapsedCallback could be implemented in the user file
   */
}
