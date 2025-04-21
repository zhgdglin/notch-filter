/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "mdma.h"
#include "sdmmc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "APP_Work.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t failure_cnt = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */
	

		
	MX_GPIO_Init();    
	HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);  // ADC数据关闭，在adc7767_init中开启
	MX_USART1_UART_Init();
	
	

	if( 1 == READ_BIT(PWR -> WKUPFR, PWR_WKUPFR_WKUPF1))  // 读取唤醒标志寄存器，判断是否为唤醒状态   //  PWR_WKUPFR
			System_State = Wake_Up;			
	
	printf(" System_State = %s \r\n", SystemTable[System_State]); 
	

	/*  判断唤醒信号有效性 （没有开启中断，需要和甲板联合测试）*/
	if(System_State == Wake_Up)
	{
		MX_TIM7_Init();
		__HAL_TIM_ENABLE(&htim7);    // 开启计数
		printf(" 开始计时1S \r\n");
		
		while(1)  
		{
				if(TIM7->CNT == 10000 - 1)     // 计数值满，倒计时1S到达
				{  
						__HAL_TIM_DISABLE(&htim7); 
						TIM7->CNT = 0;

						printf(" 计时1S结束，开始判断信号 \r\n");
					
						if( Wakeup_signal_cnt/2 <= Wakeup_signal_Low_cnt ){   // 超过一半的时间为0
							
								printf(" 信号有效 \r\n");
								printf(" Wakeup_signal_cnt =  %d\r\n Wakeup_signal_Low_cnt = %d \r\n",Wakeup_signal_cnt,Wakeup_signal_Low_cnt);
								System_State = Wake_Work;
								printf(" 唤醒成功 \r\n");
								break;   // 跳出循环
							
							} else{
												printf(" 信号无效，重新开始计数 \r\n");
												TIM7->CNT = 0;
												__HAL_TIM_ENABLE(&htim7); 
												failure_cnt++;
												if(failure_cnt++ > 6)   // 失败5次
												{	
													  failure_cnt = 0;
														printf(" 唤醒失败多次，继续休眠 \r\n");
														Enter_Standby_Mode();
												}
										}
							
						Wakeup_signal_cnt = 0;
						Wakeup_signal_Low_cnt = 0;
			
				}	else { 													 // 如果未到1S
									Wakeup_signal_cnt++;
									if( (( GPIOA -> IDR ) & 0x00000001) == 0x00000000)  // 判断PA0是否为0
									Wakeup_signal_Low_cnt++;
								}
		} // while(1)
	}

		
	printf(" 其余外设开启 \r\n");
	printf(" System_State = %s \r\n", SystemTable[System_State]); 
	

	
  Power_ON();      // 打开板子的所有电源控制
  HAL_Delay(100);  // 为稳定开启
	
  MX_DMA_Init();  // SPI 
  MX_MDMA_Init();
  MX_SDMMC1_SD_Init();
  MX_TIM6_Init();  // us级延时时钟
  MX_ADC3_Init();	// 电池电压
  MX_I2C1_Init(); // MPU6050
  MX_SPI1_Init();	// ADC时序
  MX_USART3_UART_Init();  //RS232
  MX_FATFS_Init();
  MX_TIM12_Init(); // ADC时钟 1M
  MX_TIM2_Init();   // 5分钟定时，中断14
//	MOTOR_REVERSE;   // 电机正反转选择，必须在时钟前
//	//MOTOR_FORWARD;
	MX_TIM13_Init(); // 电机，中断10   放在后面的 Motor_rotate 中
	MX_TIM7_Init();  // 1S定时器，中断15
	
	__HAL_TIM_CLEAR_IT (&htim7 ,TIM_IT_UPDATE );        // 清除T7标志
	__HAL_TIM_SET_COUNTER(&htim7, 0);   								// 清零T7计数
	HAL_TIM_Base_Start_IT (&htim7 );                    // 允许T7中断

  MX_TIM4_Init();  // 发射

		/* 开启5分钟定时，如果没有收到指令，则进入休眠 */
		Minute_5_start = 1;
		__HAL_TIM_CLEAR_IT (&htim2 ,TIM_IT_UPDATE );        // 清除T2标志
		HAL_TIM_Base_Start_IT (&htim2 );                    // 允许T2中断
		
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
 
  /* USER CODE BEGIN 2 */

  printf("\r\n 初始化完成\r\n");
	printf("****进入用户程序****\r\n\r\n");
  APP_Init();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		APP_Process();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 80;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 20;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_ADC|RCC_PERIPHCLK_SDMMC;
  PeriphClkInitStruct.PLL2.PLL2M = 4;
  PeriphClkInitStruct.PLL2.PLL2N = 50;
  PeriphClkInitStruct.PLL2.PLL2P = 5;
  PeriphClkInitStruct.PLL2.PLL2Q = 2;
  PeriphClkInitStruct.PLL2.PLL2R = 6;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL2;
  PeriphClkInitStruct.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();
  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x24000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
  MPU_InitStruct.SubRegionDisable = 0x0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

