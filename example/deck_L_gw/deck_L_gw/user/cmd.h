#ifndef __CMD_H
#define __CMD_H


#include "APP_Work.h"


//typedef struct 
//{
//	bool IDdata[8];   // 8位ID数据
//	bool CMDdata[8];  // 8位CMD数据 		 
//}ID_CMD;          	// 通信帧数据


//typedef struct 
//{
//	uint8_t IDdata;   // 8位ID数据
//	uint8_t CMDdata;  // 8位CMD数据 		 
//}ID_CMD;          	// 通信帧数据


//void Output_single_data(u8 figure);  
//u8 ID_CMD_Output_Fun(ID_CMD data);
//uint8_t Odd(char data);



//void Output_single_date(u8 fre);
//void switch_cmd_do(void);
//void CMD_55(void);
//void CMD_56(void);
//void CMD_47(void);
//void CMD_48(void);
//void CMD_49(void);
//void CMD_11(void);
//void send_wakeup(void);
//void Send_LFM(void);
//void Send_single_frequency(ID_CMD data);
//void Delay_10ms(uint16_t cnt);

//	
//u8 self_check_f(void);
//u8 stop_circular_response(void);
//u8 circular_response(void);
//u8 wait_release(void);
//u8 send_CMD_wait_response(void);
//u8 send_ID_wait_response(void);

//void Deck_Transmit_frame(ID_CMD data); 

//extern void switch_cmd_do(void);



void Send_wakeup(void);
void Send_aframe1(void);
void Send_LFM(void); 
void Send_single_frequency(bool* order_data);
void Deck_Send_frame(bool* order_data);
void Serial_Send_frame(void);

bool*  order_convert(uint8_t CMD_data);
void switch_cmd_do(void);
void CMD_55(void);  
void CMD_49(void);
void CMD_48(void);
void CMD_47(void); 

//uint8_t receive_deal(uint8_t singal_type);
uint8_t receive_deal_9K(void);

#endif

