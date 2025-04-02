/* 
		ADC介绍：
		
		AD7767 - 1      64K采样率（系数16）
		
		MCLK 由 定时器PWM 产生  1MHZ ，对应采样率62.5K
		SYNC置高后的强制延时时间  tsetling = 	(1186 × tMCLK) + t21  = （1186 x 1us）+ 510ns = 1186us + 510ns = 1.186ms     */



#include "ad7767.h"   // 包含前放增益


__align(4)  float  AD7767_Ping[BUFF_SIZE] = {0};  
__align(4)  float  AD7767_Pang[BUFF_SIZE] = {0};
float  *p_ad_begin = NULL;  // 指示现在写入的是哪个数组  用于判断
float  *p_sd = NULL;        // 实际SD卡写入的数组
float  volatile  ad7767_data = 0;

int32_t    addata_int_temp = 0;

bool	volatile Ping_full_flag = 0;
bool	volatile Pang_full_flag = 0;
	
uint8_t SPI_Rx_buff[3] = {0,0,0};
uint8_t SPI_Tx_buff[3] = {0xA0,0xA0,0xA0};

uint16_t volatile  addata_cnt = 0;  		// 用于数组元素个数计数




 

 // 有符号 24位转32位
int32_t S24toS32(int32_t input)  
{
	if((input&0x800000)==0x800000)		//如果最高位为1，则是负数
	{
		input |= 0xff000000;		//高位补1
	}
	return input;
}




void adc7767_init(void)
{
		HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_1);   //  MCLK  =  1MHZ
	
	
		Set_Pin(ADC_SYNC);
		Reset_Pin(ADC_SYNC);   // SYNC= 0时 DRDY一直保持高电平
		Delay_us(3);//延时3us
		Set_Pin(ADC_SYNC);

		HAL_Delay(2);

		p_ad_begin = AD7767_Ping;           // 指针赋值
		p_sd = AD7767_Ping;

		HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);  // 打开ADC7767 触发
}



void change_buff(void)
{
	if(addata_cnt >= BUFF_SIZE)
	{
			addata_cnt = 0;
		
			if(p_ad_begin == AD7767_Ping)
			{
					Ping_full_flag = 1;
					p_sd  		 = AD7767_Pang;   // SD卡写入数组移动到Pang   // 真正作用的数组
					p_ad_begin = AD7767_Pang;   														// 指示作用	
			}	
			else if(p_ad_begin == AD7767_Pang)
			{
					Pang_full_flag = 1;
					p_sd       = AD7767_Ping;
					p_ad_begin = AD7767_Ping; 
			}	
	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
		
   if(GPIO_Pin == ADC_DRDY_Pin) // ADC数据转换完成信号  下降沿触发
		{
			
			__nop();__nop();__nop();__nop();__nop();
			__nop();__nop();__nop();__nop();__nop();  
			
			if(Read_Pin(ADC_DRDY) == 1)
					return;   // 将DRDY的尖刺干扰滤除
			else 
			{
					/* 时序开始 */
					Set_Pin(ADC_CS);    
					Delay_us(3);//延时3us
					Reset_Pin(ADC_CS);
					HAL_SPI_Receive(&hspi1, SPI_Rx_buff , 3,2);  
					Set_Pin(ADC_CS);
					
					/*时序结束，处理数据*/
//					ad7767_data = (SPI_Rx_buff[0]<<16) | (SPI_Rx_buff[1]<<8) |(SPI_Rx_buff[2]);  // 高位在前
//					ad7767_data &= 0x00FFFFFF;   // 确保是24位数据
//					ad7767_data = S24toS32(ad7767_data);		//  将24位转成32位
						
					addata_int_temp = (SPI_Rx_buff[0]<<16) | (SPI_Rx_buff[1]<<8) |(SPI_Rx_buff[2]);  // 高位在前
					addata_int_temp &= 0x00FFFFFF;   // 确保是24位数据
					addata_int_temp = S24toS32(addata_int_temp);		//  将24位转成32位
					ad7767_data =  ((float)(addata_int_temp))/8388608 *5;   // 16777216   8388608
				
				
					/*数据缓存*/	
					 addata_cnt++;
					
					*p_sd = ad7767_data;  // 获得AD数据
					 p_sd++; 							// 移动指针至下一个元素
				   change_buff();
							 
				}
		}
	
}


