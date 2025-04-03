#include "system.h"


void System_Init()
{
	Cache_Enable();                	//打开L1-Cache
	HAL_Init();				        			//初始化HAL库
	Stm32_Clock_Init(160,5,2,4);		//设置时钟,400Mhz	 
	LED_Init();											//LED 初始化
	KEY_Init();											//初始化按键
	Uart1_Init(115200);							//初始化串口
	Delay_Init(400);								//延时初始化
	SDRAM_Init();										//初始化SDRAM
	QSPI_MDMA_Init();  			        //配置QSPI总线 
	my_mem_init(SRAMIN);			      //初始化内部内存池(AXI)
	my_mem_init(SRAMEX);			      //初始化外部内存池(SDRAM)
	my_mem_init(SRAM12);			      //初始化SRAM12内存池(SRAM1+SRAM2)
	my_mem_init(SRAM4);				      //初始化SRAM4内存池(SRAM4)
	my_mem_init(SRAMDTCM);			    //初始化DTCM内存池(DTCM)
	my_mem_init(SRAMITCM);			    //初始化ITCM内存池(ITCM)
	EXTI_Init();
	IIC_Init();
	RTC_Init();
	
	//发送接收变量初始化
	g_IntMode = Receive;
	g_RecvCnt = 0;
  g_1stSynIsOK = 0;
	g_CommunicationMode = Receive;
	g_UartHEXLen = 0;
  g_SendByteLen = 0;
	g_DecodeEN = 0;
	g_save = 1;
	DIR_FLAG_LOW;
	delay_ms(2); 
	FPGA_TRANS_HIGH;
	
	//算法变量初始化
	g_fl = 10000;
	g_fh = 14000;
	g_fs = 96000;
	g_SynTime = 64.0/1000;
	g_alpha = 1;
	g_MaxByteLen = 30;
	g_MOrder=6;
	g_DopplerEstimation=1;
	g_DownSamplingNum = 3;
	ParameterInit();
		
	//功放初始化 32挡位并关闭
	g_PlvLevel = 5*g_PlvBase;
	POWER_AMP_OFF_ZZ_NEW();		//关闭功放
}

void Soft_Reset(void)
{
	__set_FAULTMASK(1); 						// 关闭所有中断
	NVIC_SystemReset(); 						// 复位
}




















