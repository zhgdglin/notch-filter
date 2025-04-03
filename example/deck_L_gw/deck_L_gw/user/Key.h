#ifndef __Key_H
#define __Key_H

#include "APP_Work.h"
#include "tim.h"

#define Press				1
#define Release			0


#define  Set_row4_1()   Set_Pin(KEY1)
#define  Set_row3_1()   Set_Pin(KEY2)
#define  Set_row2_1()   Set_Pin(KEY3)
#define  Set_row1_1()   Set_Pin(KEY4)

#define  Set_row4_0()     Reset_Pin(KEY1) 
#define  Set_row3_0()     Reset_Pin(KEY2)
#define  Set_row2_0()     Reset_Pin(KEY3)
#define  Set_row1_0()     Reset_Pin(KEY4)

#define  get_HOME()    				Read_Pin(KEY10)
#define  get_column5()				Read_Pin(KEY5)
#define  get_column4()	      Read_Pin(KEY6)
#define  get_column3()     	  Read_Pin(KEY7)
#define  get_column2()	      Read_Pin(KEY8)
#define  get_column1()	     	Read_Pin(KEY9)

extern uint8_t Key_Home_State; 
extern uint8_t Key_Scan_flag;

void Open_beep(void);
void Close_beep(void);
void Matrix_Key_funtcion(u8 key_value);
void Matrix_Key_Test(u8 key_value);
u8 Matrix_Key_Scan(void);
void HOME_Scan(void);
void HOME_Function(void);




#endif  
