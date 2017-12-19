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

//CAN读传感器指令	0x0A BUFFER[0]

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

//=================PA6电机CCW做脉冲输出
#define	MOTOR_PWM_PORT	GPIOA
#define	MOTOR_PWM_Pin		GPIO_Pin_7
//=================PA7电机CW做方向输出
#define	MOTOR_DIR_PORT	GPIOA
#define	MOTOR_DIR_Pin		GPIO_Pin_6

#define	MOTOR_PWM_Frequency	2000	//脉冲频率--单位Hz
#define	PWM_UpdataCount	1					//频率变化占计数个数，就是计数PWM_Updata个数后更新一次输出频率
#define	PWM_RunUpCount	100				//加速/减速需要的步数
#define	MOTOR_TIMx	TIM1					//旋转电机所使用的定时器

#define	Steeps		1			//一个窗口用到的驱动脉冲数

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
#define	Sensor1_Status	Sensor1_Port->IDR & Sensor1_Pin
#define	Sensor2_Status	Sensor2_Port->IDR & Sensor2_Pin
#define	Sensor3_Status	Sensor3_Port->IDR & Sensor3_Pin
#define	Sensor4_Status	Sensor4_Port->IDR & Sensor4_Pin

SWITCHID_CONF SWITCHID;					//拔码开关结构体
PWM_TimDef		PWM_Tim;					//PWM控制脉冲结构体

u8 SensorBD	=	0;		//传感器板标志，0-非传感器板，1-传感器板
u8 MotorBD	=	0;		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
u32	MotorTime	=	0;	//旋转电机测试运行时间

u8	CANID	=	0;
u16 SYSTime=0;
u16 data=0;
u8 Flag=0;
u8 Sensor[4]	=	{0};

u8 RunToWindow	=	0;		//旋转到相应窗口 0-无，1-1号，2-2号，3-3号，4-4号

CanRxMsg RxMessage;			//CAN接收 
CanTxMsg TxMessage;			//CAN发送




void SWITCHID_Configuration(void);			//拔码开关初始化及读数
void Motor_Configuration(void);					//步进电机驱动配置
void Sensor_Configuration(void);				//传感器配置
void CAN_Configuration(void);						//CAN配置



void Motor_RunSet(int Num);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
void Sensor_Read(void);					//读传感器信号	

u8 Motor_Server(void);				//步进电机返回状态
void CAN_Server(void);			//CAN收发数据管理
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
	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);			//SYS-LED 1HZ 50%
	SysTick_Configuration(1000);							//系统嘀嗒时钟配置72MHz,单位为uS
	SWITCHID_Configuration();									//拔码开关初始化及读数
	Motor_Configuration();										//步进电机驱动配置
	CAN_Configuration();							//CAN1配置---标志位查询方式，不开中断--100K
	Sensor_Configuration();						//传感器配置
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
	if(MotorBD)
	{
		if(Motor_Server())					//步进电机返回状态---为定时器中断
		{
			return;
		}
	}
	IWDG_Feed();								//独立看门狗喂狗
	if(SensorBD)
	{
		Sensor_Read();		//读传感器信号
	}
	CAN_Server();				//CAN收发数据管理	
	SYSTime++;
	if(SYSTime>=10000)
	{
		
		SYSTime=0;
	}
//	if(SYSTime	==	1)
//	{
//		Motor_RunSet(2000);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
//	}
//	else if(SYSTime	==	5000)
//	{
//		Motor_RunSet(-2000);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
////		TIM1->CNT	=	1;
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
		CAN_FilterInitConfiguration_StdData(0X0A,CANID,0xFFBF);			//CAN滤波器配置---标准数据帧模式---BIT7不过滤
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
	
	if(MotorBD)		//旋转电机控制板标志，0-非电机控制板，1-电机控制板
	{
		u8 Status	=	0;
		Status	=	PWM_OUT_TIMServer(&PWM_Tim);		//获取定时器返回状态,返回0表示无中断返回
		if(Status	==1)			//表示定时中断
		{
//			MotorTime	=	0;		//电机重运行时间清零
			//=======================检查有无运行到位
			CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
			if(RxMessage.Data[0]!=0	&&	RunToWindow==1)
			{
				Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
			}
			else if(RxMessage.Data[1]!=0	&&	RunToWindow==2)
			{
				Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
			}
			else if(RxMessage.Data[2]!=0	&&	RunToWindow==3)
			{
				Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
			}
			else if(RxMessage.Data[3]!=0	&&	RunToWindow==4)
			{
				Motor_RunSet(0);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
			}
			
			return 1;															//表示定时器中断
		}
		else if(Status	==2)	//表示计数完成====电机运行步数完成
		{
//			MotorTime	=	0;		//电机重运行时间清零
			return 1;																//表示定时器中断
		}
		else
		{
			MotorTime++;
			if(MotorTime>=1000)
			{
				
				MotorTime=0;
			}
			if(MotorTime	==	1)
			{
				Motor_RunSet(100);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
			}
			else if(MotorTime	==	500)
			{
				Motor_RunSet(-100);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
			}
			return 0;
		}		
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
void Sensor_Read(void)		//读传感器信号
{	
//	u8 SensorON	=	10;			//传感器有效值
	if(Sensor1_Status)
	{
		if(Sensor[0]<0xFF)
		Sensor[0]++;
	}
	else
	{
		Sensor[0]	=	0;
	}
	if(Sensor2_Status)
	{
		if(Sensor[1]<0xFF)
		Sensor[1]++;
	}
	else
	{
		Sensor[1]	=	0;
	}
	if(Sensor3_Status)
	{
		if(Sensor[2]<0xFF)
		Sensor[2]++;
	}
	else
	{
		Sensor[2]	=	0;
	}
	if(Sensor4_Status)
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
	
	u8 status	=	0;		//CAN读取返回0表示无效
	status	=	CAN_RX_DATA(&RxMessage);									//检查CAN接收有无数据
	if(status	==0)
		return;
	if(SensorBD)															//传感器板
	{
		if(RxMessage.StdId	==	0xC0)							//旋转电机控制板发来数据
		{
			if(RxMessage.Data[0]	==	0x0A)					//旋转电机控制板查询传感器状态
			{	
				Sensor_Read();		//读传感器信号
				CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
				memset(Sensor,0x00,4);
			}
		}
	}
	else if(MotorBD)				//旋转电机控制板
	{
		if(RxMessage.StdId	==	0x80)							//旋转传感器发来数据
		{
			if(RxMessage.Data[0]	==	0x0A)					//旋转电机控制板查询传感器状态
			{				
//				CAN_StdTX_DATA(CANID,4,Sensor);				//CAN使用标准帧发送数据
//				memset(Sensor,0x00,4);
			}
		}
	}
	else
	{
		memset(RxMessage.Data,0x00,4);
	}
}



#endif
