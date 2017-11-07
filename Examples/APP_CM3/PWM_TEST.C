#ifdef PWM_TEST
#include "PWM_TEST.H"

#include "string.h"
#include "math.h"


#include "STM32_EXTI.H"
#include "STM32_USART.H"

#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_GPIO.H"
#include "STM32F10x_BitBand.H"

#define RxBufferSize	16

PWM_TimDef* 	PWM_Tim;
volatile GPIO_TypeDef* GPIOx;			//x=A/B/C/D/E/F/G
volatile TIM_TypeDef * TIMx;
volatile u32* p	=	0;
volatile u32 temp	=	0;
u16 systime	=	0;
volatile u16 Pin	=	0;
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PWM_TEST_Configuration(void)
{
	SYS_Configuration();				//系统配置
	GPIO_DeInitAll();													//将所有的GPIO关闭----V20170605
	SysTick_Configuration(1000);	//系统嘀嗒时钟配置72MHz,单位为uS
	
//	GPIO_Configuration0();
//	TIM_Configuration(TIM1,7200,3000);	//定时时间设定
//	PWM_Configuration(TIM2,7200,10000,51);
//	PWM_OUT(TIM1,PWM_OUTChannel1,20000,50);		//PWM设定
//	PWM_OUT(TIM2,PWM_OUTChannel1,5,100);	//PWM设定
//	PWM_OUT(TIM2,PWM_OUTChannel1,100000,500);		//sys_led
//	PWM_OUT(TIM1,PWM_OUTChannel1,1200,500);		//PWM设定
//	PWM_OUT2(TIM1,PWM_OUTChannel1,2400,600);		//PWM设定
//	PWM_OUT2(TIM3,PWM_OUTChannel3,2400,600);		//PWM设定
//	PWM_OUT2(TIM4,PWM_OUTChannel2,2400,600);		//PWM设定
//	PWM_OUT_COUNT(TIM2,PWM_OUTChannel1,10000,200);		//sys_led
	
//	GPIO_Configuration_IPD(GPIOA,	GPIO_Pin_4);			//将GPIO相应管脚配置为上拉输入模式----V20170605--BUTTON
//	GPIO_Configuration_IPD(GPIOA,	GPIO_Pin_5);			//将GPIO相应管脚配置为上拉输入模式----V20170605
//	GPIO_Configuration_IPD(GPIOA,	GPIO_Pin_6);			//将GPIO相应管脚配置为上拉输入模式----V20170605
	
//	PWM_OUT(TIM2,PWM_OUTChannel1,38000,500);		//sys_led
//	PWM_OUT(TIM3,PWM_OUTChannel1,20000,30);		//PWM设定
//	PWM_OUT(TIM4,PWM_OUTChannel1,20000,40);		//PWM设定
//	
//	PWM_OUT(TIM1,PWM_OUTChannel2,20000,50);		//PWM设定
//	PWM_OUT(TIM2,PWM_OUTChannel2,20000,500);	//PWM设定

//	SetPWM_Num(TIM2,10);		//设置计数值

//	PWM_Tim->PWM_BasicData->GPIOx	=	GPIOA;
//	PWM_Tim->PWM_BasicData->TIMx	=	TIM1;
//	
//	GPIOx	=	PWM_Tim->PWM_BasicData->GPIOx;
//	TIMx	=	PWM_Tim->PWM_BasicData->TIMx;

//	PWM_Tim->PWM_RunData->Pulse	=	0x20;
//	PWM_Tim->PWM_RunData->PWM_Count	=	0x30;
//	PWM_Tim->PWM_RunData->PWM_Cycle	=	0x50;

//	PWM_OUT_TIMConf(PWM_Tim);										//PWM输出配置
//	
//	GPIOx	=	PWM_Tim->PWM_BasicData->GPIOx;
//	TIMx	=	PWM_Tim->PWM_BasicData->TIMx;
	

//	PWM_OUT_TIMSet(PWM_Tim,10);		//设置计数值

	PWM_Tim->GPIOx	=	GPIOA;
	PWM_Tim->TIMx		=	TIM1;
	
	PWM_Tim->Pulse	=	0x10;
	PWM_Tim->PWM_Count	=	0x20;
	
	GPIOx	=	PWM_Tim->GPIOx;
	TIMx	=	PWM_Tim->TIMx;
	
	PWM_OUT_TIMConf(PWM_Tim);										//PWM输出配置
	
	GPIOx	=	PWM_Tim->GPIOx;
	TIMx	=	PWM_Tim->TIMx;
	
	


}
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PWM_TEST_Server(void)
{
	if(PWM_OUT_TIMServer(PWM_Tim)	==1)
	{
		systime	=	0;
	}
	else
	{
		systime++;
		if(systime>=1000)
		{
			systime	=	0;
			PWM_OUT_TIMSet(PWM_Tim,10);		//设置计数值
		}
	}
	//========================
//	if(PWM_CountServer()	==1)
//	{
//	}
//	else
//	{
//		systime++;
//		if(systime>=100)
//		{
//			systime	=	0;
//			SetPWM_Num(TIM2,10);		//设置计数值
//		}
//	}
	
	

}


#endif
