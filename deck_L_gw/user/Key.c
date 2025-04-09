#include "Key.h"


// HOME  ->  0X80		SCROLL ->  0X40
// CLEAR ->  0X20		ENTER  ->  0X10

// 移植说明：  本次使用 4行5列的矩阵键盘 ，且右上角为独立按键     
// 驱动方式：  行输出列输入，逐个开启行，并检测列

							
							


void Open_beep(void)  // 开启蜂鸣器
{
	HAL_TIM_PWM_Start (&htim3,TIM_CHANNEL_1);   // 先关闭 太吵了
}

void Close_beep(void)  // 关闭蜂鸣器
{
	HAL_TIM_PWM_Stop (&htim3,TIM_CHANNEL_1);
}




void Matrix_Key_funtcion(u8 key_value)  // 矩阵按键功能执行函数
{

/* 改了原先框架的*/
	if(key_value == 0xff)
			return ;
	else{
				switch(menu.Current_Page)	{
			
					case MAIN_PAGE:  
									main_page_proccess(key_value);  	
									break;
					
					case CONTROL_PAGE:  
									control_page_proccess(key_value);  
									break;
					
					case SINGAL_SET_PAGE:  
									singal_set_page_proccess(key_value);  	
									break;
					
					case RUNNING_CMD_PAGE:  
									running_cmd_page_proccess(key_value);  	
									break;		
					
					case MESSAGE_PAGE:  
									version_message_page_proccess(key_value);  
									break;			
					
					case CMD_CYCLE_PAGE:  
									cmd_key_do(key_value);  	
									break;	
					
					case FINISH_CMD_PAGE:  
								cmd_key_do(key_value);  
								break;	
					default:break;  			}
				
	}
}





/**引脚状态读取函数**/
int get_key_status(void)
{
	u8 temp = 0;
	if(get_column1() == Press)temp|=1<<0;
	if(get_column2() == Press)temp|=1<<1;
	if(get_column3() == Press)temp|=1<<2;
	if(get_column4() == Press)temp|=1<<3;
	if(get_column5() == Press)temp|=1<<4;
	return temp;
}

/**输出引脚全拉高**/
void set_out_high(void)
{
	Set_row4_1();
	Set_row3_1();
	Set_row2_1();
	Set_row1_1();
}

u8 Matrix_Key_Scan(void)  // 矩阵按键扫描
{
	u8 KeyValue=0xff;
	
	set_out_high();
	
	/* HOME键扫描 外部中断驱动*/
	if(exit_key.Current_Key_Value == 0x80)  
	{	
		KeyValue = 0x80;
		exit_key.Current_Key_Value = 0x00;
		return KeyValue;
	}
	
	/*矩阵按键扫描*/
	if(get_key_status() != Release)
	{
		HAL_Delay(50);																					//延时消抖，该延时函数是软件延时，非准确延时
		if(get_key_status() != Release)
		{
			Open_beep();

			Set_row4_1();			//设置第四行为 高电平，其它行为低电平，通过扫描，可以判断那一列出现高电平
			Set_row3_0();
			Set_row2_0();
			Set_row1_0();
			HAL_Delay(5);
			switch(get_key_status()&0x1F)							//取低五位电平值，判断那一按键按下
			{
				case 0x01: KeyValue = 0x0c; goto over;break;				// 0000 0001   
				case 0x02: KeyValue = 0x0d; goto over;break;				// 0000 0010
				case 0x04: KeyValue = 0x0e; goto over;break;				// 0000 0100
				case 0x08: KeyValue = 0x0f; goto over;break;				// 0000 1000
				case 0x10: KeyValue = 0x10; goto over;break;				// 0001 0000
			} 

			Set_row4_0();
			Set_row3_1();		//设置第三行为 高电平
			Set_row2_0();
			Set_row1_0();
			HAL_Delay(5);
			switch(get_key_status()&0x1F)							
			{
				case 0x01: KeyValue = 0x08; goto over;break;				// 0000 0001 
				case 0x02: KeyValue = 0x09; goto over;break;				// 0000 0010
				case 0x04: KeyValue = 0x0a; goto over;break;				// 0000 0100
				case 0x08: KeyValue = 0x0b; goto over;break;				// 0000 1000
				case 0x10: KeyValue = 0x20; goto over;break;				// 0001 0000
			} 
			
			Set_row4_0();
			Set_row3_0();
			Set_row2_1(); 		//设置第二行为 高电平
			Set_row1_0();
			HAL_Delay(5);
			switch(get_key_status()&0x1F)							
			{
				case 0x01: KeyValue = 0x04; goto over;break;				// 0000 0001 
				case 0x02: KeyValue = 0x05; goto over;break;				// 0000 0010
				case 0x04: KeyValue = 0x06; goto over;break;				// 0000 0100
				case 0x08: KeyValue = 0x07; goto over;break;				// 0000 1000
				case 0x10: KeyValue = 0x40; goto over;break;				// 0001 0000
			} 
			
			Set_row4_0();
			Set_row3_0();
			Set_row2_0(); 
			Set_row1_1();       //设置第一行为 高电平
			HAL_Delay(5);
			switch(get_key_status()&0x1F)							
			{
				case 0x01: KeyValue = 0x00; goto over;break;				// 0000 0001 
				case 0x02: KeyValue = 0x01; goto over;break;				// 0000 0010
				case 0x04: KeyValue = 0x02; goto over;break;				
				case 0x08: KeyValue = 0x03; goto over;break;			
				// HOME 为独立按键				
			}
			over:
			set_out_high();
			while(get_key_status() != Release) {};				//等待按键松开
			Close_beep();
		}
	}
	return KeyValue;
}


uint8_t Key_Value = 0;   			// 8位按键数据   
uint8_t Key_Home_State = 3;   // HOME按键状态,注意和下方正常工作的状态不同
uint8_t Key_Scan_flag = 0;		// 可扫描标志

void HOME_Scan(void)
{
	if( Key_Scan_flag == 1)  // 1ms检测一次
	{
		Key_Scan_flag = 0;
		
		Key_Value <<= 1;
    Key_Value |= Read_Pin(KEY10); 
		
		// 低电平有效， 8位是 0X0F  ，16位则是0x00FF
		if(Key_Value == 0x0F) { //0000 1111    //正在抬起
		
			Key_Home_State = Release;  
	
		}else if(Key_Value == 0xF0 )  // 1111 0000   //正在按下 
				{ 
					exit_key.Current_Key_Value = 0x80;
//					Key_Home_State = Press; 
//					printf("Press");
//					/* 要执行的功能 */
//					HOME_Function();
				}
	}
}

void HOME_Function(void)
{
	Open_beep();
	
	lcd_DisBlink(1,0);
	/*不加延时LCD会显示BUG*/
	HAL_Delay (1);
	draw_main_page();
//	open_refresh_bettary_tim();
	
	Close_beep();
	
}

void Matrix_Key_Test(u8 key_value)  // 矩阵按键驱动测试： 按下什么显示什么
{
		/* 直接进入按键功能*/
	if(key_value == 0xff)
			return ;
	switch(key_value)  //
	{
		case 0x0c:  //K16 (第四行第一列)
			lcd_DisStr(0,0,"C");
			printf("C \r\n");
			break;

		case 0x0d:  //K17（第四行第二列）
			lcd_DisStr(0,0,"D");
			printf("D \r\n");
			break;
		
		case 0x0e:  //K18（第四行第三列）
			lcd_DisStr(0,0,"E");
			printf("E \r\n");
			break;
		
		case 0x0f:  //K19（第四行第四列）
			lcd_DisStr(0,0,"F");
			printf("F \r\n");
			break;
		
		case 0x10:  //K20（第四行第五列）
			lcd_DisStr(0,0,"ENTER");
			printf("ENTER \r\n");
			break;
	
		
		case 0x08:  //K11（第三行第一列）
			lcd_DisStr(0,0,"8");
			printf("8 \r\n");
			break;
		
		case 0x09:  //K12（第三行第二列）
			lcd_DisStr(0,0,"9");
			printf("9 \r\n");
			break;
		
		case 0x0a:  //K13（第三行第三列）
			lcd_DisStr(0,0,"A");
			printf("A \r\n");
			break;
		
		case 0x0b:  //K14（第三行第四列）
			lcd_DisStr(0,0,"B");
			printf("B \r\n");
			break;
		
		case 0x20:  //K15（第三行第五列）
			lcd_DisStr(0,0,"CLEAR");
			printf("CLEAR \r\n");
			break;
		
		
		case 0x04:  //K6（第二行第一列）
			lcd_DisStr(0,0,"4");
			printf("4 \r\n");
			break;
		
		case 0x05:  //K7（第二行第二列）
			lcd_DisStr(0,0,"5");
			printf("5 \r\n");
			break;
		
		case 0x06:  //K8（第二行第三列）
			lcd_DisStr(0,0,"6");
			printf("6 \r\n");
			break;
		
		case 0x07:  //K9（第二行第四列）
			lcd_DisStr(0,0,"7");
			printf("7 \r\n");
			break;
		
		case 0x40:  //K10（第二行第五列）
			lcd_DisStr(0,0,"SCROLL");
			printf("SCROLL \r\n");
			break;
		
		case 0x00:  //K1（第一行第一列）
			lcd_DisStr(0,0,"0");
			printf("0 \r\n");
			break;
		
		case 0x01:  //K2（第一行第二列）
			lcd_DisStr(0,0,"1");
			printf("1 \r\n");
			break;
		
		case 0x02:  //K3（第一行第三列）
			lcd_DisStr(0,0,"2");
			printf("2 \r\n");
			break;
		
		case 0x03:  //K4（第一行第四列）
			lcd_DisStr(0,0,"3");
			printf("3 \r\n");
			break;		
		// 0x10 KEY5 为独立按键（第一行第五列）
		default:break;
	}
}


