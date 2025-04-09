/******************************************************************
Copyright (C), 2020-2030, Qingdao Shengwei marine Tech. Co., Ltd.
File name: 	   // Demo工程
Author: 	     // 会上树的猪
Version: 	     // V.1
Date: 	       // 2023/02/06
Description:   // 简版通信机驱动，无任何算法应用
History:       // V.0
*******************************************************************/
#ifndef _PARAMETER_H
#define _PARAMETER_H

#include "bsp.h"

/***************************************************************************************/

//#define g_FDE

//公共系统变量宏定义
#define g_PlvBase		  655
#define g_ADBit       4      //AD16位，该值为2，24或者32位，该值为4
#define g_DABit       2      //DA16位，该值为2
//MASS系统变量宏定义
#define g_Order		    192
#define g_Delay		    96
#define g_MASSMaxLen    256
//FDE系统变量宏定义
//#define g_FilterOrder	64
//#define g_FilterDelay	32
#define g_FDEMaxLen   1149

/**********************Turbo移植宏定义参数***********************/
#define	g_fs			      96000              //采样频率
#define	g_fl			      10000              //频带下限频率
#define	g_fh			      14000              //频带上限频率
#define	g_f0			      12000              //载波频率
#define g_fb			      4000               //通带带宽
#define g_LFMLen              6144               //LFM长度
#define g_DataBagLen          6144               //缓冲大小
#define g_Blank               6144             //保护延迟长度
#define g_FilterOrder	      32                 //升余弦滚降滤波器阶数
#define g_LowPassFilterOrder  32                 //低通滤波器阶数
#define g_FilterDelay	      16                 //滤波器延迟

//更改程序需要改变的2个变量
#define g_FrameBlockNum       17                 // [0.6*SigSynLFM Delay 0.6*SigSynLFM Delay SigMod]
#define g_SigSendLen          104447             //待发送信号长度

#define g_cplx                2                  //复数信号需要用两个数据表示
#define g_OK                  1
#define g_PI                  3.141592653589793
#define g_ReceiveBlock        6144
#define g_FrameByteNum        16
#define g_BuchangCont         6
/******************************结束*****************************/

//功能性变量定义
extern u8    g_DopplerEstimation;
extern u8    g_SL;
extern u16   g_f;
extern u8    g_save;
extern u32   g_TimeCnt;
extern u8    g_CommunicationMode;
//extern u8    g_IntMode;
//extern u8    g_SysMode;
extern u8    g_UUVControlBegin;
extern u8    g_ReceiveNum; 

//测距模式的标志及变量
extern u8    g_TDMaster;
extern u8    g_TDSlave;
extern u32   g_TDTimeStartSubS;
extern u8    g_TDTimeStartS;
extern u8    g_TDTimeStartM;
	
//接收模式的全局控制标志及变量
extern u16   g_FFTLen;
//extern u16   g_ReceiveBlock;
extern u8    g_1stSynIsOK;
extern u32   g_RecvBlockCnt;
extern u8	   g_DecodeEN;
extern u32   g_1stPoint;
extern float g_Peak;
extern float *g_1stCopy_HFM;


//发送模式的全局控制标志及变量
extern u32   g_PlvLevel;
extern u32   g_SendByteLen;

//存储模式及与存储相关的变量
extern volatile u8    g_StorageDataOK;	
extern volatile u32   g_SaveCnt;
extern volatile u16   g_StoreBlockNum;
extern s32   *g_StoreDataBuf;
extern u8    g_DeleteData;
extern u8    g_ReadLog;
extern u8    g_DeleteLog;



//定义结构体
typedef enum				 
{
	Transmit = 1,      //发射模式
	Receive	 = 2,      //接收模式
	Storage	 = 3,      //存储模式
	Debug	 = 4,      //调试模式
	TD       = 5,      //测距模式
	Idle     = 6,
	SaveTest = 7,
	TransmitTD = 8,      //发射模式
}Work_Mode;

typedef enum
{
	Ctrl = 0,         //控制指令
	Pass = 1,         //透传通信指令
}Protocol_Type;

//typedef enum				 
//{
//	MASS   = 1,      //MASS通信体制
//	FDE	   = 2,      //SC_FDE通信体制
//	SLTest = 3,    	 //声源级测试
//}Sys_Mode;

typedef struct _SYS_ANSWER_CMD
{
  u8 POWR_ANSWER[9];                //功率设置查询回复
  u8 PAMP_ANSWER[9];                //前放设置查询回复
	u8 SYN_ANSWER[9];                 //同步成功回复
	u8 DECODE_LOG_DELETE_ANSWER[9];   //解码日志删除回复	
	u8 RTC_ANSWER[15];                //RTC设置回复
	u8 DATA_SAVE_ANSWER[9];           //同步成功回复
	u8 DATA_SAVE_DELETE_ANSWER[9];    //解码日志删除回复
	u8 PARA_ANSWER[14];               //参数设置查询回复
	u8 DATA_LEN_ERROR[8];             //传输数据长度不匹配，错误
	u8 DATA_TRANSMIT_OVER[8];         //数据发送完毕
	u8 DATA_TRANSMIT_START[8];        //数据发送开始
	u8 INIT_SUCCESS[8];               //初始化成功
	u8 SYS_SET_ANSWER[9];
	u8 FRE_SET_ANSWER[9];
	u8 SAVE_SET_ANSWER[9];
	u8 POWER_SET_ANSWER[9];
	u8 DOPPLER_SET_ANSWER[9];
  u8 TD_MASTER_SEND_ANSWER[8];
	u8 TD_MASTER_ANSWER[10];
	u8 TD_SLAVE_ANSWER[8];
	u8 UUV_CONTROL_BEGIN_ANSWER[9];
	u8 VSOUND_SET_ANSWER[8];
	u8 TDTime_SET_ANSWER[8];
	u8 BuChang_SET_ANSWER[8];
}SYS_ANSWER_CMD;

typedef struct _SYS_CMD
{
	u8 POWR_SET[1];         //功率挡位设置
  u8 PAMP_SET[1];         //前放设置
  u8 POWR_CHECK[1];       //功率查询
  u8 PAMP_CHECK[1];       //前放查询
	u8 DECODE_LOG_READ[1];  //解码日志查询
	u8 DECODE_LOG_DELETE[1];//解码日志删除
	u8 RTC_SET[1];          //RTC设置
  u8 DATA_SAVE[1];        //数据存储
	u8 DATA_SAVE_DELETE[1]; //数据存储删除
	u8 PARA_SET[1];         //参数设置
	u8 PARA_CHECK[1];       //参数查询
	u8 SYS_SET[1];          //通信体制设置
	u8 FRE_SET[1];          //声源级测试频点设置
	u8 SAVE_SET[1];         //同步后整帧信号存储设置
	u8 POWER_SET[1];        //任意功率大小设置
	u8 DOPPLER_SET[1];      //多普勒估计补偿设置
	u8 TD[1];               //控制发起测距功能
	u8 UUV_CONTROL_BEGIN[1];//uuv决定是否可以开始通信设置
	u8 SOFTRESET[1];        //软复位指令
	u8 TDTime_SET[1];
	u8 BuChang_SET[1];
}SYS_CMD;

typedef struct _UART_Protocol
{
  u8 Preamble[4]; //前导码
  u8 End_flag[2]; //结束标志
}UART_Protocol;

extern SYS_CMD SysCmd;
extern SYS_ANSWER_CMD SysAnswerCmd;
extern UART_Protocol STM32UartProtocol;



/**********************Turbo移植参数***********************/

extern  volatile char       flag;
extern volatile uint8_t     g_SaveOK;

extern uint32_t    g_TransCnt;
extern uint32_t    g_C_TransCnt;
//extern u32         g_SaveCnt;
extern uint8_t    *g_MCU_ADC_Buf1;
extern uint16_t   *g_MCU_ADC_Buf2;
extern int32_t    *g_ADC_Buf1;
extern int32_t    *g_ADC_Buf2;
extern uint8_t     g_RecvBuf1IsOK;
extern uint8_t     g_RecvBuf2IsOK;
extern uint8_t     g_CommunicationMode;
extern float      *Sig;
//extern float *Sig_10kHz;
extern u32         g_PowrLevel;


// 2023/5/25添加
extern char        g_WorkMode;
extern char        g_IntMode;
extern uint8_t     g_SynOK;
extern uint8_t     g_FirstSynOK;
extern uint8_t     g_SecondSynOK;
extern uint32_t    g_PinYi;
extern float       g_Doppler;
extern s32         g_Vrealitive;
extern uint32_t    g_Slide;
extern uint8_t     g_RxCnt;
extern uint8_t     g_TxCnt;
extern uint32_t    g_Pose;
extern uint32_t    g_1stPose;
extern uint32_t    g_2stPose;
extern uint32_t    g_SynPose;
extern uint8_t     g_SendOK;

extern uint32_t    g_RxNum;
extern uint8_t    *g_RxBuff;
extern u8         *g_uChar;
extern char       *g_Info;
extern char       *g_InfoDecode;
extern uint8_t    *USART_RX_BUF;
extern uint8_t    *g_CacheA;
extern s32        *g_CacheB;
extern s32        *g_SD_WriteBuff;
extern char       *g_SD_TxtName;
extern uint16_t   *g_InterleaveSD;
extern float      *g_Cachef32;
extern float      *g_Backup;
extern float      *g_FrameSig;
extern float      *g_SigBuchang;
//extern float      *g_LFM;
//extern float      *g_HFM;
//extern float      *g_SynRefConj;
extern float      *g_HFMRefConj;
extern float      *g_LFMRefConj;

extern float       g_CosineF0[8];
extern float       g_g_NegCosineF0[8];
extern float       g_SineF0[8];
extern float       g_NegSineF0[8];
extern float       g_LowpassFirCoeffs[g_LowPassFilterOrder+1];
extern float       g_RcosCoeffs[g_FilterOrder+1];
extern float *g_SynHFM;

//测距模式的标志及变量
extern u8    g_TDMaster;
extern u8    g_TDSlave;
extern u8    g_TDSlaveNum;
extern u32   g_TDPose;
extern u8    g_TDSuccess;
extern u16   g_VSound;
extern u16   g_TDTime;
extern u32   g_BuChang;
extern u8    g_TD;
extern u16   *g_uShortTD;
extern u8    *g_uCharTD;
extern float *g_Sig;
extern float *g_LFM;
/******************************结束*****************************/

extern u8    g_SpreadCodeLen;
extern float g_alpha_filter;
extern u8    g_M;
extern u8    g_InfoSymbolNumPerFrameMASS;
extern u8    g_PolitSymbolNumPerFrame;
extern u8    g_SlidingInterval;
extern char  g_SpreadCode[2048];
extern float g_PulseShape[289];
extern u8    g_UpSamplingMultipleMASS;
extern u8    g_TotalSymbolNumPerFrame;
extern u8    g_r;
extern u32   g_ReceiveBlockNumber;
extern u16   g_TotalBitNumPerFarme;
extern u16   g_InfoByteNumPerFrame;
extern u32   g_ModuLen;
extern u32   g_SigLen;
extern u8 	 *g_BitsDecoded;
extern float CarryCos[8];
extern float CarrySin[8];
extern float g_LowPassCoeffDemodu[2*g_Delay+1];

#endif
/***************************** 声威海洋科技 (END OF FILE) *********************************/
