#include "UI.h"
#include "adc.h"
///* 电池电压获取 WSJ */
//float Read_battery(void)  // 电池电压读取 最大3V
//{
//	HAL_ADC_Start(&hadc3);
//	HAL_ADC_PollForConversion(&hadc3, 0xff);
//	
//	static uint32_t  ADC_Internal_Data = 0;     // 内部ADC值  16位
//	static float     Battery_Voltage   = 0;     // 电池电压
//	
//	ADC_Internal_Data = HAL_ADC_GetValue(&hadc3);
//	Battery_Voltage = (ADC_Internal_Data*3.3)/65535;  
//	
//		return Battery_Voltage*7.27;  // 7.27= 24/3.3  ，转换成电池电压
//}


/**获取电池电压函数(杜工版)**/
static float GetPowerValue(void )
{
//        char data[10];
//        static float ave_Battery_remain;
        uint16_t ADC_value;
        static float Battery_remain;
//        static int ADC_cnt=0;
	
	/**获取电池电量**/
//	HAL_ADCEx_Calibration_Start(&hadc1,ADC_SINGLE_ENDED);  //L4版本
		HAL_ADCEx_Calibration_Start(&hadc3, ADC_CALIB_OFFSET,ADC_SINGLE_ENDED);  
		HAL_Delay (1);
	HAL_ADC_Start(&hadc3);
	
	ADC_value=HAL_ADC_GetValue (&hadc3);
	Battery_remain=ADC_value*3300/4096*9.2/0.986/1000;  // 0.986应该为偏差系数  万用表测量和ADC对比
 
	if(Battery_remain>21.6)
	{
			Battery_remain=Battery_remain-21.6;
			Battery_remain=Battery_remain/3.1;
			Battery_remain=Battery_remain*92.5+7.5;
			
			if(Battery_remain>=100)Battery_remain=100;
			if(Battery_remain<=1)Battery_remain=1;
	}
	else 
	{        
			//21.6计算的值为7.5%
			Battery_remain=Battery_remain-18.4;
			Battery_remain=Battery_remain/3.2;
			Battery_remain=Battery_remain*7.5;
	}
		HAL_ADC_Stop(&hadc3);
	return Battery_remain;
}



/**显示电池电压函数**/
void draw_battery_level(u8 x,u8 y)
{
	double  battery_level_temp = 0;
//	double battery_level = 0;
//	u8 temp_x = x;    //行
//	u8 temp_y = y;    //起始列
//	char temp[6]={0};
//	char temp1[5]={0};
	
	battery_level_temp = GetPowerValue();	
	
	
//	battery_level = get_battery_value();	
//	battery_level_temp = GetPowerValue();	
//	Double2String(temp,6,battery_level);
//	Double2String(temp1,4,battery_level_temp);
//	temp1[4]="%";
//	for(int i = 0;i<=3;i++){
//		temp[i] = battery_level%10;
//		battery_level/=10;
//	}
//	for(int i = 0;i<=3;i++){
//		if(i<2)
//		lcd_DisByte(temp_x,temp_y+i,re_lcd_list_location(temp[3-i]));
//		else
//		lcd_DisByte(temp_x,temp_y+i+1,re_lcd_list_location(temp[3-i]));
//	}
//	lcd_DisStr(temp_x,temp_y-3,"电压：");
//	lcd_DisStr(temp_x,temp_y,temp);
//	lcd_DisStr(temp_x,temp_y+3,"V");
	
	// 测试电池图标显示
//	static double  battery_level_temp = 0;
//	battery_level_temp = battery_level_temp + 10.0;
	
	GetPower_State((u8)battery_level_temp);
	DisplayGRAM_Power((u8)battery_level_temp);	
//	lcd_DisStr(2,5,temp1);
}



void draw_main_page(void)  /**画主界面**/
{
	lcd_clear();
	HAL_Delay (1);
	
//	DisplayGRAM();  //显示在第二行最右边
//	CGRAM();     
	draw_battery_level(1,3);   // 显示在右上角
	lcd_DisStr(2,0,"1.发射信号");
	lcd_DisStr(3,0,"2.发射信号强度");
	lcd_DisStr(4,0,"3.设备信息");
	menu.Current_Page = MAIN_PAGE;
}

void draw_control_page(void)  /**画操作界面**/
{
	lcd_clear();
	HAL_Delay (1);
	lcd_DisStr(1,0,"请输入设备ID");
	lcd_DisStr(2,0,"I");lcd_DisStr(2,1,"D");lcd_DisStr(2,2,"：");
	lcd_DisStr(3,0,"请输入命令");
	lcd_DisStr(4,0,"命");lcd_DisStr(4,1,"令");lcd_DisStr(4,2,"：");
	HAL_Delay (1);
	lcd_Blink(2,3);
//	lcd_DisBlink(2,3);
	menu.Current_Page = CONTROL_PAGE;
}

// void draw_set_singal_page(void)  /**画选择信号强度界面**/
// {
// 	lcd_clear();
// 	HAL_Delay (1);
	
// 	lcd_DisStr(1,0,"1.195dB");
// 	lcd_DisStr(2,0,"2.190dB");
// 	lcd_DisStr(3,0,"3.185dB");
// 	/*画箭头*/
// 	lcd_clear_row(2,4);
// 	lcd_clear_row(3,4);
// 	lcd_DisByte(1,4,0x1b);
	
// 	menu.Current_Page = SINGAL_SET_PAGE;
// }


// ...existing code...

char duty_input[4] = {0}; // 保存输入的占空比字符串
uint8_t duty_input_len = 0;

void draw_set_singal_page(void)
{
    lcd_clear();
    HAL_Delay(1);

    lcd_DisStr(1,0,"输入占空比(%)");
    lcd_DisStr(3,0,duty_input); // 显示当前输入
	// 	if(key_value == 0x80)
// 		home_key_action();

    menu.Current_Page = SINGAL_SET_PAGE;
}

// ...existing code...





void draw_cmd_base_page(void)   /**画命令基础界面**/
{
	lcd_clear();
	HAL_Delay (1);
	lcd_DisStr(1,0,"编号：");
	lcd_DisStr(2,0,"应答：");
	lcd_DisStr(3,0,"距离：");
	menu.Current_Page = RUNNING_CMD_PAGE;
}

void draw_version_message_page(void)  /**画固件信息页面**/
{
	lcd_clear();
	HAL_Delay (1);
	lcd_DisStr(1,0,"固件版本：");
	lcd_DisStr(1,5,"V0.0.1");
//	lcd_DisStr(3,0,"距离：");
	menu.Current_Page = MESSAGE_PAGE;
}

void draw_cmd_cycle_page(void)  /**画释放循环命令第二界面**/
{
	lcd_clear_row(3,0);
	lcd_clear_row(4,0);
	lcd_DisStr(3,0,"释放：");
	lcd_DisStr(4,0,"循环：");
//	menu.Current_Page = CMD_CYCLE_PAGE;
}






//unsigned char AC_TABLE[]={
//    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,      //第一行汉字位置
//    0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,      //第二行汉字位置
//    0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,      //第三行汉字位置
//    0x98,0x99,0x9a,0x9b,0x9c,0x9d,0x9e,0x9f,      //第四行汉字位置
//};

//uchar fanlong[] = {
//    0x10,0x80,0x08,0x80,0x7E,0xFC,0x24,0x80,0x18,0xFC,0xFF,0x04,0x00,0xFC,0x3E,0x80,
//    0x22,0xF8,0x3E,0x80,0x22,0xF8,0x3E,0x80,0x22,0xFA,0x22,0x82,0x2A,0x82,0x24,0x7E };/*龍,16 X 16 ,逐行式，顺向（高位，低位）*/

//uchar guo[] = {
//    0x7F,0xFC,0x40,0x84,0x40,0xA4,0x40,0xA4,0x5F,0xF4,0x40,0x84,0x5E,0xA4,0x52,0xB4,
//    0x5E,0x64,0x40,0x44,0x46,0xD4,0x59,0x54,0x42,0x24,0x40,0x04,0x7F,0xFC,0x40,0x04}; /*"國",0*//* (16 X 16 , 宋体 )*/

////逐行式 阳码，即带白色底纹
//uchar fan[] = {0xFF,0xF7,0xDF,0xC3,0xEC,0x3F,0xED,0xFF,0xFD,0xFF,0xFC,0x03,0x0D,0xFB,0xED,0x77,0xED,0xAF,0xED,0xDF,0xED,0xAF,0xEB,0x77,0xEA,0xFB,0xD7,0xFF,0xB8,0x01,0xFF,0xFF};/*"·μ",0*/
//uchar hui[] = {0xFF,0xFF,0xC0,0x07,0xDF,0xF7,0xDF,0xF7,0xD8,0x37,0xDB,0xB7,0xDB,0xB7,0xDB,0xB7,0xDB,0xB7,0xDB,0xB7,0xD8,0x37,0xDF,0xF7,0xDF,0xF7,0xC0,0x07,0xDF,0xF7,0xFF,0xFF};/*"??",1*/
//uchar she[] = {0xFF,0xFF,0xDE,0x0F,0xEE,0xEF,0xEE,0xEF,0xFE,0xEF,0xFD,0xF1,0x0B,0xFF,0xEC,0x07,0xEE,0xF7,0xEE,0xEF,0xEF,0x6F,0xEB,0x5F,0xE7,0xBF,0xEF,0x5F,0xEF,0xE7,0xF3,0xF9};
//uchar ding[]= {0xFD,0xFF,0xFE,0xFF,0x80,0x01,0xBF,0xFD,0x7F,0xFB,0xFF,0xFF,0xC0,0x07,0xFE,0xFF,0xFE,0xFF,0xEE,0xFF,0xEE,0x07,0xEE,0xFF,0xEE,0xFF,0xD6,0xFF,0xB8,0x01,0x7F,0xFF};



/***杜工最新给的***/
uchar power[]=
  {                                                            
		0x00, 0x00,                                                                        
		0x00, 0x00,                                                                         
		0x00, 0x00,                                                                         
		0x00, 0x00,                                                                         
		0x7F, 0xFC,                                                                         
		0x40, 0x04,                                                                     
		0x40, 0x26,                                                                       
		0x5C, 0x76,                                                                     
		0x40, 0x26,
		0x40, 0x04, 
		0x7F, 0xFC, 
		0x00, 0x00, 
		0x00, 0x00, 
		0x00, 0x00, 
		0x00, 0x00, 
    0x00, 0x00, 
 };

void CGRAM(void)
{
//    uchar i;
//                
//        Lcd_Write_Cmd(0x40);//电源图标
//        for(i=0;i<32;i++)
//        {
//                Lcd_Write_Data(power[i]); //
//                HAL_Delay(2);
//        }
//        
//                
//                Lcd_Write_Cmd(0x60); //设
//    for(i=0;i<32;i++)
//    {
//        Lcd_Write_Data(she[i]); //
//        HAL_Delay(2);
//    }

//    Lcd_Write_Cmd(0x70);//置
//    for(i=0;i<32;i++)
//    {
//               Lcd_Write_Data(ding[i]); //
//               HAL_Delay(2);
//    }

}



void DisplayGRAM(void)  // 显示GRAM中的图像
{ 
    lcd_locate_zh(2,7);
    Lcd_Write_Data(0x00);
    Lcd_Write_Data(0x00);

}
void DisplayGRAM1(void)
{ 
    lcd_locate_zh(4, 3);
    Lcd_Write_Data(0x00);
    Lcd_Write_Data(0x04);


    lcd_locate_zh(4, 4);
    Lcd_Write_Data(0x00);
    Lcd_Write_Data(0x06);
}


/** 电池图标图像数组 **/ 
uchar power_new_100[]=  //100%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xBF, 0xF6, 
  0xBF, 0xF6, 
  0xBF, 0xF6, 
  0xBF, 0xF6, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};
 
uchar power_new_90[]= //90%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xBF, 0xE6, 
  0xBF, 0xE6, 
  0xBF, 0xE6, 
  0xBF, 0xE6, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_80[]= //80%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xBF, 0xC6, 
  0xBF, 0xC6, 
  0xBF, 0xC6, 
  0xBF, 0xC6, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_70[]= //70%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xBF, 0x86, 
  0xBF, 0x86, 
  0xBF, 0x86, 
  0xBF, 0x86, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_60[]=  //60%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xBF, 0x06, 
  0xBF, 0x06, 
  0xBF, 0x06, 
  0xBF, 0x06, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_50[]=   //50%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xBE, 0x06, 
  0xBE, 0x06, 
  0xBE, 0x06, 
  0xBE, 0x06, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_40[]=	//40%
{
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0xFF, 0xFC, 
	0x80, 0x04, 
	0xBC, 0x06, 
	0xBC, 0x06, 
	0xBC, 0x06, 
	0xBC, 0x06, 
	0x80, 0x04, 
	0xFF, 0xFC, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
};

uchar power_new_30[]= //30%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xB8, 0x06, 
  0xB8, 0x06, 
  0xB8, 0x06, 
  0xB8, 0x06, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_20[]=	//20%
{
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0xFF, 0xFC, 
	0x80, 0x04, 
	0xB0, 0x06, 
	0xB0, 0x06, 
	0xB0, 0x06, 
	0xB0, 0x06, 
	0x80, 0x04, 
	0xFF, 0xFC, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
};

uchar power_new_10[]= //10%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0xFF, 0xFC, 
  0x80, 0x04, 
  0xA0, 0x06, 
  0xA0, 0x06, 
  0xA0, 0x06, 
  0xA0, 0x06, 
  0x80, 0x04, 
  0xFF, 0xFC, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};

uchar power_new_0[]=	//00%
{
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x7F, 0xFE, 
  0x40, 0x02, 
  0x40, 0x03, 
  0x40, 0x03, 
  0x40, 0x03, 
  0x40, 0x03, 
  0x40, 0x02, 
  0x7F, 0xFE, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
  0x00, 0x00, 
};




static void CGRAM_Fill_Power(uint8_t power) 
{
 uint8_t i;
 
 //填充电池图标位置
 Lcd_Write_Cmd(0x40);
 
 switch(power)
 {
 case 1:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_10[i]); 
	HAL_Delay(2);
	}
	break;
 case 2:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_20[i]); 
	HAL_Delay(2);
	}
	break;
 case 3:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_30[i]); 
	HAL_Delay(2);
	}
	break;
 case 4:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_40[i]); 
	HAL_Delay(2);
	}
	break;
 case 5:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_50[i]); 
	HAL_Delay(2);
	}
	break;
 case 6:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_60[i]); 
	HAL_Delay(2);
	}
	break;
 case 7:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_70[i]); 
	HAL_Delay(2);
	}
	break;
 case 8:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_80[i]); 
	HAL_Delay(2);
	}
	break;
 case 9:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_90[i]); 
	HAL_Delay(2);
	}
	break;
 case 10:
	for(i=0;i<32;i++)
	{
	Lcd_Write_Data(power_new_100[i]); 
	HAL_Delay(2);
	}
	break; 
	}
}


uint8_t power_state; //状态

void GetPower_State(uint8_t battery)
{ 
 if(battery >= 95)	//95%
	{ 
	power_state = 10;
	}
	else if(battery >= 85) //85%?-95%
	{
	power_state = 9; 
	}
	else if(battery >= 75) //75%?-85%
	{
	power_state = 8;
	}
	else if(battery >= 65) //65%?-75%
	{                      
	power_state = 7;     
	}                      
	else if(battery >= 55) //55%?-65%
	{                      
	power_state = 6;     
	}                      
	else if(battery >= 45) //45%?-55%
	{                      
	power_state = 5;     
	}                      
	else if(battery >= 35) //35%?-45%
	{                      
	power_state = 4;     
	}                      
	else if(battery >= 25) //25%?-35%
	{                      
	power_state = 3;     
	}                      
	else if(battery >= 15) //15%?-25%
	{
	power_state = 2;
	} 
	else //1%?-15%
	{
	power_state = 1;
	}
}

void DisplayGRAM_Power(uint8_t battery)
{
	switch(power_state)
	{
	case 10:
	if(battery < 92)
	{
	power_state=9;
	}
	break;
	case 9:
	if(battery<82)
	{
	power_state=power_state-1;
	}
	else if(battery>97)
	{
	power_state=power_state+1;
	}
	break;
	case 8:
	if(battery<72)
	{
	power_state=power_state-1;
	}
	else if(battery>87)
	{
	power_state=power_state+1;
	}
	break;
	case 7:
	if(battery<62)
	{
	power_state=power_state-1;
	}
	else if(battery>77)
	{
	power_state=power_state+1;
	}
	break;
	case 6:
	if(battery<52)
	{
	power_state=power_state-1;
	}
	else if(battery>67)
	{
	power_state=power_state+1;
	}
	break;
	case 5:
	if(battery<42)
	{
	power_state=power_state-1;
	}
	else if(battery>57)
	{
	power_state=power_state+1;
	}
	break;
	case 4:
	if(battery<32)
	{
	power_state=power_state-1;
	}
	else if(battery>47)
	{
	power_state=power_state+1;
	}
	break;
	case 3:
	if(battery<22)
	{
	power_state=power_state-1;
	}
	else if(battery>37)
	{
	power_state=power_state+1;
	}
	break;
	case 2:
	if(battery<12)
	{
	power_state=power_state-1;
	}
	else if(battery>27)
	{
	power_state=power_state+1;
	}
	break;
	case 1:
	if(battery>17)
	{
	power_state=power_state+1;
	}
	break;
	}

	CGRAM_Fill_Power(power_state);
	
	lcd_locate_zh(1,7);
	Lcd_Write_Data(0x00);
	Lcd_Write_Data(0x00);
}


