#ifndef __PARAMETER_H
#define __PARAMETER_H
#include "system.h"

//系统变量宏定义
#define g_Order		192
#define g_Delay		96
#define g_PlvBase	654
#define g_DeBug   0

//算法初始变量
extern u16 g_fl;
extern u16 g_fh;
extern u32 g_fs;
extern float g_SynTime;
extern u8  g_alpha;
extern u8  g_MaxByteLen;
extern u8  g_MOrder;
extern u8  g_DopplerEstimation;
extern u8  g_DownSamplingNum;

//根据初始变量计算出的中间变量
extern u16 g_f0;
extern u16 g_fb;
extern u8 g_Nsap;
extern u16 g_ReceiveBlock;
extern u16 g_TransmitBlock;
extern u16 g_FFTLen;
extern u32 g_ReceiveBlockNumber;
extern u32 g_TransmitBlockNumber;
extern u32 g_ModuLen;
extern u32 g_SigLen;
extern u8 g_save;

//控制接收、发送与存储的变量
extern u8  g_IntMode;
extern u8  g_1stSynIsOK;
extern u32 g_RecvCnt;
extern u32 g_TmitCnt;
extern u32 g_PlvLevel;
extern u32 g_UartHEXLen;
extern u8  g_SendByteLen;
extern u8	 g_DecodeEN;
extern u8  g_CommunicationMode;
extern u32 g_1stPoint;
extern u8  g_StorageDataOK;	
extern u32 g_SaveCnt;

//接收、发送和存储过程中的数组
extern u8    *g_CacheA;
extern s32	 *g_CacheB;
extern float *g_Cachef32;
extern float *g_SynHFM;
extern float *g_1stCopy;
extern float *g_2ndCopy;
//extern float *g_SynLFM;
//extern float *g_1stCopyLFM;
extern float *g_SigBlk;
extern float *g_SigMix;
extern float *g_BagBlk;
extern float *g_SynBlk;
extern float *g_SigBaseDown;
extern float *g_BagTmpBlk;
extern float *g_BackUp;
extern u8 	 *uChar;
extern s32   *g_StoreDataBuf;

//算法中MATLAB产生的固定数组
extern char  g_MCodeInital[12];
extern char  g_FeedFinal[12];
//extern float PN1_Freq[128*2];
//extern float PN2_Freq[128*2];
extern float g_BandPassCoeff[65];
extern float g_LowPassCoeff[2*g_Delay+1];

//定义结构体
typedef enum				 
{
	Transmit = 1,      //发射模式
	Receive	 = 2,      //接收模式
	Storage	 = 3,      //存储模式
	Debug	   = 4,      //调试模式
}Work_Mode;

typedef enum
{
	Ctrl = 0,         //控制类型
	Pass = 1,         //透传类型
}Protocol_Type;

typedef struct _SYS_ANSWER_CMD
{
  u8 POWR_ANSWER[9];                //功率设置查询回复
  u8 PAMP_ANSWER[9];                //前放设置查询回复
	u8 SYN_ANSWER[9];                 //同步成功回复
	u8 DECODE_LOG_DELETE_ANSWER[9];   //解码日志删除回复	
	u8 RTC_ANSWER[14];                //RTC设置回复
	u8 DATA_SAVE_ANSWER[9];           //同步成功回复
	u8 DATA_SAVE_DELETE_ANSWER[9];    //解码日志删除回复
	u8 PARA_ANSWER[14];               //参数设置查询回复
	u8 DATA_LEN_ERROR[8];             //传输数据长度不匹配，错误
	u8 DATA_TRANSMIT_OVER[8];         //数据发送完毕
	u8 INIT_SUCCESS[8];               //初始化成功
	u8 SAVE_SET_ANSWER[9];
	u8 POWER_SET_ANSWER[9];
	u8 DOPPLER_SET_ANSWER[9];
}SYS_ANSWER_CMD;

typedef struct _SYS_CMD
{
	u8 POWR_SET[1];         //功率设置
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
	u8 SAVE_SET[1];         //存储设置
	u8 POWER_SET[1];        //功率设置
	u8 DOPPLER_SET[1];      //多普勒设置
}SYS_CMD;

extern SYS_CMD SysCmd;
extern SYS_ANSWER_CMD SysAnswerCmd;
#endif /*__PARAMETER_H*/




