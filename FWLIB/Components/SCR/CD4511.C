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



#include "CD4511.H"
#include "STM32_GPIO.H"
//#include "STM32F10x_BitBand.H"

CD4511BcdDef* CD4511Bcd;		//BCD码结构体
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*******************************************************************************/
void CD4511_Delay(u32 time)
{
	while(time--);
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*******************************************************************************/
void CD4511_PinConf(CD4511_Pindef *Pinfo)
{
	GPIO_Configuration_OPP50	(Pinfo->CD4511_A0_PORT,		Pinfo->CD4511_A0_Pin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(Pinfo->CD4511_A1_PORT,		Pinfo->CD4511_A1_Pin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(Pinfo->CD4511_A2_PORT,		Pinfo->CD4511_A2_Pin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
	GPIO_Configuration_OPP50	(Pinfo->CD4511_A3_PORT,		Pinfo->CD4511_A3_Pin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度2MHz----V20170605
}
/*******************************************************************************
*函数名			:	CD4511_Clear
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void CD4511_Clear(CD4511_Pindef *Pinfo)		//清除输出：bitA~bitG输出低电平
{
	GPIO_ResetBits(Pinfo->CD4511_A0_PORT, Pinfo->CD4511_A0_Pin);			//A/A0
	GPIO_SetBits	(Pinfo->CD4511_A1_PORT, Pinfo->CD4511_A1_Pin);			//B/A1
	GPIO_ResetBits(Pinfo->CD4511_A2_PORT, Pinfo->CD4511_A2_Pin);			//C/A2
	GPIO_SetBits	(Pinfo->CD4511_A3_PORT, Pinfo->CD4511_A3_Pin);			//D/A3
}
/*******************************************************************************
*函数名			:	CD4511_Clear
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void CD4511_WriteData(CD4511_Pindef *Pinfo,unsigned char num)		//BCD转换为Segment输出 只输出0~9
{
	num	=	num%10;			//限定在1位数
	CD4511Bcd	=	(CD4511BcdDef*)&num;				//转换为BCD码
	
	if(CD4511Bcd->A)
	{
		GPIO_SetBits(Pinfo->CD4511_A0_PORT, Pinfo->CD4511_A0_Pin);
	}
	else
	{
		GPIO_ResetBits(Pinfo->CD4511_A0_PORT, Pinfo->CD4511_A0_Pin);			//A/A0
	}
	if(CD4511Bcd->B)
	{
		GPIO_SetBits(Pinfo->CD4511_A1_PORT, Pinfo->CD4511_A1_Pin);
	}
	else
	{
		GPIO_ResetBits(Pinfo->CD4511_A1_PORT, Pinfo->CD4511_A1_Pin);			//B/A1
	}
	if(CD4511Bcd->C)
	{
		GPIO_SetBits(Pinfo->CD4511_A2_PORT, Pinfo->CD4511_A2_Pin);
	}
	else
	{
		GPIO_ResetBits(Pinfo->CD4511_A2_PORT, Pinfo->CD4511_A2_Pin);			//C/A2
	}
	if(CD4511Bcd->D)
	{
		GPIO_SetBits(Pinfo->CD4511_A3_PORT, Pinfo->CD4511_A3_Pin);
	}
	else
	{
		GPIO_ResetBits(Pinfo->CD4511_A3_PORT, Pinfo->CD4511_A3_Pin);			//D/A3
	}
}

/*******************************************************************************
* 函数名			:	CD4511_DisplayOFF
* 功能描述		:	关闭显示---关NPN三极管
* 输入			: void
* 返回值			: void
*******************************************************************************/
void CD4511_DisplayOFF(CD4511_Pindef *Pinfo)			//关闭显示---关NPN三极管
{
//	GPIO_ResetBits(Pinfo->CD4511_EN_PORT, Pinfo->CD4511_EN_Pin);
//	CD4511_Delay(1000);
}
/*******************************************************************************
* 函数名			:	CD4511_DisplayOFF
* 功能描述		:	关闭显示---关NPN三极管
* 输入			: void
* 返回值			: void
*******************************************************************************/
void CD4511_DisplayON(CD4511_Pindef *Pinfo)			//开显示---关NPN三极管
{
//	GPIO_SetBits(Pinfo->CD4511_EN_PORT, Pinfo->CD4511_EN_Pin);
//	CD4511_Delay(1000);
}
