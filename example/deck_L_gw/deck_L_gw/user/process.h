#ifndef __PROCESS_H
#define __PROCESS_H


#include "APP_Work.h"




void init_struct_ID_CMD(void);

void read_key_value_do(u8 key_value);
void main_page_proccess(u8 key_value);
void main_num_key_do(u8 key_value);
/*输出功率设置*/
void singal_num_key_do(u8 key_value);

void control_page_proccess(u8 key_value);
void cmd_key_do(u8 key_value);
void home_key_action(void);
void scroll_key_action(void);
void clear_key_action(void);
/*ENTER 按键操作*/
void enter_key_action(void);

void singal_set_page_proccess(u8 key_value);

void running_cmd_page_proccess(u8 key_value);
/*version_message_page_proccess*/
void version_message_page_proccess(u8 key_value);

void num_key_do(u8 key_value);





void open_refresh_bettary_tim(void);
void close_refresh_bettary_tim(void);
void set_lcd_blink(void);

char* Int2String(int num,char *str);
char *CMD_int_to_string(void);
char *ID_int_to_string(void);
void Double2String(char *str,int len,double value);

// extern char RESPONSE_TEMP[][30];


#endif /*_Date_H*/

