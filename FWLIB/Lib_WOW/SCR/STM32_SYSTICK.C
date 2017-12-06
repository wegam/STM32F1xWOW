/******************************** User_library *********************************
* 文件名 	: STM32_SDCard.H
* 作者   	: wegam@sina.com
* 版本   	: V
* 日期   	: 2016/01/01
* 说明   	: 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#include "STM32_SYSTICK.H"
#include "STM32_WOW.H"
//#include "STM32F10x_BitBand.H"


/*******************************************************************************
* 函数名		:	SysTick_Configuration
* 功能描述	:	系统嘀嗒时钟配置72MHz,单位为uS
* 输入		:	0<Time<= 1C71C7(1864135)
* 输出		:
* 返回 		:
*******************************************************************************/
void SysTick_Configuration(unsigned long Time)	//系统嘀嗒时钟配置72MHz,单位为uS
{
	if(Time	==	0)
		return;
//	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);					//系统时钟 72MHZ
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);			//系统时钟/8==9MHz
//	SysTick_SetReload(9);			//1uS
	SysTick_SetReload(9*Time-1);				//Time--uS
	SysTick_ITConfig(DISABLE);					//关闭中断
	SysTick_CounterCmd(SysTick_Counter_Enable);	//使能计数
	SysTick_ITConfig(ENABLE);
}
/*******************************************************************************
* 函数名		:	SysTick_Server
* 功能描述	:	嘀嗒时钟服务
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void SysTick_Server(void)				//嘀嗒时钟服务
{
	WOW_Server();	
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*******************************************************************************/
void SysTick_DeleyuS(unsigned int Time)
{
	unsigned int Ctrl	=	0;		//保存原控制参数
	unsigned int Load	=	0;		//保存原重装载值
	if(Time	==	0)
		return;
	Ctrl	=	SysTick->CTRL&0x0000FFFF;		//获取原控制参数
	Load	=	SysTick->LOAD;		//获取原重装载值
	SysTick->CTRL &= 0xFFFFFFFD;									//SysTick_ITConfig(DISABLE);					//关闭中断
	SysTick->CTRL &= SysTick_Counter_Disable;			//SysTick_CounterCmd(SysTick_Counter_Disable);	//关闭计数
	SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;	//SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);			//系统时钟/8==9MHz
	SysTick->VAL = SysTick_Counter_Clear;					//SysTick_CounterCmd(SysTick_Counter_Clear);	//清除倒计数值
	SysTick->LOAD = 9*Time-8;	//SysTick_SetReload(9*time);				//Time--uS  -8为了减小误差，不可以-9，防止Time==1的情况
	SysTick->CTRL |= SysTick_Counter_Enable;			//SysTick_CounterCmd(SysTick_Counter_Enable);	//使能计数
	while(SysTick->VAL	==	0);			//等待开始装载
	while(SysTick->VAL	!=	0);			//等待倒计数完成
	
	
	SysTick->LOAD	=	Load;			//恢复原控制参数
	SysTick->CTRL	=	Ctrl;			//恢复原控制参数
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*******************************************************************************/
void SysTick_DeleymS(unsigned int Time)
{
	unsigned int Ctrl	=	0;		//保存原控制参数
	unsigned int Load	=	0;		//保存原重装载值
	if(Time	==	0)
		return;
	Ctrl	=	SysTick->CTRL&0x0000FFFF;		//获取原控制参数
	Load	=	SysTick->LOAD;		//获取原重装载值
	SysTick->CTRL &= 0xFFFFFFFD;									//SysTick_ITConfig(DISABLE);					//关闭中断
	SysTick->CTRL &= SysTick_Counter_Disable;			//SysTick_CounterCmd(SysTick_Counter_Disable);	//关闭计数
	SysTick->CTRL &= SysTick_CLKSource_HCLK_Div8;	//SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);			//系统时钟/8==9MHz
	SysTick->VAL = SysTick_Counter_Clear;					//SysTick_CounterCmd(SysTick_Counter_Clear);	//清除倒计数值
	SysTick->LOAD = 9*Time*1000-8;	//SysTick_SetReload(9*time);				//Time--uS  -8为了减小误差，不可以-9，防止Time==1的情况
	SysTick->CTRL |= SysTick_Counter_Enable;			//SysTick_CounterCmd(SysTick_Counter_Enable);	//使能计数
	while(SysTick->VAL	==	0);			//等待开始装载
	while(SysTick->VAL	!=	0);			//等待倒计数完成
	
	
	SysTick->LOAD	=	Load;			//恢复原控制参数
	SysTick->CTRL	=	Ctrl;			//恢复原控制参数
}



