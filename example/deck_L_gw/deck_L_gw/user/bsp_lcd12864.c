/**
 * All rights Reserved, Designed By S08
 * @Title:  bsp_lcd12864.c
 * @Description:    TODO(用一句话描述该文件做什么)
 * @author: S08
 * @date:   2019年9月11日 上午10:56:08
 * @version V1.0
 * @Copyright: 2019 xxx Inc. All rights reserved.
 * 注意：本内容仅限于
 */
//
#include "bsp_lcd12864.h"
#include "tim.h"


/* 
		LCD12864 驱动说明 
	  底层传输（协议）
		命令层  没有寄存器地址，根据代码的格式自动识别是什么命令   可通过RE开启扩展
		
*/
    



/*********定义GPIO结构体 ********************/
//GPIO_InitTypeDef  GPIOStru;                             //引脚号，速度，模式，定义用于定义所以引脚为输出的变量。

/****初始化 所有端口 为推挽输出模式  *********/
void IOInitOut(void)                                    //把 所有端口 初始化为推挽输出模式的函数。
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = LCD_CLK_Pin|LCD_SID_Pin|LCD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
}



/****初始化数据data引脚为浮空输入模式  *******/

void IOInitIn(void)                                     //把 数据引脚 初始化为浮空输入的函数。
{

}



//向LCD12864写字节函数
void SendByte(uchar Dbyte)
{
      uchar i;
     for(i=8;i>0;i--)
     {
       LCD_SCLK_0;

       if (Dbyte&0x80)
       {
           LCD_SID_1;
       }
       else
       {
           LCD_SID_0;
       }
       LCD_SCLK_1;   //如果显示有问题，则需在此后加延迟
       Delay_us(5);      //72MHZ速度太快，M0的48MHZ就无需加
       LCD_SCLK_0;
       Dbyte<<=1;
     }

}






void CheckBusy( void )
{

}


//写命令
void Lcd_Write_Cmd(uchar Cbyte )
{
     LCD_CS_1;

     SendByte(0xf8);              //11111,RW(0),RS(0),0
     SendByte(0xf0&Cbyte);
     SendByte(0xf0&Cbyte<<4);

     LCD_CS_0;
}

//写数据函数
void Lcd_Write_Data(uchar Dbyte )   // 单独使用此函数，将显示 字符表中对应的字符  
{
    LCD_CS_1;

    SendByte(0xfa);              //11111,RW(0),RS(1),0
    SendByte(0xf0&Dbyte);
    SendByte(0xf0&Dbyte<<4);

    LCD_CS_0;

}



/*********显示字符串的函数  ****************/
void DisStr(u8 *s)                                      //显示字符串的函数  ,传入数组名
{

    while(*s != '\0')
    {
        Lcd_Write_Data(*s);
        s++;
        HAL_Delay(2);                                   //控制每一个字符之间显示的时间 间隔
    }
}


/********中文起始显示位置设定函数  **************/
 /*   X取值为1-4；Y的取值为0-7 */

//void lcd_locate_zh(u8 X,u8 Y)  //设定显示位置：  先设定显示位置函数，再设定要显示的字符
void lcd_locate(u8 X,u8 Y)  		
{
  switch(X)
  {
    case 1: Lcd_Write_Cmd(0x80+Y); break;
    case 2: Lcd_Write_Cmd(0x90+Y); break;
    case 3: Lcd_Write_Cmd(0x88+Y); break;
    case 4: Lcd_Write_Cmd(0x98+Y); break;
  } 
	HAL_Delay(1);   /* 设点函数延迟，不加这句多个语句运行时会出现乱码*/
}




void lcd_DisByte(char X,char Y,char byte)
{

  lcd_locate_zh( X, Y);
  Lcd_Write_Data(byte);      
}


/********位置显示字符串函数  ****************/
/*
X:行取值为1-4；Y：列取值为0-7

*/
void lcd_DisStr(char X,char Y,char *s)
{

   lcd_locate_zh( X, Y);
//	 HAL_Delay(1);

   while(*s != '\0')      //判断字符串是否显示完毕
   {
     if(Y==16)            //判断换行
     {            //若不判断,则自动从第一行到第三行
        Y=0;
        X++;
        lcd_locate_zh( X, Y);
     }
     if(X==8 && Y==16)
     {
        X=0;      //一屏显示完,回到屏左上角
        lcd_locate_zh( X, Y);
     }

     Lcd_Write_Data(*s);      //
     s++;
     HAL_Delay(1);//2ms刷新有点慢
     Y++;
   }
	
}



////显示整型变量的函数
void lcd_DisInt(long int num)  //显示整型变量的函数,最多显示16位的整数。只能显示正数。
{
	u8 temp[17];
	u8 str[17];
	int i=0,j=0;
	while(num != 0)	  //这里不能用num%10 != 0，如果num是10的整数倍，
	                  //例如，100，这样就会出错，根本就不能进入循环体。
	{
		temp[i] = (num%10)+0x30;
		num/=10;
		i++;
	}
	i--;           //因为i在退出循环之前还自加了一次，此时，
	                //指向最后一个存储有用值的元素的后一个位置。
	while(i != -1)	 //因为i=0时，temp[0]还是有用值。
	{
		str[j] = temp[i];
		j++;
		i--;	
	}
	str[j]='\0';  //因为i在退出循环之前还自加了一次，此时，
	              //指向最后一个存储有用值的元素的后一个位置。
	DisStr(str);	
}



/**甲板单元使用，计算要显示16进制单个字符的地址（按手册字符表）**/
u8 re_lcd_list_location(u8 num)
{
	if (num<0x0a){num+=0x30;return num ;}
	else if(num >= 0x0a)
	{num+=0x37;return num ;}
	return num ;
}
/********清全屏函数  ***********************/
void lcd_clear(void)
{
   Lcd_Write_Cmd(0x01);
}


void lcd_Blink(char X, char Y) //游标显示
{
										
  lcd_locate_zh(X, Y); 
	Lcd_Write_Cmd(0x0F);        // 0000 1110   B2:开启显示  B2:开启游标显示  B0:不反白
	
}

void lcd_DisBlink(char X, char Y)
{
    lcd_locate_zh(X, Y); 
    Lcd_Write_Cmd(0x0C);        // 0000 1100  B2:开启显示  B2:关闭游标显示  B0:不反白
}

/**清空一行的字符**/
void lcd_clear_row (char X,char Y)
{
	uint8_t y = Y;
	for(; y<=7; y++)
		lcd_DisStr(X,y,"  ");
}




//void DisplayGRAM(void)
//{ 
////    lcd_locate_zh(4, 3);
//	    lcd_locate_zh(2, 7);//+
//    Lcd_Write_Data(0x00);
//    Lcd_Write_Data(0x00);


////    lcd_locate_zh(4, 4);
//	    lcd_locate_zh(2, 8);//+
//    Lcd_Write_Data(0x00);
//    Lcd_Write_Data(0x02);
//}
//void DisplayGRAM1(void)
//{ 


//    lcd_locate_zh(4, 3);
//    Lcd_Write_Data(0x00);
//    Lcd_Write_Data(0x04);


//    lcd_locate_zh(4, 4);
//    Lcd_Write_Data(0x00);
//    Lcd_Write_Data(0x06);
//}



void InitDis(void)                 /***初始化12864 和要用到的STM32 的GPIO******/                      
{
    IOInitOut();                //初始化 12864 和要用到的 STM 32 的引脚。
    HAL_Delay(5);

    Lcd_Write_Cmd(0x30);        //选择基本指令集，和，串行数据模式。   Function Set 指令
    HAL_Delay(5);

    Lcd_Write_Cmd(0x01);        //清除显示，并将 DDRAM 的地址计数器 AC 设为 00H. Display clear指令
    HAL_Delay(5);
    Lcd_Write_Cmd(0x02);	
    HAL_Delay(5);
    Lcd_Write_Cmd(0x06);        //设置，外部读写数据后,地址记数器 AC 会自动加 1。 Enry Mode Set指令
    HAL_Delay(5);
    Lcd_Write_Cmd(0x0c);        //开显示,无游标,不反白.  Display Control指令
    HAL_Delay(5);
	
	
//    Lcd_Write_Cmd(0x34); //绘图模式关闭
//    HAL_Delay(5);
//	  lcd_clear();
//    HAL_Delay(5);
//    Lcd_Write_Cmd(0x36); //绘图模式开启   /* 开启下面代码  会出现雪花屏 */
//    HAL_Delay(5); 
}

