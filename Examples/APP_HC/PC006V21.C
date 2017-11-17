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

#include "STM32_GPIO.H"
#include "STM32_PWM.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32F10x_BitBand.H"


u16 SYSTime=0;
u16 data=0;
u8 Flag=0;

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
	PWM_OUT(TIM1,PWM_OUTChannel1,1,500);			//SYS-LED 1HZ 50%
	SysTick_Configuration(1000);							//系统嘀嗒时钟配置72MHz,单位为uS
	
	//位
	GPIO_Configuration_OPP50	(GPIOA,		GPIO_Pin_6);			//CCW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOA,		GPIO_Pin_7);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_12);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_13);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_14);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(GPIOB,		GPIO_Pin_15);			//CW	//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	
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
	SYSTime++;	
	if(SYSTime	<500)		//0.5S关闭输出
	{
		PA6	=	0;
		PA7	=	0;
	}
	else if(SYSTime	<	2500)	//正转2S
	{
		PA6	=	1;
		PA7	=	0;
	}
	else if(SYSTime	<	3000)	//0.5S关闭输出
	{
		PA6	=	0;
		PA7	=	0;
	}
	else if(SYSTime	<	5000)	//反转2S
	{
		PA6	=	0;
		PA7	=	1;
	}
	else
	{
		SYSTime	=	0;			//清零计数
	}
	
	if(SYSTime	%500	==0)
	{
		GPIO_Toggle	(GPIOB,		GPIO_Pin_12);		//将GPIO相应管脚输出翻转----V20170605
		GPIO_Toggle	(GPIOB,		GPIO_Pin_13);		//将GPIO相应管脚输出翻转----V20170605
		GPIO_Toggle	(GPIOB,		GPIO_Pin_14);		//将GPIO相应管脚输出翻转----V20170605
		GPIO_Toggle	(GPIOB,		GPIO_Pin_15);		//将GPIO相应管脚输出翻转----V20170605
	}
}

#endif
