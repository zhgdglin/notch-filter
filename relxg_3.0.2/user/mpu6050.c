/**
  ******************************************************************************
  * @file    bsp_mpu6050.c
  * @author  STMicroelectronics
  * @version V1.0
  * @date    2013-xx-xx
  * @brief    mpu6050驱动
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 霸道 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 

#include "mpu6050.h"
#include "usart.h"
#include "i2c.h"
#include <math.h>   // 平方根 sqrt()   反正切 atan()

/* MPU6050数据 */
short Acel[3];
short Gyro[3];
float Temp;




/**
  * @brief   写一个字节到I2C设备中
  * @param   
  *		@arg pBuffer:缓冲区指针
  *		@arg WriteAddr:写地址 
  * @retval  正常返回1，异常返回0
  */
uint8_t I2C_ByteWrite(uint8_t pBuffer, uint8_t WriteAddr)
{
  HAL_StatusTypeDef hal_status = HAL_I2C_Mem_Write(&hi2c1,MPU6050_SLAVE_ADDRESS,WriteAddr,sizeof(uint8_t),&pBuffer,sizeof(uint8_t),1000);
  if(hal_status==HAL_OK)
    return 1; //正常返回1
  else
    return 0; //错误返回0
}



/**
  * @brief   从I2C设备里面读取一块数据 
  * @param   
  *		@arg pBuffer:存放从slave读取的数据的缓冲区指针
  *		@arg WriteAddr:接收数据的从设备的地址
  *     @arg NumByteToWrite:要从从设备读取的字节数
  * @retval  正常返回1，异常返回0
  */
uint8_t I2C_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{ 
  HAL_StatusTypeDef hal_status;
  for(int i=0;i<NumByteToRead;i++)
  {
    hal_status = HAL_I2C_Mem_Read(&hi2c1,MPU6050_SLAVE_ADDRESS,ReadAddr+i,sizeof(uint8_t),pBuffer+i,sizeof(uint8_t),1000);
  }
  
  
  if(hal_status==HAL_OK)
    return 1; //正常返回1
  else
    return 0; //错误返回0
}




/**
  * @brief   写数据到MPU6050寄存器
  * @param   
  * @retval  
  */
void MPU6050_WriteReg(uint8_t reg_add,uint8_t reg_dat)
{
	I2C_ByteWrite(reg_dat,reg_add); 
}

/**
  * @brief   从MPU6050寄存器读取数据
  * @param   
  * @retval  
  */
void MPU6050_ReadData(uint8_t reg_add,unsigned char* Read,uint8_t num)
{
  
	I2C_BufferRead(Read,reg_add,num);
}



/*****************  应用层 ***********************/

uint8_t MPU6050ReadID(void)
{
	unsigned char Re = 0;
    MPU6050_ReadData(MPU6050_RA_WHO_AM_I,&Re,1);    //读器件地址
	if(Re != 0x68)
		return 0;
		
		printf("MPU6050正常工作 ");
		printf(" ID = %x\r\n", Re);
		return 1;
	
		
}

/**
  * @brief   读取MPU6050的加速度数据
  * @param   
  * @retval  
  */
void MPU6050ReadAcc(short *accData)
{
    uint8_t buf[6];
    MPU6050_ReadData(MPU6050_ACC_OUT, buf, 6);
    accData[0] = (buf[0] << 8) | buf[1];
    accData[1] = (buf[2] << 8) | buf[3];
    accData[2] = (buf[4] << 8) | buf[5];
}

/**
  * @brief   读取MPU6050的角加速度数据
  * @param   
  * @retval  
  */
void MPU6050ReadGyro(short *gyroData)
{
    uint8_t buf[6];
    MPU6050_ReadData(MPU6050_GYRO_OUT,buf,6);
    gyroData[0] = (buf[0] << 8) | buf[1];
    gyroData[1] = (buf[2] << 8) | buf[3];
    gyroData[2] = (buf[4] << 8) | buf[5];
}

/**
  * @brief   读取MPU6050的原始温度数据
  * @param   
  * @retval  
  */
void MPU6050ReadTemp(short *tempData)
{
	uint8_t buf[2];
    MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //读取温度值
    *tempData = (buf[0] << 8) | buf[1];
}

/**
  * @brief   读取MPU6050的温度数据，转化成摄氏度
  * @param   
  * @retval  
  */
void MPU6050_ReturnTemp(float *Temperature)
{
	short temp3;
	uint8_t buf[2];
	
	MPU6050_ReadData(MPU6050_RA_TEMP_OUT_H,buf,2);     //读取温度值
  temp3= (buf[0] << 8) | buf[1];	
	*Temperature=((double) temp3/340.0)+36.53;
}

///*一阶补偿*/
//float Angle_offset(float acx1,float gyroy2)
//{
//	float hk;
//	hk=-100.0f/(8192-100)*(acx1-100)+100;
//	//MPU_Get_Raw_data(&aacx,&aacy,&aacz,&gyrox,&gyroy,&gyroz);
//	Angle_ax = asin((acx1-hk) /8192);   //去除零点偏移,计算得到角度（弧度）
//	Angle_ax = Angle_ax*180/3.14f;     //弧度转换为度
//	gyroy=-1*(gyroy2+3)/16.4f; 
//	//if(abs(gyroy)<1.2)
//	Angle1=0.95f*Angle_ax+0.05f*(Angle1 + gyroy*0.01);
//	/*else
//	Angle1=0.9f*Angle_ax+0.1f*(Angle1 + gyroy*0.01);	*/
//	if(Angle1>=90)
//		Angle1=90;
//	else if(Angle1<=-90)
//		Angle1=-90;
//	pitIsrCnt1=true;
//	return Angle1;
//}


float Read_mpu6050(void)
{
  float ax,ay,az;
	float pitch;
	
		MPU6050ReadAcc(Acel);
		MPU6050ReadGyro(Gyro);
		MPU6050_ReturnTemp(&Temp);
	
		ax=(float)Acel[0];
		ay=(float)Acel[1];
		az=(float)Acel[2];
	
		
		pitch = -atan(ax/az)*57.2957f;  
		
		printf("		俯仰pitch: %.2f \r\n" , pitch);  

  return pitch;
}





/**
  * @brief   初始化MPU6050芯片
  * @param   
  * @retval  
  */
extern int led2_time;
void MPU6050_Init(void)
{

	HAL_Delay(200);
  MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x00);	     //解除休眠状态
  MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV , 0x07);	    //陀螺仪采样率
  MPU6050_WriteReg(MPU6050_RA_CONFIG , 0x06);	
  MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG , 0x01);	  //配置加速度传感器工作在2G模式
  MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18);     //陀螺仪不自检，2000deg/s, 对应16.4
	
  HAL_Delay(200);
  if( MPU6050ReadID() == 0 )
  {
    printf("\r\n没有检测到MPU6050传感器！\r\n");
  }
  
  
}
