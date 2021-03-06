/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : PC001V21.c
* Author             : WOW
* Version            : V2.0.1
* Date               : 06/26/2017
* Description        : PC001V21层控制板.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


//SW6 ON表示与旋转相关的板
//SW5 ON表示旋转电机控制板，OFF表示传感器板

//CAN滤波器组使用 0x0A
//传感器板CAN-ID为0x80
//旋转电机控制板CAN-ID为0xC0

//旋转电机控制板对外接口
//CAN滤波器组0x09
//CAN-ID为0xCC
#define	Cmd_Group	0x09
#define	Cmd_ID		0xCC

//其它数据通讯端口
//CAN滤波器组 0x08
//CAN-ID为拔码地址
#define	DATA_Group	0x08
#define	DATA_ID			0xFA

#ifdef PC006V21			//分检机控制板

#include "PC006V21.H"

#include	"stdio.h"			//用于printf
#include	"string.h"		//用于printf
#include	"stdarg.h"		//用于获取不确定个数的参数
#include	"stdlib.h"		//malloc动态申请内存空间



#include "SWITCHID.H"
#include "STM32_WDG.H"
#include "STM32_EXTI.H"
#include "STM32_GPIO.H"
#include "STM32_PWM.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_CAN.H"
#include "STM32F10x_BitBand.H"

//=================PA6电机控制板CCW做脉冲输出
#define	MOTOR_PWM_PORT	GPIOA
#define	MOTOR_PWM_Pin		GPIO_Pin_7
//=================PA7电机控制板CW做方向输出
#define	MOTOR_DIR_PORT	GPIOA
#define	MOTOR_DIR_Pin		GPIO_Pin_6

//=================PA6传感器板CCW做脉冲输出
#define	TXLED_PORT	GPIOA
#define	TXLED_Pin		GPIO_Pin_7
//=================PA7传感器板CW做方向输出
#define	RXLED_PORT	GPIOA
#define	RXLED_Pin		GPIO_Pin_6

#define	MOTOR_PWM_Frequency	500	//脉冲频率--单位Hz
#define	PWM_UpdataCount	10					//频率变化占计数个数，就是计数PWM_Updata个数后更新一次输出频率
#define	PWM_RunUpCount	1000				//加速/减速需要的步数
#define	MOTOR_TIMx	TIM1				//旋转电机所使用的定时器

#define	Steeps		4000					//一个窗口用到的驱动脉冲数
#define	MaxWindow	4							//最大窗口数

#define	MOTOR_RunRight			MOTOR_DIR_PORT->BSRR 	= MOTOR_DIR_Pin		//EN	=	1;	//顺时针
#define	MOTOR_RunLeft				MOTOR_DIR_PORT->BRR		= MOTOR_DIR_Pin		//EN	=	0;	//逆时钟

//=================传感器
#define	Sensor1_Port	GPIOB		//前传感器
#define	Sensor1_Pin		GPIO_Pin_4

#define	Sensor2_Port	GPIOB		//后传感器
#define	Sensor2_Pin		GPIO_Pin_5

#define	Sensor3_Port	GPIOB		//药品传感器
#define	Sensor3_Pin		GPIO_Pin_6

#define	Sensor4_Port	GPIOB		//备用传感器
#define	Sensor4_Pin		GPIO_Pin_7

//======传感器状态：：读取状态为高表示无信号，低-有信号
#define	Sensor1_Status	(Sensor1_Port->IDR & Sensor1_Pin)
#define	Sensor2_Status	(Sensor2_Port->IDR & Sensor2_Pin)
#define	Sensor3_Status	(Sensor3_Port->IDR & Sensor3_Pin)
#define	Sensor4_Status	(Sensor4_Port->IDR & Sensor4_Pin)

SWITCHID_CONF SWITCHID;					//拔码开关结构体
PWM_TimDef		PWM_Tim;					//PWM控制脉冲结构体

u8 SwitchData	=	0;	//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
u8 SensorBD	=	0;		//传感器板标志，0-非传感器板，1-传感器板
u8 MotorBD	=	0;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
u32	MotorTime	=	0;	//旋转电机测试运行时间

u8	CANID	=	0;
u16 SYSTime=0;
//u16 data=0;
//u8 Flag=0;
u8 Sensor[4]	=	{0};	//4个传感器感应值存储区
u8 SensorON		=	0	;		//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报

u8 RunToWindow	=	0;		//旋转到相应窗口 0-无，1-1号，2-2号，3-3号，4-4号
u8 StatusOfWindow	=	0;	//当时停止位置：0-未初始化，1-原点，2-2号窗口，3-3号窗口，4-4号窗口
u8 CMDOfWindow	=	0;		//运行到指定窗口命令



CanRxMsg RxMessage;			//CAN接收 
CanTxMsg TxMessage;			//CAN发送




void SWITCHID_Configuration(void);			//拔码开关初始化及读数
void Motor_Configuration(void);					//步进电机驱动配置
void Sensor_Configuration(void);				//传感器配置
void CAN_Configuration(void);						//CAN配置
void LED_Configuration(void);						//作为传感器板时，发送LED指示灯


void Motor_RunSet(int Num);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
void Sensor_Read(void);					//读传感器信号	
void Reset_Data(void);					//复位所有的全局变量值
u8 SetToWindows(u8 nWindows);		//运行到指定窗口	

void SensorBD_Server(void);		//传感器板服务程序
void MotorBD_Server(void);		//电机控制板服务程序
u8 Motor_Server(void);				//步进电机返回状态
void CAN_Server(void);			//CAN收发数据管理
void Switch_Server(void);			//检查拔码地址有无变更，如果变更，重新配置运行参数
/*******************************************************************************
*函数名		:	function
*功能描述	:	函数功能说明
*输入			: 
*输出			:	无
*返回值		:	无
*例程			:
*******************************************************************************/
void PC006V21_Configuration(void)
{
	SYS_Configuration();											//系统配置 STM32_SYS.H	
	SysTick_DeleymS(100);											//SysTick延时nmS
	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);			//SYS-LED 1HZ 50%
	SWITCHID_Configuration();									//拔码开关初始化及读数
	
	if(SensorBD)
	{
		Sensor_Configuration();				//传感器配置
		SysTick_Configuration(100);		//系统嘀嗒时钟配置72MHz,单位为uS
		LED_Configuration();					//作为传感器板时，发送LED指示灯
	}
	else if(MotorBD)
	{
		SysTick_Configuration(1000);							//系统嘀嗒时钟配置72MHz,单位为uS
		Motor_Configuration();										//步进电机驱动配置
	}
	
	CAN_Configuration();							//CAN1配置---标志位查询方式，不开中断--500K

//	IWDG_Configuration(1000);	//独立看门狗配置	Tout-超时复位时间，单位ms
	
}

/*******************************************************************************
*函数名		:	function
*功能描述	:	函数功能说明
*输入			: 
*输出			:	无
*返回值		:	无
*例程			:
*******************************************************************************/
void PC006V21_Server(void)
{
	if(SensorBD)
	{
		SensorBD_Server();		//传感器板服务程序
	}	
	else if(MotorBD)
	{
		MotorBD_Server();		//电机控制板服务程序
	}
	Switch_Server();			//检查拔码地址有无变更，如果变更，重新配置运行参数
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void SensorBD_Server(void)		//传感器板服务程序
{
	Sensor_Read();		//读传感器信号
	if(SensorON	&&	SYSTime%10==0)			//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报
	{
		CAN_StdTX_DATA(CANID,4,Sensor);			//CAN使用标准帧发送数据
	}
	
	SYSTime++;
	if(SYSTime>=100000)
	{		
		SYSTime=0;
	}
	//===============传感器板在上传传感器数据时LED闪烁
	if(SensorON)
	{
		if(SYSTime%1000==0)
		{
			TXLED_PORT->BSRR=TXLED_Pin;		//PA6传感器板CCW做脉冲输出---LED_ON
			RXLED_PORT->BSRR=RXLED_Pin;		//PA7传感器板CW做方向输出---LED_ON
		}
		else	if(SYSTime%1000==200)
		{
			TXLED_PORT->BRR=TXLED_Pin;		//PA6传感器板CCW做脉冲输出---LED_OFF
			RXLED_PORT->BRR=RXLED_Pin;		//PA7传感器板CW做方向输出---LED_OFF
		}
	}
	else
	{
		TXLED_PORT->BRR=TXLED_Pin;		//PA6传感器板CCW做脉冲输出---LED_OFF
		RXLED_PORT->BRR=RXLED_Pin;		//PA7传感器板CW做方向输出---LED_OFF
	}		
	IWDG_Feed();				//独立看门狗喂狗
	CAN_Server();				//CAN收发数据管理
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void MotorBD_Server(void)		//电机控制板服务程序
{
	if(Motor_Server())					//步进电机返回状态---为定时器中断
	{
		return;
	}
	if(SYSTime==0	)
	{
//			memset(Sensor,0x08,4);
		Sensor[0]	=	0xAF;
		Sensor[1]	=	0x01;
		SensorON	=	1;
		CAN_StdTX_DATA(CANID,2,Sensor);				//CAN使用标准帧发送数据
		memset(Sensor,0x00,2);
	}
	else if(SYSTime==2000)
	{
//			Sensor[0]	=	0xAF;
//			Sensor[1]	=	0x00;
//			SensorON	=	0;
//			CAN_StdTX_DATA(CANID,2,Sensor);				//CAN使用标准帧发送数据
//			memset(Sensor,0x00,2);
	}	
	SYSTime++;
	if(SYSTime>=4000)
	{		
		SYSTime=0;
	}
	
	IWDG_Feed();				//独立看门狗喂狗
	CAN_Server();				//CAN收发数据管理
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void LED_Configuration(void)	//作为传感器板时，发送LED指示灯
{
//	//=================PA6传感器板CCW做脉冲输出
//#define	TXLED_PORT	GPIOA
//#define	TXLED_Pin		GPIO_Pin_7
////=================PA7传感器板CW做方向输出
//#define	RXLED_PORT	GPIOA
//#define	RXLED_Pin		GPIO_Pin_6
	GPIO_Configuration_OPP2	(TXLED_PORT,TXLED_Pin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP2	(RXLED_PORT,RXLED_Pin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
}

/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void CAN_Configuration(void)		//CAN配置
{
	//SW6 ON表示与旋转相关的板
	//SW5 ON表示旋转电机控制板，OFF表示传感器板
	//CAN滤波器组使用 0x0A
	//传感器板CAN-ID为0x80
	//旋转电机控制板CAN-ID为0xC0
	
	CAN_Configuration_NR(500000);							//CAN1配置---标志位查询方式，不开中断--500K
	if((SWITCHID.nSWITCHID	&	0x20)	==	0x20)
	{
		if((SWITCHID.nSWITCHID	&	0x30)	==	0x30)	//电机控制板
		{
			CANID	=	0xC0;
		}
		else
		{
			CANID	=	0x80;			
		}
		//======旋转电机控制板与传感器板通讯端口滤波器-----旋转电机控制板与旋转传感器通讯滤波器组
		CAN_FilterInitConfiguration_StdData(0X0A,CANID,0xFFBF);			//CAN滤波器配置---标准数据帧模式---BIT7不过滤
		//======旋转电机控制板与外部通讯通讯端口滤波器
		if(MotorBD)	//旋转电机控制板标志，0-非电机控制板，1-电机控制板
		{
			CAN_FilterInitConfiguration_StdData(Cmd_Group,	Cmd_ID,	0xFFFF);			//CAN滤波器配置---标准数据帧模式---电机控制板对外控制接口
			CAN_FilterInitConfiguration_StdData(DATA_Group,	DATA_ID,0xFFFF);			//CAN滤波器配置---标准数据帧模式---电机控制板对外数据接口			
		}
	}
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void SWITCHID_Configuration(void)			//拔码开关初始化及读数
{
	//SW6 ON表示与旋转相关的板
	//SW5 ON表示旋转电机控制板，OFF表示传感器板
	//CAN滤波器组使用 0x0A
	//传感器板CAN-ID为0x80
	//旋转电机控制板CAN-ID为0xC0
	//	u8 MotorBD	=	0;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
	//	u8 SensorBD	=	0;		//传感器板标志，0-非传感器板，1-传感器板
	
	SWITCHID.NumOfSW	=	6;
	//SW1--PA4
	SWITCHID.SW1_PORT	=	GPIOA;
	SWITCHID.SW1_Pin	=	GPIO_Pin_4;
	//SW2--PA5
	SWITCHID.SW2_PORT	=	GPIOA;
	SWITCHID.SW2_Pin	=	GPIO_Pin_5;
	//SW3--PB12
	SWITCHID.SW3_PORT	=	GPIOB;
	SWITCHID.SW3_Pin	=	GPIO_Pin_12;
	//SW4--PB13
	SWITCHID.SW4_PORT	=	GPIOB;
	SWITCHID.SW4_Pin	=	GPIO_Pin_13;
	//SW5--PB14
	SWITCHID.SW5_PORT	=	GPIOB;
	SWITCHID.SW5_Pin	=	GPIO_Pin_14;
	//SW6--PB15
	SWITCHID.SW6_PORT	=	GPIOB;
	SWITCHID.SW6_Pin	=	GPIO_Pin_15;
	
	SWITCHID_Conf(&SWITCHID);		//
	SWITCHID_Read(&SWITCHID);		//
	
	if((SWITCHID.nSWITCHID	&	0x20)	==	0x20)
	{
		if((SWITCHID.nSWITCHID	&	0x30)	==	0x30)	//电机控制板
		{
			MotorBD		=	1;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
		}
		else
		{
			SensorBD	=	1;		//传感器板标志，0-非传感器板，1-传感器板		
		}
	}
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void Motor_Configuration(void)			//步进电机驱动配置
{
	//======PWM输出
	PWM_Tim.PWM_BasicData.GPIOx	=	MOTOR_PWM_PORT;
	PWM_Tim.PWM_BasicData.GPIO_Pin_n	=	MOTOR_PWM_Pin;
	PWM_Tim.PWM_BasicData.PWM_Frequency	=	MOTOR_PWM_Frequency;	//频率 最小频率0.2Hz
	PWM_Tim.PWM_BasicData.PWM_Updata	=	PWM_UpdataCount;		//频率变化占计数个数，就是计数PWM_Updata个数后更新一次输出频率
	PWM_Tim.PWM_BasicData.PWM_RunUp	=	PWM_RunUpCount;				//加速/减速需要的步数
	
	PWM_Tim.PWM_BasicData.TIMx	=	MOTOR_TIMx;
	PWM_OUT_TIMConf(&PWM_Tim);									//PWM输出配置;
//	PWM_OUT_SetFre(&PWM_Tim);										//设置时间
	PWM_OUT_SetCount(&PWM_Tim,0);									//设置总输出脉冲个数

	//======方向控制输出
	GPIO_Configuration_OPP50	(MOTOR_DIR_PORT,		MOTOR_DIR_Pin);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void Sensor_Configuration(void)		//传感器配置
{
//#define	Sensor1_Port	GPIOB		//前传感器
//#define	Sensor1_Pin		GPIO_Pin_4

//#define	Sensor2_Port	GPIOB		//后传感器
//#define	Sensor2_Pin		GPIO_Pin_5

//#define	Sensor3_Port	GPIOB		//药品传感器
//#define	Sensor3_Pin		GPIO_Pin_6

//#define	Sensor4_Port	GPIOB		//备用传感器
//#define	Sensor4_Pin		GPIO_Pin_7
	
	//========传感器配置为上拉输入模式
	GPIO_Configuration_IPU(Sensor1_Port,	Sensor1_Pin);			//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(Sensor2_Port,	Sensor2_Pin);			//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(Sensor3_Port,	Sensor3_Pin);			//将GPIO相应管脚配置为上拉输入模式----V20170605
	GPIO_Configuration_IPU(Sensor4_Port,	Sensor4_Pin);			//将GPIO相应管脚配置为上拉输入模式----V20170605
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void Motor_RunSet(int Num)			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
{
	if(Num	==	0)		//强制停止
	{
//		MOTOR_DISABLE;								//停止电机
		PWM_OUT_SetCount(&PWM_Tim,0);		//设置输出个数
	}
	else if(Num>0)
	{
//		Motor_Configuration();			//步进电机驱动配置
		PWM_Tim.PWM_BasicData.PWM_Frequency=MOTOR_PWM_Frequency;						//频率增加--加速
		PWM_OUT_SetFre(&PWM_Tim);
		MOTOR_RunRight;													//顺时钟
		PWM_OUT_SetCount(&PWM_Tim,Num*Steeps);		//设置总输出脉冲个数
	}
	else
	{
//		Motor_Configuration();			//步进电机驱动配置
		PWM_Tim.PWM_BasicData.PWM_Frequency=MOTOR_PWM_Frequency;						//频率增加--加速
		PWM_OUT_SetFre(&PWM_Tim);
		Num	=	0	-	Num;
		MOTOR_RunLeft;													//顺时钟
		PWM_OUT_SetCount(&PWM_Tim,Num*Steeps);		//设置总输出脉冲个数
	}	
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
u8 Motor_Server(void)				//步进电机返回状态
{
	//SW6 ON表示与旋转相关的板
	//SW5 ON表示旋转电机控制板，OFF表示传感器板
	//CAN滤波器组使用 0x0A
	//传感器板CAN-ID为0x80
	//旋转电机控制板CAN-ID为0xC0
	//	u8 MotorBD	=	0;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
	//	u8 SensorBD	=	0;		//传感器板标志，0-非传感器板，1-传感器板	
	//	u8 RunToWindow	=	0;		//旋转到相应窗口 0-无，1-1号，2-2号，3-3号，4-4号
	
	u8 Status	=	0;
	CAN_Server();						//CAN收发数据管理
	Status	=	PWM_OUT_TIMServer(&PWM_Tim);		//获取定时器返回状态,返回0表示无中断返回
	
		if(Status	==1)			//表示定时中断
		{
//			MotorTime	=	0;		//电机重运行时间清零
			//=======================检查有无运行到位
			if(StatusOfWindow	==	0)	//找原点
			{
				if(RxMessage.Data[1]!=0)
				{
					StatusOfWindow	=	1;
					RunToWindow	=	0;
					Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
					return 2;
				}
			}
			if(RunToWindow==1	&&	StatusOfWindow!=1)
			{
				if(RxMessage.Data[1]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
					return 2;
				}
			}
			else if(RunToWindow==2	&&	StatusOfWindow!=2)
			{
				if(RxMessage.Data[3]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
					return 2;
				}
			}
			else if(RunToWindow==3	&&	StatusOfWindow!=3)
			{
				if(RxMessage.Data[2]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
					return 2;
				}
			}
			else if(RunToWindow==4	&&	StatusOfWindow!=3)
			{
				if(RxMessage.Data[1]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
					return 2;
				}
			}
			
			return 1;															//表示定时器中断
		}
		else if(Status	==2)	//表示计数完成====电机运行步数完成
		{
			MotorTime	=	0;		//电机重运行时间清零
			return 1;																//表示定时器中断
		}
			
	MotorTime++;
	if(MotorTime>=1000)
	{
		
		MotorTime=0;
	}
	return 0;
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
u8 SetToWindows(u8 nWindows)		//运行到指定窗口
{
	//	u8 RunToWindow	=	0;		//旋转到相应窗口 0-无，1-1号，2-2号，3-3号，4-4号
	//	u8 StatusOfWindow	=	0;	//当时停止位置：0-未初始化，1-原点，2-2号窗口，3-3号窗口，4-4号窗口
	//	u8 CMDOfWindow	=	0;		//运行到指定窗口命令
	//	#define	Steeps		4000	//一个窗口用到的驱动脉冲数
	//	#define	MaxWindow	8							//最大窗口数
	if(RunToWindow	!=0	)				//正在运行
	{
		return 0;
	}	
	if(nWindows	==0)									//无效指令
	{
		Motor_RunSet(0);								//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
		return 0;
	}	
	if(nWindows	>	MaxWindow)					//超出最大窗口数里
	{
		return 0;	
	}
	if(nWindows	==	StatusOfWindow)		//当前窗口---不需要旋转
	{
		return 0;
	}
	
	if(nWindows>StatusOfWindow)	//顺时针转动
	{
		RunToWindow	=	nWindows;		//需要运行到的窗口数
		Motor_RunSet(0-(nWindows-StatusOfWindow)*Steeps);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
	}
	else		//逆转
	{
		RunToWindow	=	nWindows;
		Motor_RunSet((nWindows-StatusOfWindow)*Steeps);				//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
	}
	return 1;
}


/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void Sensor_Read(void)		//读传感器信号
{	
	//	u8 SensorON		=	0	;		//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报
	if(Sensor1_Status	==	0)
	{
		if(Sensor[0]<0xFF)
		Sensor[0]++;
	}
	else
	{
		Sensor[0]	=	0;
	}
	if(Sensor2_Status	==	0)
	{
		if(Sensor[1]<0xFF)
		Sensor[1]++;
	}
	else
	{
		Sensor[1]	=	0;
	}
	if(Sensor3_Status	==	0)
	{
		if(Sensor[2]<0xFF)
		Sensor[2]++;
	}
	else
	{
		Sensor[2]	=	0;
	}
	if(Sensor4_Status	==	0)
	{
		if(Sensor[3]<0xFF)
		Sensor[3]++;
	}
	else
	{
		Sensor[3]	=	0;
	}
	
//	if(Sensor[0]>SensorON	||	Sensor[1]>SensorON	||	Sensor[2]>SensorON	||	Sensor[3]>SensorON	)
//	{
//		memset(Sensor,0x00,4);
//		CAN_StdTX_DATA(CANID,4,Sensor);			//CAN使用标准帧发送数据
//	}
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void CAN_Server(void)			//CAN收发数据管理
{
	//SW6 ON表示与旋转相关的板
	//SW5 ON表示旋转电机控制板，OFF表示传感器板
	//CAN滤波器组使用 0x0A
	//传感器板CAN-ID为0x80
	//旋转电机控制板CAN-ID为0xC0
	//	u8 MotorBD	=	0;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
	//	u8 SensorBD	=	0;		//传感器板标志，0-非传感器板，1-传感器板
	
	//	u8 RunToWindow	=	0;		//旋转到相应窗口 0-无，1-1号，2-2号，3-3号，4-4号
	//	u8 SensorON		=	0	;		//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报
	u8 status	=	0;		//CAN读取返回0表示无效
	
	
	status	=	CAN_RX_DATA(&RxMessage);									//检查CAN接收有无数据
	if(status	==0)
		return;
	if(SensorBD)															//传感器板
	{
//		Sensor[0]=0x01;
//		Sensor[1]=0x02;
//		Sensor[2]=0x03;
//		Sensor[3]=0x04;
//		CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
//		status	=	CAN_RX_DATA(&RxMessage);									//检查CAN接收有无数据
//	if(status	==0)
//		return;
//		memset(RxMessage.Data,0x00,4);
		if(RxMessage.StdId	==	0xC0)							//旋转电机控制板发来数据
		{
			//传感器信号采集开关
			if(RxMessage.Data[0]==0xAF	&&	RxMessage.Data[1]==0x01)
			{				
				SensorON	=	1;		//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报
			}
			else if(RxMessage.Data[0]==0xAF	&&	RxMessage.Data[1]==0x00)
			{				
				SensorON	=	0;		//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报
			}
			memset(RxMessage.Data,0x00,2);
//			memset(Sensor,0x05,4);
//			CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
//			if(RxMessage.Data[0]	==	0x0A)					//旋转电机控制板查询传感器状态
//			{	
//				Sensor_Read();		//读传感器信号
//				CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
//				memset(Sensor,0x00,4);
//			}
		}
	}
	else if(MotorBD)				//旋转电机控制板
	{
//		status	=	CAN_RX_DATA(&RxMessage);									//检查CAN接收有无数据
//		if(status	==0)
//			return;
		if(RxMessage.StdId	==	0x80)							//旋转传感器发来数据
		{
//			memset(Sensor,0x88,4);
//			memset(RxMessage.Data,0x00,8);
//			CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
//			if(RxMessage.Data[0]	==	0x0A)					//旋转电机控制板查询传感器状态
//			{				
//				CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
////				memset(Sensor,0x00,4);
//			}
		}
	}
	else
	{
//		memset(RxMessage.Data,0x00,4);
	}
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void Switch_Server(void)			//检查拔码地址有无变更，如果变更，重新配置运行参数
{
	
	SWITCHID_Read(&SWITCHID);		//
	
	if(SWITCHID.nSWITCHID	!=	SwitchData)
	{
		Reset_Data();		//复位所有的全局变量值
		SwitchData	=	SWITCHID.nSWITCHID;
		PC006V21_Configuration();
	}
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void Reset_Data(void)		//复位所有的全局变量值
{
//	SwitchData	=	0;	//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
	SensorBD	=	0;		//传感器板标志，0-非传感器板，1-传感器板
	MotorBD	=	0;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
	MotorTime	=	0;	//旋转电机测试运行时间

//	CANID	=	0;
	SYSTime=0;

	memset(Sensor,0x00,4);	//4个传感器感应值存储区
	SensorON		=	0	;		//旋转电机控制板发来开启传感器采集命令BUFFER[0]=0XAF BUFFER[1]=0关，=1开，0--不采集信号，1-采集信号0.5ms主动上报

	RunToWindow	=	0;		//旋转到相应窗口 0-无，1-1号，2-2号，3-3号，4-4号
	StatusOfWindow	=	0;	//当时停止位置：0-未初始化，1-原点，2-2号窗口，3-3号窗口，4-4号窗口
	CMDOfWindow	=	0;		//F
}



#endif
