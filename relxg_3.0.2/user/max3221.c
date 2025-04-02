#include "max3221.h"
#include "gpio.h"
#include "usart.h"
#include <string.h>

/*  使用 USART3 */


// EN#  			PE10  // 低电平有效  接收关闭
// FORCEON  	PE8   
// FORCEOFF#	PE9 	// 低电平有效  电源关闭
// INVALID#   不连接



char RS232_Tx_Data[20] = "RS232 test";
//uint8_t d1[] = "功能1执行";  // 测试用
//uint8_t d2[] = "功能2执行";  // 测试用

uint8_t RS232_Rx_Buff = 0;   // 单
//uint8_t RS232_Rx_Buff[8] = {0};   // 数组形式 串口控制命令以8字节写入

void RS232_Init(void)
{
		MX_USART3_UART_Init();
	  RS232_RX_ENABLE;         // 接收使能
	  AUTO_PWR_DOWN_ENABLE;    // 无数据断电
		HAL_UART_Transmit(&huart3, (uint8_t *)RS232_Tx_Data, sizeof(RS232_Tx_Data), 0xFFFF); 
}


void RS232_Receive(void)
{
	 HAL_UART_Receive(&huart3, &RS232_Rx_Buff, 1, 0xFFFF);
	
	 HAL_UART_Transmit(&huart3, &RS232_Rx_Buff, 1, 0xFFFF);   // 回传指令
	
	 if(RS232_Rx_Buff == 0x56)
		{
						// 执行 功能1
			// HAL_UART_Transmit(&huart3, d1, sizeof(d1), 0xFFFF);  // 测试用
		}
		
		if(RS232_Rx_Buff == 0x58)
		{ 
			// 执行 功能2
			// HAL_UART_Transmit(&huart3,  d2, sizeof(d2), 0xFFFF); // 测试用
		}
}
