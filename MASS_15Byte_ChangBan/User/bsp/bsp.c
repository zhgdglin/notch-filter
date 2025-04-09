/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.3
Date: 	       // 2023/08/10
Description:   // 简版通信机驱动，无任何算法应用
History:       // V.0、V.1
*******************************************************************/
#include "bsp.h"

/*
*********************************************************************************************************
*	                                   函数声明
*********************************************************************************************************
*/
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
static void MPU_Config(void);


/*
*********************************************************************************************************
*	函 数 名: bsp_Init
*	功能说明: 初始化所有的硬件设备。该函数配置CPU寄存器和外设的寄存器并初始化一些全局变量。只需要调用一次
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Init(void)
{
	MPU_Config();
	CPU_CACHE_Enable();
	HAL_Init();
	SystemClock_Config();

	/*Event Recorder：
	 - 可用于代码执行时间测量，MDK5.25及其以上版本才支持，IAR不支持。
	 - 默认不开启，如果要使能此选项，务必看V7开发板用户手册第8章*/	
  
#if Enable_EventRecorder == 1  
	/* 初始化EventRecorder并开启 */
	EventRecorderInitialize(EventRecordAll, 1U);
	EventRecorderStart();
#endif
    bsp_InitExtIO();
	bsp_InitTimer();  	    /* 初始化滴答定时器 */
	LED_Init();
    IWDG_Init(IWDG_PRESCALER_64,5*500); 	//分频数为64,重载值为500,溢出时间为5s

  
  /* 针对不同的应用程序，添加需要的底层驱动模块初始化函数 */	
  /* SDMMC初始化 */
    bsp_InitExtSDRAM();     /* 初始化SDRAM */
  /* 初始化动态内存空间 */
    my_mem_init(SRAMIN);			      //初始化内部内存池(AXI 448kB)
	my_mem_init(SRAMEX);			      //初始化外部内存池(SDRAM 28MB)
	my_mem_init(SRAM12);			      //初始化SRAM12内存池(SRAM1+SRAM2 240kB)
	my_mem_init(SRAM4);				      //初始化SRAM4内存池(SRAM4 60kB)
	my_mem_init(SRAMDTCM);			      //初始化DTCM内存池(DTCM 120kB)
    QSPI_MDMA_Init();
    bsp_InitSPIBus();	      /* 配置SPI1总线 */
 
	bsp_UartInit(115200);   /* 初始化串口 */	

	FatFsInit();

	STM32H7ToFPGABusInit();
    EXTI_Init();
	RTC_Init();
  
    DRDY_Low;//FPGA进入接收模式
	g_CommunicationMode = Receive;	
	
    DIR_FLAG_LOW;
	FPGA_TRANS_HIGH;
  
    PGA281CtrlInit();
	PowerAmplifierCtrlInit();
  
  //串口挂在了DMA，启动要清除状态
  __HAL_UART_CLEAR_IDLEFLAG(&huart1);
    
	
	AlgorithmVarialbeInit();
	PowerSet(0x33);
	PreAmpSet(0x34);
	
	g_spiLen = g_ReceiveBlock*4;
	g_IntMode=Receive;

	    //接受信号相关变量初始化
	    g_SynOK = 0;
		g_FirstSynOK = 0;
		g_SecondSynOK = 0;
		//信号发射相关变量初始化
		g_SendOK = 0;
		g_TransCnt = 0;
		g_C_TransCnt = 0;
		g_RxCnt = 0;
		
//	    g_PowrLevel = g_PlvBase*5;
//		g_WorkMode = IDLE;//IDLE;
//	    g_IntMode = Receive;//Receive;
		
		//SD卡存储相关变量初始化
		g_StorageDataOK = 1;	
		g_SaveCnt = 0;
        g_StoreBlockNum = 1;
		g_SaveOK = 0;
		
		//测试参数
		flag = 0;
		bsp_spiTransfer();
		
		SysAnswerCmd.POWR_ANSWER[6] = 0x33;
		Uart1_Puts(SysAnswerCmd.POWR_ANSWER,9);	
}
  



void AlgorithmVarialbeInit(void)
{
	
	//发射机测试	
	g_CacheB = mymalloc(SRAM12, g_DataBagLen*4);//24k
	memset(&g_CacheB[0], 0, g_DataBagLen*4);
	
	g_Cachef32 = mymalloc(SRAM12, g_DataBagLen*4);//24k  SRAMEX
	memset(&g_Cachef32[0] , 0, g_DataBagLen*4);
	
	//调试加的注释
	g_FrameSig = mymalloc(SRAMEX,g_DataBagLen*(g_FrameBlockNum+2)*sizeof(float)); //552k
	memset(&g_FrameSig[0],0,g_DataBagLen*(g_FrameBlockNum+2)*sizeof(float));
	
	g_Backup = mymalloc(SRAMEX,g_DataBagLen*(g_FrameBlockNum+2)*sizeof(float));//2包用作同步，18+2包储存数据，552k
	memset(&g_Backup[0], 0, g_DataBagLen*(g_FrameBlockNum+2)*sizeof(float));
	
	g_HFMRefConj = mymalloc(SRAMEX,2048*4*2);//32k
	memset(&g_HFMRefConj[0],0,2048*4*2);	
	LocalSynRefGen_HFM(g_HFMRefConj);        //生成本地HFM频域拷贝信号
	
	g_LFMRefConj = mymalloc(SRAMEX,2048*4*2);//32k
	memset(&g_LFMRefConj[0],0,2048*4*2);	
	LocalSynRefGen_LFM(g_LFMRefConj);        //生成本地HFM频域拷贝信号
	
	g_uChar = mymalloc(SRAMEX,g_SigSendLen*2*sizeof(char));      
	memset(&g_uChar[0],0,g_SigSendLen*2*sizeof(char));
	
	g_RxBuff = mymalloc(SRAMEX,(2048+128)*sizeof(char));
	memset(&g_RxBuff[0],0,(2048+128)*sizeof(char));//+128是给字节数表示和校验位的
	
	memset(g_spiTxBuf,0,SPI_BUFFER_SIZE);
	memset(g_spiRxBuf,0,SPI_BUFFER_SIZE);
	
	g_uCharTD = mymalloc(SRAMEX,6144*15*2*sizeof(char));      
	memset(&g_uCharTD[0],0,6144*15*2*sizeof(char));
	
	g_uShortTD = mymalloc(SRAMEX,6144*15*sizeof(short));      
	memset(&g_uShortTD[0],0,6144*15*sizeof(short));
	
	g_Sig = mymalloc(SRAMEX,6144*15*sizeof(float));      
	memset(&g_Sig[0],0,6144*15*sizeof(float));

	g_LFM = mymalloc(SRAMEX,6144*sizeof(float));      
	memset(&g_LFM[0],0,6144*sizeof(float));
	LFMSignalGen(0.064f,g_LFM);
	
	g_SynHFM = mymalloc(SRAMIN,g_ReceiveBlock*4);//24k
	memset(&g_SynHFM[0],0,g_ReceiveBlock*4);
	HFMSignalGen(0.064f,g_SynHFM);
	
	g_UpSamplingMultipleMASS = floor(g_fs/g_fb*(1+g_alpha_filter));
	g_TotalSymbolNumPerFrame = g_InfoSymbolNumPerFrameMASS+g_PolitSymbolNumPerFrame;
	g_r = log2(g_M);
	g_TotalBitNumPerFarme = g_r*g_InfoSymbolNumPerFrameMASS;
	g_InfoByteNumPerFrame = g_TotalBitNumPerFarme/8;
	g_ModuLen = g_TotalSymbolNumPerFrame*g_SpreadCodeLen*g_UpSamplingMultipleMASS;
	g_SigLen = g_ModuLen+g_ReceiveBlock*4;
	g_BitsDecoded = mymalloc(SRAMIN,g_MASSMaxLen);      
	memset(&g_BitsDecoded[0],0,g_MASSMaxLen);
}
/*
*********************************************************************************************************
*	函 数 名: SystemClock_Config
*	功能说明: 初始化系统时钟
*            	System Clock source            = PLL (HSE)
*            	SYSCLK(Hz)                     = 400000000 (CPU Clock)
*           	HCLK(Hz)                       = 200000000 (AXI and AHBs Clock)
*            	AHB Prescaler                  = 2
*            	D1 APB3 Prescaler              = 2 (APB3 Clock  100MHz)
*            	D2 APB1 Prescaler              = 2 (APB1 Clock  100MHz)
*            	D2 APB2 Prescaler              = 2 (APB2 Clock  100MHz)
*            	D3 APB4 Prescaler              = 2 (APB4 Clock  100MHz)
*            	HSE Frequency(Hz)              = 25000000
*           	PLL_M                          = 5
*            	PLL_N                          = 160
*            	PLL_P                          = 2
*            	PLL_Q                          = 4
*            	PLL_R                          = 2
*            	VDD(V)                         = 3.3
*            	Flash Latency(WS)              = 4
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	HAL_StatusTypeDef ret = HAL_OK;

	/* 锁住SCU(Supply configuration update) */
	MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

	/* 
      1、芯片内部的LDO稳压器输出的电压范围，可选VOS1，VOS2和VOS3，不同范围对应不同的Flash读速度，
         详情看参考手册的Table 12的表格。
      2、这里选择使用VOS1，电压范围1.15V - 1.26V。
    */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY));

	/* 使能HSE，并选择HSE作为PLL时钟源 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
		
	RCC_OscInitStruct.PLL.PLLM = 5;
	RCC_OscInitStruct.PLL.PLLN = 160;
	RCC_OscInitStruct.PLL.PLLP = 2;
	RCC_OscInitStruct.PLL.PLLR = 2;
	RCC_OscInitStruct.PLL.PLLQ = 4;		
		
	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;	
	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

	/* 
       选择PLL的输出作为系统时钟
       配置RCC_CLOCKTYPE_SYSCLK系统时钟
       配置RCC_CLOCKTYPE_HCLK 时钟，对应AHB1，AHB2，AHB3和AHB4总线
       配置RCC_CLOCKTYPE_PCLK1时钟，对应APB1总线
       配置RCC_CLOCKTYPE_PCLK2时钟，对应APB2总线
       配置RCC_CLOCKTYPE_D1PCLK1时钟，对应APB3总线
       配置RCC_CLOCKTYPE_D3PCLK1时钟，对应APB4总线     
    */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
	
	/* 此函数会更新SystemCoreClock，并重新配置HAL_InitTick */
	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
	if(ret != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}

  /* 使用IO的高速模式，要使能IO补偿，即调用下面三个函数 
    （1）使能CSI clock
    （2）使能SYSCFG clock
    （3）使能I/O补偿单元， 设置SYSCFG_CCCSR寄存器的bit0 */
  
	__HAL_RCC_CSI_ENABLE() ;

	__HAL_RCC_SYSCFG_CLK_ENABLE() ;

	HAL_EnableCompensationCell();

  /* AXI SRAM的时钟是上电自动使能的，而D2域的SRAM1，SRAM2和SRAM3要单独使能 */	
	__HAL_RCC_D2SRAM1_CLK_ENABLE();
	__HAL_RCC_D2SRAM2_CLK_ENABLE();
	__HAL_RCC_D2SRAM3_CLK_ENABLE();

}

/*
*********************************************************************************************************
*	函 数 名: Error_Handler
*	形    参: file : 源代码文件名称。关键字 __FILE__ 表示源代码文件名。
*			  line ：代码行号。关键字 __LINE__ 表示源代码行号
*	返 回 值: 无
*		Error_Handler(__FILE__, __LINE__);
*********************************************************************************************************
*/
void Error_Handler(char *file, uint32_t line)
{
	/* 
		用户可以添加自己的代码报告源代码文件名和代码行号，比如将错误文件和行号打印到串口
		printf("Wrong parameters value: file %s on line %d\r\n", file, line) 
	*/
	printf("Wrong parameters value: file %s on line %d\r\n", file, line);	
	/* 这是一个死循环，断言失败时程序会在此处死机，以便于用户查错 */
	if (line == 0)
	{
		return;
	}
	
  Soft_Reset();
//	while(1)
//	{
//	}
}
/*
*********************************************************************************************************
*	函 数 名: MPU_Config
*	功能说明: 配置MPU
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MPU_Config( void )
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* 禁止 MPU */
	HAL_MPU_Disable();

#if 1
   	/* 配置AXI SRAM的MPU属性为Write back, Read allocate，Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x24000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

 #else
	/* 配置AXI SRAM的MPU属性为NORMAL, NO Read allocate，NO Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x24000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif
	
	
	/* 配置FMC扩展IO的MPU属性为Device或者Strongly Ordered */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x60000000;
	MPU_InitStruct.Size             = ARM_MPU_REGION_SIZE_64KB;	
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;	/* 不能用MPU_ACCESS_CACHEABLE;会出现2次CS、WE信号 */
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER1;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
	/* 配置SDRAM的MPU属性为Write through, Read allocate，no Write allocate  1 0 0 */
	//  (原来的配置)   配置SDRAM的MPU属性为Write back, Read allocate，Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0xC0000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_32MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER2;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;//(原来是 MPU_TEX_LEVEL1)
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
	/* 配置SRAM1的属性为Write back, Read allocate，Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x30000000;
	MPU_InitStruct.Size             = ARM_MPU_REGION_SIZE_128KB;	
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER3;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* 配置SRAM2的属性为Write back, Read allocate，Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x30020000;
	MPU_InitStruct.Size             = ARM_MPU_REGION_SIZE_128KB;	
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER4;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);
	
	/* 配置SRAM3的属性为Write back, Read allocate，Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x30040000;
	MPU_InitStruct.Size             = ARM_MPU_REGION_SIZE_32KB;	
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER5;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* 配置SRAM4的MPU属性为Write through, Read allocate，no Write allocate  1 0 0 */
  /* (原来的配置) 配置SRAM4的MPU属性为Write back, Read allocate，Write allocate */
	MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress      = 0x38000000;
	MPU_InitStruct.Size             = MPU_REGION_SIZE_64KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number           = MPU_REGION_NUMBER6;
	MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
	
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
  
  //为了解决串口不定长数据接收与缓存之间的冲突
  MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
  MPU_InitStruct.Number           = MPU_REGION_NUMBER0;       //找一个空闲的用
  MPU_InitStruct.BaseAddress      = (uint32_t)RxBuff;         //DMA的目标地址，UART数组的指针
  MPU_InitStruct.Size             = MPU_REGION_SIZE_2KB;     //找打相应配置字，保护长度要大于数组长度
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE; //关闭这段内存的缓存功能，就不会有同步打架的问题了
  MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;
	
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/*使能 MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/*
*********************************************************************************************************
*	函 数 名: CPU_CACHE_Enable
*	功能说明: 使能L1 Cache
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void CPU_CACHE_Enable(void)
{
	/* 使能 I-Cache */
	SCB_EnableICache();

	/* 使能 D-Cache */
	SCB_EnableDCache();
  
//	SCB->CACR|=1<<2;   //强制D-Cache透写,如不开启,实际使用中可能遇到各种问题
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer10ms
*	功能说明: 该函数每隔10ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些处理时间要求不严格的
*			任务可以放在此函数。比如：按键扫描、蜂鸣器鸣叫控制等。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer10ms(void)
{
	bsp_KeyScan10ms();
}

/*
*********************************************************************************************************
*	函 数 名: bsp_RunPer1ms
*	功能说明: 该函数每隔1ms被Systick中断调用1次。详见 bsp_timer.c的定时中断服务程序。一些需要周期性处理的事务
*			 可以放在此函数。比如：触摸坐标扫描。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_RunPer1ms(void)
{

}

/*
*********************************************************************************************************
*	函 数 名: bsp_Idle
*	功能说明: 空闲时执行的函数。一般主程序在for和while循环程序体中需要插入 CPU_IDLE() 宏来调用本函数。
*			 本函数缺省为空操作。用户可以添加喂狗、设置CPU进入休眠模式的功能。
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_Idle(void)
{
	/* --- 喂狗 */

	/* --- 让CPU进入休眠，由Systick定时中断唤醒或者其他中断唤醒 */

	/* 例如 emWin 图形库，可以插入图形库需要的轮询函数 */
	//GUI_Exec();

	/* 例如 uIP 协议，可以插入uip轮询函数 */
	//TOUCH_CapScan();
}

/**********************************************************************************************************
*	函 数 名: HAL_Delay
*	功能说明: 重定向毫秒延迟函数。替换HAL中的函数。因为HAL中的缺省函数依赖于Systick中断，如果在USB、SD卡
*             中断中有延迟函数，则会锁死。也可以通过函数HAL_NVIC_SetPriority提升Systick中断
*	形    参: 无
*	返 回 值: 无
**********************************************************************************************************/

/* 当前例子使用stm32h7xx_hal.c默认方式实现，未使用下面重定向的函数 */
#if 1
void HAL_Delay(uint32_t Delay)
{
	bsp_DelayUS(Delay * 1000);
}
#endif


#if 0
static uint32_t fac_us = 0;

void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload = SysTick->LOAD; 	 
	ticks = nus*fac_us;
	told  = SysTick->VAL;
  
	while(1)
	{
		tnow = SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)
      {
        tcnt+=told-tnow;
      }
			else 
      {
        tcnt+=reload-tnow+told;	    
      }
      
			told=tnow;
			if(tcnt>=ticks)
      {
        break;
      }
		}  
	}
}
 
void delay_ms(uint16_t nms)
{
	uint32_t i;
  
	for(i=0;i<nms;i++) 
  {
    delay_us(1000);
  }
}

#endif
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
