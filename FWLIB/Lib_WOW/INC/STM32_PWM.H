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


#ifndef __STM32_PWM_H
#define __STM32_PWM_H



#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_nvic.h"

//#include "STM32_TIM.H"


typedef enum
{
  PWM_OUTChannel1	=	0,
	PWM_OUTChannel2,
	PWM_OUTChannel3,
	PWM_OUTChannel4,
} PWM_OUTChannelTypeDef;
typedef struct
{
	TIM_TypeDef*	TIMx;
	GPIO_TypeDef*	GPIOx;			//x=A/B/C/D/E/F/G
//	volatile u16 GPIO_Pin_n;						//n=0~15;
//	
//	double PWM_Frequency;			//频率
//	volatile u8 	PWM_Ratio;						//占空比 1%精度，默认50%（未设置此参数时）		
}PWM_BasicDataDef;
typedef struct
{
	u8 	Pulse;								//计算占空比，Pulse<=PWM_Ratio为高，Pulse》=PWM_Cycle为一个计算周期，清0
	u8 	PWM_Cycle ;						//根据占空比需要的计数值，如1%占空比，则计数100,50%计数2就可以
	u16 PWM_Count;						//输出脉冲数，每计一周期脉冲数增加一个，计数达到PWM_Num停止输出，需要再次输出时则清0此计数，重新开始计数
}PWM_RunDataDef;
typedef struct
{
	PWM_RunDataDef*		PWM_RunData;
	PWM_BasicDataDef*	PWM_BasicData;
	
	TIM_TypeDef*	TIMx;
	GPIO_TypeDef*	GPIOx;			//x=A/B/C/D/E/F/G
	
	u8 	Pulse;								//计算占空比，Pulse<=PWM_Ratio为高，Pulse》=PWM_Cycle为一个计算周期，清0
	u8 	PWM_Cycle ;						//根据占空比需要的计数值，如1%占空比，则计数100,50%计数2就可以
	u16 PWM_Count;						//输出脉冲数，每计一周期脉冲数增加一个，计数达到PWM_Num停止输出，需要再次输出时则清0此计数，重新开始计数
//	
//	GPIO_TypeDef* GPIOm;
//	u32*	Ptr;
//	u32 	Num;
}PWM_TimDef;
extern u32 Tim1_Count;
extern u32 Tim2_Count;
extern u32 Tim3_Count;
extern u32 Tim4_Count;

void PWM_OUT(TIM_TypeDef* TIMx,PWM_OUTChannelTypeDef PWM_OUTChanneln,double PWM_Frequency ,u16 PWM_Ratio);	//PWM设定-20161127版本	占空比1/1000
void PWM_OUT2(TIM_TypeDef* TIMx,PWM_OUTChannelTypeDef PWM_OUTChanneln,double PWM_Frequency ,u16 PWM_Ratio);	//PWM设定-20161127版本	占空比1/1200
void PWM_OUT_COUNT(TIM_TypeDef* TIMx,PWM_OUTChannelTypeDef PWM_OUTChanneln,double PWM_Frequency ,u16 PWM_Ratio);	//PWM设定-20161127版本	占空比1/1200
void PWM_OUT_TIMConf(PWM_TimDef* PWM_Tim);			//PWM输出配置
void PWM_OUT_TIMSet(PWM_TimDef* PWM_Tim,u32 PWM_Count);			//PWM输出配置
u8 PWM_OUT_TIMServer(PWM_TimDef* PWM_Tim);			//PWM输出配置
void SetPWM_Ratio(u16 PWM_Ratio);		//设置占空比---LED
void SetPWM_Num(TIM_TypeDef* TIMx,u32 PWM_Count);		//设置计数值
u8 PWM_CountServer(void);			//计数服务程序
#endif
