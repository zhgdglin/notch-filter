#include "APP_Work.h"

// 设备ID
#define Release_ID  0x5A    // 取值从 0 ~ 255

// 限位开关
//#define  DOUBLE_LIMIT      // 使用两个限位开关
#define  No_limit      0   // 限位未到达
#define  Effect_limit  1   // 限位有效
// 电机
#define MOTOR_REVERSE    Set_Pin(MOTOR)     // 电机反转 
#define MOTOR_FORWARD    Reset_Pin(MOTOR)   // 电机正转


enum 
{
	Stop    = 0,       
	Release = 1,  //钩子释放
	Recover = 2,  //钩子复位
};               // 电机运行状态

typedef struct 
{
	uint16_t prescaler;    // 分频数
	uint16_t period;			 // 计算值 
	uint16_t H_IN;	 			 // 占空比1 =  H_IN / period
	uint16_t L_IN;				 
}TIM_FREQUENCE;          // 定时器的频率参数





// 限位变量
bool LIMIT_Scan_flag  = 0;
bool volatile release_done = 0;   // 释放完成标志
bool recover_done = 0;	 // 复位完成标志

// 休眠变量
uint8_t  volatile  System_State = Normal;
uint32_t volatile  Wakeup_signal_cnt     = 0;
uint32_t volatile  Wakeup_signal_Low_cnt = 0;
//电机变量
bool volatile MOTOR_CAN_MOVE = 0;   // 电机允许转动标志
// 定时变量
bool Minute_5_start = 0;   // 5分钟计时开始标志
bool volatile TIM7_1s_Flag = 0;   

bool NEW_CMD_Flag = 0;  // 指令更新标志

ID_CMD last_message;  // 上一次解调得到的数据

// 频率变量
uint16_t Timer4_Prescaler = 0;
uint16_t Timer4_Period	  = 0;
uint16_t Timer4_oc1_Pulse = 0;
uint16_t Timer4_oc2_Pulse = 0;
// 9.5KHz = 240 000 000/ 2 /9500 = 12631.5取 12632  oc1:12632*0.35 = 4421.2取 4421   oc2:12632- 4421 = 8211

extern bool	volatile decode_finish;

const static TIM_FREQUENCE TIM4_Freq_Data [3]=  
{
		{ 0, 13333-1, 4667-1, 8666-1 },   // 9K    35%
		{ 0, 12632-1, 4421-1, 8211-1 },		// 9.5K  35%
		{ 0, 10000-1, 3500-1, 6500-1 }	  // 12K   35%
};  


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

void Power_ON(void) // 开启板子上的电源控制
{
	Set_Pin(POWER_9V);  		//  9V电源    总控
	Set_Pin(POWER_3V3); 	  //  3.3V电源  RS232，SD卡，MPU6050
	Set_Pin(POWER_CAP);     //  48V电源   电容 发射
} 


void	Power_OFF(void) // 关闭板子上的电源控制
{
	Reset_Pin(POWER_9V);  		//  9V电源    
	Reset_Pin(POWER_3V3); 	  //   RTC ，SD卡，MPU6050     // 硬件上需要下拉电阻
	Reset_Pin(POWER_CAP);     //   电容 发射
} 


/*---------------------------------------------------------  外设基本功能   ----------------------------------------- */
void Enter_Standby_Mode(void)
{
	
	Power_OFF();
	
	printf("\r\n **** 进入待机模式 **** \r\n");
	/* 清除标志位 */
	__HAL_PWR_CLEAR_WAKEUPFLAG(PWR_FLAG_WKUP1);     // PWR_WKUPCR_WKUPC1
	/* 开启唤醒引脚 */
	HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1_LOW);  // PA0 下降沿触发
	/* 进入待机模式 */
	HAL_PWR_EnterSTANDBYMode();
}



void Scan_limit(void) // 限位开关 定时器扫描方式
{
	#ifdef DOUBLE_LIMIT
	static	uint8_t Limit1_value  = 0;
	static	uint8_t Limit2_value  = 0;
	static	uint8_t Limit1_State  = No_limit;
	static	uint8_t Limit2_State  = No_limit;
	
	 /* 检测 */
		if( LIMIT_Scan_flag == 1)  // 0.9ms检测一次
		{
				LIMIT_Scan_flag = 0;
				
				Limit2_value <<= 1;
				Limit2_value |= Read_Pin(LIMIT_2); 
				
				Limit1_value <<= 1;
				Limit1_value |= Read_Pin(LIMIT_1); 
				
		
				if(Limit1_value == 0xF0 )  // 1111 0000   //正在按下
							Limit1_State = Effect_limit;
				else		
							Limit1_State = No_limit;

				
				if(Limit2_value == 0xF0 )  // 1111 0000   //正在按下
							Limit2_State = Effect_limit;
				else 
							Limit2_State = No_limit;
		}
		
			
		/* 执行 */	
		if(Limit1_State == Effect_limit)
				release_done = 1;
	
		if(Limit2_State == Effect_limit)		
				recover_done = 1;		
	#else
	static	uint8_t Limit_value  = 0;
	static	uint8_t Limit_State  = No_limit;
	
	 /* 检测 */
		if( LIMIT_Scan_flag == 1)  // 0.9ms检测一次
		{
				LIMIT_Scan_flag = 0;
				
				Limit_value <<= 1;
				Limit_value |= Read_Pin(LIMIT_2); 
	
				if(Limit_value == 0xF0 )  // 1111 0000   //正在按下
							Limit_State = Effect_limit;
				else 
							Limit_State = No_limit;
		}
		
		/* 执行 */	
		if(Limit_State == Effect_limit)
		{
				release_done = 1;
			  printf("限位到达 \r\n");
		}
		#endif
}	



void Motor_rotate(uint8_t rotation)    // 电机转动  0：停止  1：正转    2：反转
{
	/* 选择转动方向 */
	switch( rotation )
	{
		case 0:   // 电机停止
						MOTOR_CAN_MOVE = 0;   // 定时器中扫描此位 
						printf("电机stop\r\n");
						break;
		
		case 1:  // 电机正转 钩子释放
						MOTOR_CAN_MOVE = 1;
						MOTOR_FORWARD; 	
						printf("电机正转\r\n");
						break;			
		
		case 2:  // 电机反转 钩子复位
						MOTOR_CAN_MOVE = 1;
						MOTOR_REVERSE;
						printf("电机反转\r\n");
						break;
		
		default:break;
	}
	
//	MX_TIM13_Init(); // 电机，中断10
	
}
	

float Read_battery(uint8_t	vref)  // 电池电压读取 最大3V
{
	static uint32_t  ADC_Internal_Data = 0;     // 内部ADC值  16位
	static float     Battery_Voltage   = 0;     // 电池电压
	
	ADC_Internal_Data = HAL_ADC_GetValue(&hadc3);
	Battery_Voltage = (ADC_Internal_Data*3.3)/65535;  
	
	if(vref == 9)
		Battery_Voltage = Battery_Voltage*2.72f;
	
	return Battery_Voltage;

}


/*----------------------------------------------------------  水下单元向甲板单元 发射应答信号  --------------------------------------------------------*/
 

void Timer4_freq_Init(TIM_FREQUENCE freq_data)  /* 改变T4频率 */
{
	Timer4_Prescaler = freq_data.prescaler;
	Timer4_Period	   = freq_data.period;
	Timer4_oc1_Pulse = freq_data.H_IN;
	Timer4_oc2_Pulse = freq_data.L_IN;
	
	MX_TIM4_Init();
}


void Send_response_cmd(void)   /* 指令应答，9K 20ms */
{
	/*改变频率*/
	Timer4_freq_Init(TIM4_Freq_Data[0]);  // 9K
	
	 SysTick->VAL   = 0UL;  // 清除系统定时器的计数
	//	 TIM6->CNT  =  0; 
	/*最后测试发射在开启   fun函数同理*/
//	Reset_Pin(IR2110S_SD);   // 开启输出，低有效   

	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);
	
	HAL_Delay(19);
	//Delay_us(800);
	Set_Pin(IR2110S_SD);   // 20ms，结束输出
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1);  
}


void Send_response_fun(void)   /* 功能应答，9.5K 20ms */
{
	Timer4_freq_Init(TIM4_Freq_Data[1]);  // 9.5K
	
	 SysTick->VAL   = 0UL;  // 清除系统定时器的计数
	//	 TIM6->CNT  =  0; 
//	Reset_Pin(IR2110S_SD);   // 开启输出，低有效

	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Start(&htim4,TIM_CHANNEL_1);
	
	HAL_Delay(19);
	//Delay_us(800);
	Set_Pin(IR2110S_SD);   // 20ms，结束输出
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_3);
	HAL_TIM_OC_Stop(&htim4,TIM_CHANNEL_1);  
}


ID_CMD Demodulation(void)
{
	ID_CMD message;   		//本次解调得到的数据
	
	last_message.CMDdata = message.CMDdata;  // 获取上一次的命令
	
	
	/* 算法解调 返回命令*/
	
	
	// 模拟解调出来的指令
	message.IDdata  = Release_ID; 
	message.CMDdata = 0x55;

	return message;   // 可以存到一个FIFO中
}





void Delay_S(uint16_t cnt)   // 定时器7 延时1s
{
	__HAL_TIM_CLEAR_IT (&htim7 ,TIM_IT_UPDATE );        // 清除T7标志
	__HAL_TIM_SET_COUNTER(&htim7, 0);   								// 清零T7计数
	HAL_TIM_Base_Start_IT (&htim7 );                    // 允许T7中断
	
	while(cnt)
	{	
		while(TIM7_1s_Flag == 0);  // 等待中断发生
		cnt--;
		TIM7_1s_Flag = 0;	// 清除到达标志
	}
	
	HAL_TIM_Base_Stop_IT (&htim7);              //延时完毕后关闭中断      
}	


void Cmd_delay(float time)  // 延时小数的时间 单位S
{
	float decimal = 0;
	uint16_t integer_part = 0;
	uint16_t decimal_part = 0;
	
		// 电压延时范围 0 ~ 3.3 秒
		// 姿态延时范围 0 ~ 9秒
	
	if (time<0 || time>9)   //超出范围不执行 
		return;
	
	// 分别取出整数和小数部分
	integer_part = (uint16_t)time;
	decimal = time - integer_part;
	decimal_part = (uint16_t)(decimal*1000);   // 取小数点后三位，乘1000
	
	printf("Cmd_delay int %d \r\n", integer_part);
	printf("Cmd_delay dec %d \r\n", decimal_part);
	
	Delay_S(integer_part);      // 秒 延时
	HAL_Delay(decimal_part);  // 毫秒 延时
	printf("delay over \r\n");
}


void communication_process(ID_CMD message)    /* 指令通信过程 */   
{
	float temporary = 0;  // 临时变量
	
	// 每次判断FIFO的最后一组(最先进来的)
	
	if( message.IDdata != Release_ID)  // 判断解调出的ID信号是否为本设备ID
	return;
	
	/* 开启发射 */
	Set_Pin(POWER_CAP);
	
	switch(message.CMDdata)   // 根据指令执行对应功能
	{
		case 0x55:  // 释放指令
								Send_response_cmd();  				// 指令应答
								printf("\r\n 释放指令应答 0x55 \r\n ");
								Motor_rotate(Release);     	  // 释放钩子
								while(release_done != 1);     // 等待释放完毕  开关2限位
								release_done = 0;
								Motor_rotate(Stop);		 				// 电机停止
								Send_response_fun();  				// 功能应答
								printf("释放功能应答 \r\n ");
				        Reset_Pin(POWER_CAP);      //关闭发射
								break;
		
		case 0x49:  // 测距指令   （主要在甲板单元上计算）
								Send_response_cmd();  // 指令应答
								printf("\r\n 测距指令应答 0x49 \r\n ");
				        Reset_Pin(POWER_CAP);      //关闭发射
								break;
		
		case 0x48:  // 查询水下单元电池电压指令
								Send_response_cmd();  			  // 指令应答
								printf("\r\n 电池电压指令应答 0x48\r\n  ");
								temporary = 1 * Read_battery(9);				 // 延时时间 = 电压*1 S  , 读取电池电压
								printf("电池电压 = %0.3fV\r\n", Read_battery(9)); 
								Cmd_delay(temporary);				  // 延时
								Send_response_fun();  				// 功能应答
							  printf(" 电池功能应答 \r\n ");
				        Reset_Pin(POWER_CAP);      //关闭发射
								break;
		
		case 0x47:  // 查询姿态指令
								Send_response_cmd();  				// 指令应答
								printf("\r\n 姿态指令应答 0x47 \r\n ");
								temporary = 0.05f * Read_mpu6050() ;   // 延时时间 = 角度*0.05 S,  读取俯仰
								Cmd_delay(temporary); 				// 延时
								Send_response_fun();  				// 功能应答
								printf("姿态功能应答 \r\n ");
		            Reset_Pin(POWER_CAP);      //关闭发射
								break;
		
		default:break;
	}
}




void APP_Init(void)
{
	/* 关闭48V电源 */
	Reset_Pin(POWER_CAP); 
	/* RS232 */
	RS232_Init();
  
	/* 电池电压 */
	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3, 0xff);
	printf("电池电压 = %0.3fV\r\n",Read_battery(9));  	 // 以9V输出电压	
	/* MPU6050 */
	MPU6050_Init();
	
	/* 电机 和 限位开关*/  
		__HAL_TIM_CLEAR_IT (&htim13 ,TIM_IT_UPDATE );        // 清除T13标志
		HAL_TIM_Base_Start_IT (&htim13 );                    // 允许T13中断		
	
	/* RTC */
	RTC_Init();
	
// 如果需要重新写入使能下面语句，提前编译完成，大约提前5s下载
//	RTC_Write();    // 写入时间   
//	printf("\r\n RTC写入时间 \r\n");
//	RTC_Read();			// 读取写入后的时间


	/* SD卡测试 */
	Get_SD_information();
	if(System_State == Normal)  
			Check_SD_Fatfs();    // 电池掉电且SD卡无文件系统会格式化SD卡
	Fatfs_SD_Init();
	
	/* ADC */
	PREAMP_2;        // ADC 前放倍数选择
	adc7767_init();
}


void APP_Process (void)
{
	
//	 frm_sych();
//	Scan_limit();
//	if(1 == release_done )
//		Enter_Standby_Mode();
//	if(decode_finish == 0)
//	{
//		demodu_process();
//	}
//	communication_process(Demodulation());

	
	SD_Write_Process();

}


/*  定时器中断处理 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	static uint16_t TIM13_cnt = 0;
	static uint16_t TIM2_Minute_5_cnt = 0;
	static uint8_t data_temp = 0;
  /* USER CODE END Callback 0 */

  /* USER CODE BEGIN Callback 1 */

  if ( htim->Instance == TIM13) {  // TIM13  30KHZ时基
		TIM13_cnt++;
		
		if( TIM13_cnt >= 3)    // 扫描限位开关的周期 0.9ms
		 {
			TIM13_cnt = 0;
			LIMIT_Scan_flag = 1;
		  Scan_limit();
		 }
		
		if(MOTOR_CAN_MOVE == 1)
		Toggle_Pin(POWER_MOTOR);  //  翻转控制电机电平  频率：15KHz 方波	 
	}
	
	
	 if ( htim->Instance == TIM7) {  // TIM7 1S时基
		TIM7_1s_Flag = 1;  //			
	 }
	 
	 
	 if(htim->Instance == TIM2) {   // TIM2  2.5S时基

			/* 休眠判断   单测时间没问题   逻辑需要和甲板单元一起测试 */
	 		if(Minute_5_start == 1) // 开始计时5分钟
			{
			 TIM2_Minute_5_cnt++;
//			 data_temp = last_message.CMDdata;
//				
//				/* 下面判断指令的程序 需要再改进 */
//			 if(data_temp != Demodulation().CMDdata)     // 5分钟内 没有新的指令
//				 NEW_CMD_Flag = 1;
			 
			 
			 if(TIM2_Minute_5_cnt >= 120)  //120 //5分钟时间到达  5*60 = 300 S ，   300/ 2.5 = 120
				{
						TIM2_Minute_5_cnt = 0;
						Minute_5_start = 0;
							printf("\r\n 无操作时间 5分钟 \r\n ");
//				    if(NEW_CMD_Flag == 0) {
							Enter_Standby_Mode();    // 5分钟内无新指令，再次休眠
//						}     
						
				}
			}
	}
	
	 // 开始计时5分钟，5分钟之内每次中断都判断是否有新指令，有新指令则置位标志，5分钟之后判断置位标志，决策是否休眠
  /* USER CODE END Callback 1 */
}
