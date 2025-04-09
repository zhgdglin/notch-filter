#ifndef  __MAX3221_H
#define  __MAX3221_H

/*   RS232   电平转换芯片  可选择无信号时进入低功耗*/

// EN  
#define RS232_RX_DISABLE  Set_Pin(RS232_EN)
#define RS232_RX_ENABLE   Reset_Pin(RS232_EN)
// FORCEON   FORCEOFF#   // 以下模式只能使能一种
#define RS232_POWER_OFF         Reset_Pin(MAX3221_FORCEOFF)
#define AUTO_PWR_DOWN_DISABLE   Set_Pin(MAX3221_FORCEOFF);Set_Pin(MAX3221_FORCEON)
#define AUTO_PWR_DOWN_ENABLE		Set_Pin(MAX3221_FORCEOFF);Reset_Pin(MAX3221_FORCEON)


void RS232_Init(void);
void RS232_Receive(void);

#endif

