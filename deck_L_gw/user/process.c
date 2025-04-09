#include "process.h"

/* 按键的响应过程 */


char RESPONSE_TEMP[][30] =
{	
						{"等待应答"},
						{"收到应答"},
						{"超时"},
};

							
void init_struct_ID_CMD(void)  /**画操作界面前需要的初始化结构体内参数**/
{
	HAL_Delay (1);
	ID.date_num = 0;
	ID.full_status = 0;
	CMD.date_num = 0;
	CMD.full_status = 0;
}				
							
void main_num_key_do(u8 key_value)  /*主界面数字键动作*/
{
	switch (key_value)
	{
		case 0x01: 	
					init_struct_ID_CMD(); 
					draw_control_page(); 
					close_refresh_bettary_tim(); 
					break ;
		
		case 0x02:  
					draw_set_singal_page();	
					close_refresh_bettary_tim();
					break ;
		
		case 0x03:  
					draw_version_message_page();
					close_refresh_bettary_tim();
					break ;
	}
}

void main_page_proccess(u8 key_value)
{
//	if(key_value>=0x10){
//	/***命令按键操作***/
//		cmd_key_do(key_value);
//		return ;	
//	}else
	if(key_value<0x10){
	/***数字按键操作***/
		main_num_key_do(key_value);
		return ;	
	}	
}


void version_message_page_proccess(u8 key_value)   /* 显示版本号 */
{
	if(key_value == 0x80)  // HOME返回
	home_key_action(); 
}



void singal_num_key_do(u8 key_value)  /*输出功率设置*/
{
	switch (key_value)
	{
		case 0x01: 	lcd_clear_row(2,4);lcd_clear_row(3,4);lcd_DisByte(1,4,0x1b); 
								// 待加入具体的执行函数
		break ;
		case 0x02:  lcd_clear_row(1,4);lcd_clear_row(3,4);lcd_DisByte(2,4,0x1b); 
								// 待加入具体的执行函数
		break ;
		case 0x03:  lcd_clear_row(1,4);lcd_clear_row(2,4);lcd_DisByte(3,4,0x1b); 
							 // 待加入具体的执行函数
		break ;
	}	
}


void singal_set_page_proccess(u8 key_value)
{
//	if(key_value>=0x10){
//	/***命令按键操作***/
//		singal_cmd_key_do(key_value);
//		return ;	
//	}else 
	if(key_value<0x10){
	/***数字按键操作***/
		singal_num_key_do(key_value);
		return ;	
	}
	if(key_value == 0x80)
		home_key_action();
}


void running_cmd_page_proccess(u8 key_value)
{
	switch(key_value){
		case 0x10:				
			/*ENTER 操作*/
			enter_key_action();	
			break ;
		case 0x40:	
			/*SCROLL 操作*/
			scroll_key_action();
			break ;
		case 0x80:
			home_key_action();
			break ;
	}
}


void enter_key_action(void)   /*ENTER 按键操作*/
{
	if((ID.full_status == 1)&&(CMD.full_status == 1))  // 命令和ID输入满了就到转到下一个页面（无效指令也会）
	{
		/**基础显示**/
		lcd_DisBlink(1,0);
			HAL_Delay (1);
		draw_cmd_base_page();
		lcd_DisStr(1,3,ID_int_to_string());  // “ID编号显示”
		lcd_DisStr(2,3,RESPONSE_TEMP[0]);    // “等待应答”
		/**命令判断分支执行**/
		switch_cmd_do();
	}else return ;
	/**完成一遍命令流程后更改页面**/
	if(menu.Current_Page != MAIN_PAGE)
	menu.Current_Page = FINISH_CMD_PAGE;
}

/*CLEAR 按键操作*/
void clear_key_action(void)
{
	if ((CMD.date_num != 0)&&(ID.full_status == 1)){
		/**当CMD 数据不为0，即ID数据满了的时候**/
		/*清除CMD full标志位*/
		if(CMD.full_status == 1)CMD.full_status =0;
		/*清除一位*/
		lcd_DisStr(4,2+CMD.date_num,"  ");
		lcd_Blink(4,2+CMD.date_num);
			CMD.date_num --;
//		for(u8 i = 0;i<=CMD.date_num;i++)
//		lcd_DisByte(4,3+CMD.date_num,re_lcd_list_location(CMD.date_value[CMD.date_num]));		
	}else if((CMD.date_num == 0)&&(ID.date_num != 0)){
		/**CMD命令数据位为0，ID数据位还有的时候do**/
		/*清除ID full标志位*/
		if(ID.full_status == 1)ID.full_status =0;
		/*清除一位*/
		lcd_DisStr(2,2+ID.date_num,"  ");
		lcd_Blink(2,2+ID.date_num);
			ID.date_num --;
	}
}


void open_refresh_bettary_tim(void)  /*开启刷新电池电量定时器*/
{
//	HAL_TIM_Base_Start_IT(&htim6);
}

void close_refresh_bettary_tim(void)  /*关闭刷新电池电量定时器*/
{
//	HAL_TIM_Base_Stop_IT(&htim6);
//	TIME6_Callback_refresh_Status = 0;
}

/*HMOE 按键操作*/
void home_key_action(void)
{	
	Open_beep();
	
	lcd_DisBlink(1,0);
	/*不加延时LCD会显示BUG*/
	HAL_Delay (1);
	draw_main_page();
	open_refresh_bettary_tim();
	
	Close_beep();
}

/*SCROLL 按键操作*/
void scroll_key_action(void)
{
	lcd_clear();
	/*不加延时LCD会显示BUG*/
	HAL_Delay (1);
	lcd_DisBlink(1,0);
	init_struct_ID_CMD();
	draw_control_page();
}
/*数字按键操作*/
void num_key_do(u8 key_value)
{
//	u8 temp;
	if (ID.full_status == 0){
		/**ID数据键入**/
		ID.date_value[ID.date_num] = key_value;
//				temp = re_lcd_list_location(ID.date_value[ID.date_num]);
		for(u8 i = 0;i<=ID.date_num;i++)
		lcd_DisByte(2,3+ID.date_num,re_lcd_list_location(ID.date_value[ID.date_num]));
		ID.date_num ++;
		if(ID.date_num == 4){
			ID.full_status = 1;
		/*命令频率与ID频率同*/
			if(CMD.date_num == 0){
			CMD.date_value[0] = ID.date_value[0];
			CMD.date_value[1] = ID.date_value[1];
			CMD.date_num = 1;
				/*显示*/
			for(u8 i = 0;i<=CMD.date_num;i++)
			lcd_DisByte(4,3+i,re_lcd_list_location(CMD.date_value[i]));			
			}
			CMD.date_num = 2;
		}
		set_lcd_blink();
	}
	else if((CMD.full_status == 0)&&(ID.full_status == 1)){
		/**CMD数据键入**/
		CMD.date_value[CMD.date_num] = key_value;
//				temp = re_lcd_list_location(ID.date_value[ID.date_num]);
		for(u8 i = 0;i<=CMD.date_num;i++)
		lcd_DisByte(4,3+CMD.date_num,re_lcd_list_location(CMD.date_value[CMD.date_num]));
		CMD.date_num ++;
		if(CMD.date_num == 4)CMD.full_status = 1;
		set_lcd_blink();		
	}else{
	/**命令与ID都存满了，键入无效，等待删除数值后才能正常键入**/
		return;
	}
}


// 命令按键操作
// HOME  ->  0X80		SCROLL ->  0X40
// CLEAR ->  0X20		ENTER  ->  0X10
void cmd_key_do(u8 key_value)
{
	switch(key_value)
	{
		case 0x10:  enter_key_action(); 	break ;	 /* ENTER  操作 */
		case 0x20: 	clear_key_action();		break ;  /* CLEAR  操作 */
		case 0x40: 	scroll_key_action();	break ;	 /* SCROLL 操作 */
		case 0x80: 	home_key_action();		break ;  /* HOME   操作 */
		default:   	printf("CMD Key input error!"); break ;
	}
}


void control_page_proccess(u8 key_value)
{
	if(key_value>=0x10){
	/***命令按键操作***/
		cmd_key_do(key_value);
		return ;	
	}else if(key_value<0x10){
	/***数字按键操作***/
		num_key_do(key_value);
		return ;	
	}	
}



/***********************杂****************************/
void Double2String(char *str,int len,double value)
{
	len = snprintf(str,len,"%0.9lf",value);
	
	for(int i = len-1;i >= 0;i--){
		if(str[i] == '\0') continue;
		
		if(i == 0) return;
		//小数点退出
		if(str[i] == '.') {
			str[i] = '\0';
			return;
		}
		
		if(str[i] != '0') return;
		
		//把零消除掉
		if(str[i] == '0'){
			str[i] = '\0';
		}
		//如果小数点后一位也是零，那么将小数点也给消除掉
		if(str[i] == '0' && str[i-1] != '0'){
			if(str[i-1] == '.'){
				str[i-1] = '\0';
				return;
			}
			str[i] = '\0';
			return;
		}
	}
}

/**16进制转字符串**/
int hex2str(unsigned int data, char* s, int len)
{
    int i;
 
    s[len] = 0;
    for (i = len - 1; i >= 0; i--, data >>= 4)
    {
        if ((data & 0xf) <= 9)
            s[i] = (data & 0xf) + '0';
        else
            s[i] = (data & 0xf) + 'A' - 0x0a;
    }
    return 1;
}
/**
整形转字符串
使用例程：
int main()
{
    int number1 = 123456;
    int number2 = -123456;
    char string[16] = {0};
    Int2String(number1,string);
    printf("数字：%d 转换后的字符串为：%s\n",number1,string);
    Int2String(number2,string);
    printf("数字：%d 转换后的字符串为：%s\n",number2,string);
    return 0;
}
**/
char* Int2String(int num,char *str)//10进制 
{
    int i = 0;//指示填充str 
    if(num<0)//如果num为负数，将num变正 
    {
        num = -num;
        str[i++] = '-';
    } 
    //转换 
    do
    {
        str[i++] = num%10+48;//取num最低位 字符0~9的ASCII码是48~57；简单来说数字0+48=48，ASCII码对应字符'0' 
        num /= 10;//去掉最低位    
    }while(num);//num不为0继续循环
    
    str[i] = '\0';
    
    //确定开始调整的位置 
    int j = 0;
    if(str[0]=='-')//如果有负号，负号不用调整 
    {
        j = 1;//从第二位开始调整 
        ++i;//由于有负号，所以交换的对称轴也要后移1位 
    }
    //对称交换 
    for(;j<i/2;j++)
    {
        //对称交换两端的值 其实就是省下中间变量交换a+b的值：a=a+b;b=a-b;a=a-b; 
        str[j] = str[j] + str[i-1-j];
        str[i-1-j] = str[j] - str[i-1-j];
        str[j] = str[j] - str[i-1-j];
    } 
    
    return str;//返回转换后的值 
}

void set_lcd_blink(void)  /*按键游白位置*/
{
		if((ID.date_num != 0)&&(ID.full_status !=1))
		{
			lcd_Blink(2,3+ID.date_num);	
		}else if((ID.full_status == 1)&&(CMD.full_status !=1))
		{
			lcd_Blink(4,3+CMD.date_num);
		}else if(CMD.full_status == 1)
		{
			lcd_DisBlink(4,2+CMD.date_num);
		}
		return ;
}
 
char *ID_int_to_string(void)  /**输入ID数组转字符串显示**/
{
	u16 temp = 0;
	static char ID_Str_temp [4]={0};
	for (int i =0;i<4;i++){
		temp += (ID.date_value [3-i]<<(4*i)); 
	}
	hex2str(temp,ID_Str_temp,4);
	return ID_Str_temp;
}

char *CMD_int_to_string(void)  /**输入CMD数组转字符串显示**/
{
	u16 temp = 0;
	static char CMD_Str_temp [4]={0};
	for (int i =0;i<4;i++){
//		temp *= 16;
		temp += (CMD.date_value [3-i]<<(4*i)); 
	}
//	Double2String(ID_Str_temp,5,temp);
	hex2str(temp,CMD_Str_temp,4);
	return CMD_Str_temp;
}

