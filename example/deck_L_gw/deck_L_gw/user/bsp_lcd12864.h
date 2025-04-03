/**  
 * All rights Reserved, Designed By S08
 * @Title:  bsp_lcd12864.h   
 * @Description:    TODO(用一句话描述该文件做什么)
 * @author: S08     
 * @date:   2019年9月11日 上午10:56:14
 * @version V1.0 
 * @Copyright: 2019 xxx Inc. All rights reserved. 
 * 注意：本内容仅限于
 */

#ifndef BSP_LCD12864_H_
#define BSP_LCD12864_H_

#include "APP_Work.h"



#define LCD_CS_1              Set_Pin(LCD_CS)
#define LCD_CS_0              Reset_Pin(LCD_CS)

#define LCD_SCLK_1            Set_Pin(LCD_CLK)
#define LCD_SCLK_0            Reset_Pin(LCD_CLK)

#define LCD_SID_1             Set_Pin(LCD_SID)
#define LCD_SID_0             Reset_Pin(LCD_SID)

#define DRAW_OPEN  1
#define DRAW_CLOSE 0
#define uchar unsigned char
#define lcd_locate_zh(x,y) lcd_locate(x,y)




void IOInitOut(void);                                       //把 所有端口 初始化为输出模式的函数。
void IOInitIn(void);                                        //把 数据data引脚 0--7 初始化为浮空输入的函数。
/*********基础时序操作  *****************/
void WaitBusy(void);                                        //等待12864的忙状态结束的函数。
void WriteCmd(u8 cmd);                                      //写命令函数。
void WriteData(u8 data);                                    //写数据函数。
/*********初始化函数  *******************/
void InitDis(void);                                        

void LCD_TestIntru(void);
	
/*********基础功能函数  *****************/
void lcd_locate(u8 X,u8 Y);
//void lcd_locate_zh(u8 X,u8 Y);                              

void DisStr(u8 *s);                                         //显示字符串的函数。
void DisInt(long int num);                                  //显示整型变量的函数,最多显示16位的整数。
void DisFloat(float fnum);                                  //显示有4位小数的浮点数，总位数不超过16位。
void lcd_clear(void);                                       //清屏函数,写入清屏命令
void lcd_Blink(char X, char Y);                           
void lcd_DisBlink(char X, char Y);


void CGRAM(void);
void DisplayGRAM(void);

/**返回字符表上的字符显示地址**/
u8 re_lcd_list_location(u8 num);
/********* 高级功能函数  ****************/
void lcd_DisByte(char X,char Y,char byte);
void lcd_DisStr(char X,char Y,char *s);                           //按起始位置显示字符串函数：X取1--4，Y取 1--8
void lcd_DisInt(long int num);

void lcd_clear_row (char X,char Y);                          //清空一行的字符

void hanzi_Disp(unsigned char x,unsigned char y,unsigned char *s);

static void CGRAM_Fill_Power(uint8_t power);
void GetPower_State(uint8_t battery);
void DisplayGRAM_Power(uint8_t battery);

void Lcd_Write_Cmd(uchar Cbyte );
void Lcd_Write_Data(uchar Dbyte );
void lcd_show_chinese(void);
void print_mode(uint8_t choose);


#endif /* BSP_LCD12864_H_ */
