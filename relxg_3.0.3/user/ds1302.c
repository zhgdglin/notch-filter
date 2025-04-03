#include "ds1302.h"
//#include "gpio.h"
//#include "usart.h"  // 打印
//#include "tim.h"    // 延时
#include <stdlib.h>
#include <string.h>  //


// PB8 ---- SCLK 
// PB9 ---- SDA  低位在前
// PE0 ---- CE   高电平有效

enum   // RTC 数组索引 
{
	year = 0,
	month,
	date,
	hour,
	minute,
	second,
};

__align(4) int32_t RTC_Time[6] = {0}; // 按照 年, 月, 日, 小时, 分钟, 秒 
// char RTC_Time[6] = {0}; // 按照 年, 月, 日, 小时, 分钟, 秒 



void Set_SDIO_IN(void) // 设置IO 引脚为 输入
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  GPIO_InitStruct.Pin = RTC_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT; 
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}


void Set_SDIO_OUT(void) // 设置IO 引脚为 输出
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
  GPIO_InitStruct.Pin = RTC_IO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; 
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}




/*-------------------------  传输层------------------------------*/

// 写入：低位在前，上升沿有效
void ds1302_write(uint8_t byte) 
{
    static uint8_t i;
	
    for(i = 0; i < 8; i++)  
	  {  
			Reset_Pin(RTC_SCLK);  
      
   		if((byte & 0x01) == 1)    // 判断最低位   
					Set_Pin(RTC_IO);
   		else  
					Reset_Pin(RTC_IO);
			
			Delay_us(1);	

		  Set_Pin(RTC_SCLK); 
		  Delay_us(1);
   		byte = byte >> 1;  // 右移
	  } 
}


// 读取：低位在前 下降沿有效
uint8_t ds1302_read(void) 
{
	  static uint8_t i = 0;
	  static uint8_t byte = 0;
	
		Set_Pin(RTC_SCLK); 
    for(i = 0; i < 8; i++)  
	  {  
      Reset_Pin(RTC_SCLK);   // CLK下降沿读取
			Delay_us(1);
			
			byte = byte >> 1;  // 右移 
			if( Read_Pin(RTC_IO) == 1)
				byte |= 0x80;                // 读取完成后高位在前
			
   	   Set_Pin(RTC_SCLK);  
			 Delay_us(1);
	  }
		
		return byte;
}	


/*-------------------------  协议层------------------------------*/

void DS1302_Write(uint8_t address,uint8_t data)
{
	
	Reset_Pin(RTC_CE); 
	Reset_Pin(RTC_SCLK); 
	HAL_Delay(1);
	
	Set_Pin(RTC_CE);       // 开启片选
	__nop();
	Set_SDIO_OUT();
	
	address &= 0xFE;        /* 宏定义的是读取地址，改成写入地址 */
	ds1302_write(address); // 第一个字节是地址
	ds1302_write(data);   
	
	Delay_us(10);
	Reset_Pin(RTC_CE);
	Reset_Pin(RTC_SCLK); 
}



uint8_t DS1302_Read(uint8_t address)
{
	static uint8_t  data = 0;
	
	Reset_Pin(RTC_CE); 
	Reset_Pin(RTC_SCLK); 
	HAL_Delay(1);
	
	Set_Pin(RTC_CE);       // 开启片选
	__nop();
	
	Set_SDIO_OUT();				 // 引脚设置为输出
	ds1302_write(address); // 地址
	
	Set_SDIO_IN();				 // 引脚设置为输入
	data = ds1302_read();    
	
	Reset_Pin(RTC_CE);
	Reset_Pin(RTC_SCLK); 
  return  data;
}



/*-------------------------  应用层------------------------------*/

// 写入时间  用16进制写入，如14：25  , 写入小时0x14 分钟0x25
void RTC_Write(void)
{
		DS1302_Write(ADDR_1302_Write_Lock, 0x00); // 关闭写保护, 允许写入
	
   	DS1302_Write(ADDR_1302_Second, 0x00);  // 秒
	  DS1302_Write(ADDR_1302_Minute, 0x20);	 // 分
		DS1302_Write(ADDR_1302_Hour,   0x9);  // 小时
		DS1302_Write(ADDR_1302_Date,   0x11);  // 日
		DS1302_Write(ADDR_1302_Month,  0x05);  // 月
		DS1302_Write(ADDR_1302_Year,   0x24);	 // 年
	
		DS1302_Write(ADDR_1302_Write_Lock, 0x80); // 开启写保护，禁止写入
}




#if 0
// 用于100以内或者位数为2 的  16进制数转10进制数
// 最大输入 0x99
// 效果：输入 0x23（16进制）   输出 23（10进制）
int32_t hex_to_dec(int32_t data)
{
	uint16_t data_ones;  // 个位
	uint16_t data_tens;	 // 十位.
	
	data_tens = data /16;
	data_ones = data %16;
	
	return data_tens*10 + data_ones;
}


// 读取时间  转10进制输出
void RTC_Read(void)
{
	
	RTC_Time[year]    =  DS1302_Read(ADDR_1302_Year);	    // 年
	RTC_Time[month]   =  DS1302_Read(ADDR_1302_Month);    // 月
	RTC_Time[date]    =  DS1302_Read(ADDR_1302_Date);     // 日
	RTC_Time[hour]    =  DS1302_Read(ADDR_1302_Hour);     // 小时
	RTC_Time[minute]  =  DS1302_Read(ADDR_1302_Minute);	  // 分
	RTC_Time[second]  =  DS1302_Read(ADDR_1302_Second);   // 秒
	
  
	
//	printf("20%x年%x月%x日 ,%x:%x:%x \r\n",    (RTC_Time[year])\
//																						,(RTC_Time[month])\
//																						,(RTC_Time[date])\
//																						,(RTC_Time[hour])\
//																						,(RTC_Time[minute])\
//																						,(RTC_Time[second])  	);			
//	RTC_Time[year]    |= 0xFFFFFF00; 
//	RTC_Time[month]   |= 0xFFFFFF00; 
//	RTC_Time[date]    |= 0xFFFFFF00; 
//	RTC_Time[hour]    |= 0xFFFFFF00; 
//	RTC_Time[minute]  |= 0xFFFFFF00; 
//	RTC_Time[second]  |= 0xFFFFFF00; 	
//	
//	printf("%x.%x.%x ,%x.%x.%x \r\n",    (RTC_Time[year])\
//																						,(RTC_Time[month])\
//																						,(RTC_Time[date])\
//																						,(RTC_Time[hour])\
//																						,(RTC_Time[minute])\
//																						,(RTC_Time[second])  	);		

RTC_Time[year] = hex_to_dec(RTC_Time[year]);
RTC_Time[month] = hex_to_dec(RTC_Time[month]);
RTC_Time[date] = hex_to_dec(RTC_Time[date]);
RTC_Time[hour] = hex_to_dec(RTC_Time[hour]);
RTC_Time[minute] = hex_to_dec(RTC_Time[minute]);
RTC_Time[second] = hex_to_dec(RTC_Time[second]);

	printf("%d.%d.%d ,%d.%d.%d \r\n",    (RTC_Time[year])\
																						,(RTC_Time[month])\
																						,(RTC_Time[date])\
																						,(RTC_Time[hour])\
																						,(RTC_Time[minute])\
																						,(RTC_Time[second])  	);

}
#endif

//void RTC_Set_24Hour(void)  // 没有验证过
//{
//	 uint8_t hourset = 0;
//	 hourset = DS1302_Read(ADDR_1302_Hour);
//	 if( (hourset & 0x80) == 0)  // 判断是否为24小时制
//		 return;
//	 
//	 DS1302_Write(ADDR_1302_Hour, hourset & 0x7F);   // 设置为24小时
//}




// 读取RTC时间  
char* RTC_Read(void)  // RTC_Read
{
	uint8_t i = 0;
	
	char *p_str_0 = NULL;
	char *p_rtc_time = NULL;
	char RTC_Time_Char[6][2];
	char rtc_time_str[50] = "20";   // 2024年的 20
	char str_0[2] = "0";
	
	p_rtc_time = rtc_time_str;
	p_str_0 = "0";
	
	/* 读取寄存器 */
	RTC_Time[year]    =  DS1302_Read(ADDR_1302_Year);	    // 年
	RTC_Time[month]   =  DS1302_Read(ADDR_1302_Month);    // 月
	RTC_Time[date]    =  DS1302_Read(ADDR_1302_Date);     // 日
	RTC_Time[hour]    =  DS1302_Read(ADDR_1302_Hour);     // 小时
	RTC_Time[minute]  =  DS1302_Read(ADDR_1302_Minute);	  // 分
	RTC_Time[second]  =  DS1302_Read(ADDR_1302_Second);   // 秒


	/* 转换成字符串 */
		for(i = 0; i < 6; i++) {

			itoa(RTC_Time[i] , RTC_Time_Char[i] , 16);   
			if(strlen(RTC_Time_Char[i]) == 1)
			{
					strcat(p_str_0,RTC_Time_Char[i]);   // 单数时间前面补0
				  strcpy(RTC_Time_Char[i],p_str_0);   
					strcpy(p_str_0,str_0);
			}
//			printf("%s, " ,  RTC_Time_Char[i]);
		}
																					
  /* 拼接并输出 */
	strcat(rtc_time_str, RTC_Time_Char[0]);
	printf("\r\n%s \r\n" ,  rtc_time_str);


	return p_rtc_time;
}


	
void RTC_Init(void)
{
	char *str = NULL;
	Reset_Pin(RTC_SCLK);
	Reset_Pin(RTC_CE);
	HAL_Delay(1);
	
//	printf("\r\n写入新的RTC时间：\r\n");
//	RTC_Write();

	str = RTC_Read();   	// 读取原始时间
	printf("\r\nRTC时间 = %s\r\n", str);

}


