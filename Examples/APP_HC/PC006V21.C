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

#ifdef PC006V21			//分检机控制板

#include "PC006V21.H"

#include "SWITCHID.H"
#include "STM32_WDG.H"
#include "STM32_EXTI.H"
#include "STM32_GPIO.H"
#include "STM32_PWM.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32F10x_BitBand.H"

//=================PA6电机CCW做脉冲输出
#define	MOTOR_PWM_PORT	GPIOA
#define	MOTOR_PWM_Pin		GPIO_Pin_6
//=================PA7电机CW做方向输出
#define	MOTOR_DIR_PORT	GPIOA
#define	MOTOR_DIR_Pin		GPIO_Pin_7

#define	MOTOR_PWM_Frequency	10000	//脉冲频率--单位Hz
#define	MOTOR_TIMx	TIM1					//所使用的定时器

#define	Steeps		10			//一个窗口用到的驱动脉冲数

#define	MOTOR_RunRight			MOTOR_DIR_PORT->BSRR 	= MOTOR_DIR_Pin		//EN	=	1;	//顺时针
#define	MOTOR_RunLeft				MOTOR_DIR_PORT->BRR		= MOTOR_DIR_Pin		//EN	=	0;	//逆时钟

SWITCHID_CONF SWITCHID;					//拔码开关结构体
PWM_TimDef		PWM_Tim;					//PWM控制脉冲结构体

u16 SYSTime=0;
u16 data=0;
u8 Flag=0;


void SWITCHID_Configuration(void);			//拔码开关初始化及读数
void Motor_Configuration(void);					//步进电机驱动配置

void Motor_RunSet(int Num);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
	

u8 Motor_Server(void);				//步进电机返回状态
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
	IWDG_Configuration(1000);	//独立看门狗配置	Tout-超时复位时间，单位ms
	
	//位
//	GPIO_Configuration_OPP50	(GPIOA,		GPIO_Pin_6);			//CCW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
//	GPIO_Configuration_OPP50	(GPIOA,		GPIO_Pin_7);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_0);			//备1	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_1);			//备2	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_12);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_13);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_14);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_15);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	
	EXTI_Configuration_ITF		(GPIOB, 	GPIO_Pin_4);		//外部下降沿中断配置,抢占1，响应1--20171213
	EXTI_Configuration_ITF		(GPIOB, 	GPIO_Pin_5);		//外部下降沿中断配置,抢占1，响应1--20171213
	EXTI_Configuration_ITF		(GPIOB, 	GPIO_Pin_6);		//外部下降沿中断配置,抢占1，响应1--20171213
	EXTI_Configuration_ITF		(GPIOB, 	GPIO_Pin_7);		//外部下降沿中断配置,抢占1，响应1--20171213
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
	if(Motor_Server())					//步进电机返回状态---为定时器中断
	{
		return;
	}
	IWDG_Feed();								//独立看门狗喂狗
	SYSTime++;
	if(SYSTime>=2000)
	{
		
		SYSTime=0;
	}
	if(SYSTime	==	0)
		Motor_RunSet(-2);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
	else if(SYSTime	==	1000)
		Motor_RunSet(2);			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
//	if(Trigger_Line.Trigger_Line4)
//	{
//		PA6	=	0;
//		PA7	=	1;
//	}
//	else if(Trigger_Line.Trigger_Line5)
//	{
//		PA6	=	1;
//		PA7	=	0;
//	}
//	if(SYSTime	<500)		//0.5S关闭输出
//	{
//		PA6	=	0;
//		PA7	=	0;
//	}
//	else if(SYSTime	<	2500)	//正转2S
//	{
//		PA6	=	1;
//		PA7	=	0;
//	}
//	else if(SYSTime	<	3000)	//0.5S关闭输出
//	{
//		PA6	=	0;
//		PA7	=	0;
//	}
//	else if(SYSTime	<	5000)	//反转2S
//	{
//		PA6	=	0;
//		PA7	=	1;
//	}
//	else
//	{
//		SYSTime	=	0;			//清零计数
//	}
	
//	if(SYSTime	%2000	==0)
//	{
//		GPIO_Toggle	(GPIOB,		GPIO_Pin_0);		//将GPIO相应管脚输出翻转----V20170605
//		GPIO_Toggle	(GPIOB,		GPIO_Pin_1);		//将GPIO相应管脚输出翻转----V20170605
////		GPIO_Toggle	(GPIOA,		GPIO_Pin_6);		//将GPIO相应管脚输出翻转----V20170605
////		GPIO_Toggle	(GPIOA,		GPIO_Pin_7);		//将GPIO相应管脚输出翻转----V20170605
//		
//		GPIO_Toggle	(GPIOB,		GPIO_Pin_12);		//将GPIO相应管脚输出翻转----V20170605
//		GPIO_Toggle	(GPIOB,		GPIO_Pin_13);		//将GPIO相应管脚输出翻转----V20170605
//		GPIO_Toggle	(GPIOB,		GPIO_Pin_14);		//将GPIO相应管脚输出翻转----V20170605
//		GPIO_Toggle	(GPIOB,		GPIO_Pin_15);		//将GPIO相应管脚输出翻转----V20170605
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
u8 Motor_Server(void)				//步进电机返回状态
{
	u8 Status	=	0;
	Status	=	PWM_OUT_TIMServer(&PWM_Tim);		//获取定时器返回状态
	if(Status	==1)			//表示定时中断
	{
		return 1;																//表示定时器中断
	}
	else if(Status	==2)	//表示计数完成====电机运行步数完成
	{
		return 1;																//表示定时器中断
	}
	else
	{
		return 0;
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
	SWITCHID.NumOfSW	=	6;
	//SW1--PA4
	SWITCHID.SW1_PORT	=	GPIOA;
	SWITCHID.SW1_Pin	=	GPIO_Pin_4;
	//SW2--PA5
	SWITCHID.SW1_PORT	=	GPIOA;
	SWITCHID.SW1_Pin	=	GPIO_Pin_5;
	//SW3--PB12
	SWITCHID.SW1_PORT	=	GPIOB;
	SWITCHID.SW1_Pin	=	GPIO_Pin_12;
	//SW4--PB13
	SWITCHID.SW1_PORT	=	GPIOB;
	SWITCHID.SW1_Pin	=	GPIO_Pin_13;
	//SW5--PB14
	SWITCHID.SW1_PORT	=	GPIOB;
	SWITCHID.SW1_Pin	=	GPIO_Pin_14;
	//SW6--PB15
	SWITCHID.SW1_PORT	=	GPIOB;
	SWITCHID.SW1_Pin	=	GPIO_Pin_15;
	
	SWITCHID_Conf(&SWITCHID);		//
	SWITCHID_Read(&SWITCHID);		//
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
	PWM_Tim.PWM_BasicData.PWM_Frequency	=	MOTOR_PWM_Frequency;
	PWM_Tim.PWM_BasicData.TIMx	=	MOTOR_TIMx;
	PWM_OUT_TIMConf(&PWM_Tim);									//PWM输出配置;
	PWM_OUT_TIMSet(&PWM_Tim,0);									//设置总输出脉冲个数

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
*注释				:	wegam@sina.com
*******************************************************************************/
void Motor_RunSet(int Num)			//使药架旋转---Num为正值时表时顺时针转(从上往下视角)Num个格数，负值时为逆时针转Num个格数，0为停止
{
	if(Num	==	0)		//强制停止
	{
//		MOTOR_DISABLE;								//停止电机
		PWM_OUT_TIMSet(&PWM_Tim,0);		//设置输出个数
	}
	else if(Num>0)
	{
		MOTOR_RunRight;													//顺时钟
		PWM_OUT_TIMSet(&PWM_Tim,Num*Steeps);		//设置总输出脉冲个数
	}
	else
	{
		Num	=	0	-	Num;
		MOTOR_RunLeft;													//顺时钟
		PWM_OUT_TIMSet(&PWM_Tim,Num*Steeps);		//设置总输出脉冲个数
	}	
}



#endif
