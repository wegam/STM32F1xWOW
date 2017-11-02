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

u16 systime	=	0;
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
	PWM_OUT_COUNT(TIM2,PWM_OUTChannel1,10,500);		//sys_led
	
//	GPIO_Configuration_IPD(GPIOA,	GPIO_Pin_4);			//将GPIO相应管脚配置为上拉输入模式----V20170605--BUTTON
//	GPIO_Configuration_IPD(GPIOA,	GPIO_Pin_5);			//将GPIO相应管脚配置为上拉输入模式----V20170605
//	GPIO_Configuration_IPD(GPIOA,	GPIO_Pin_6);			//将GPIO相应管脚配置为上拉输入模式----V20170605
	
//	PWM_OUT(TIM2,PWM_OUTChannel1,5,500);		//sys_led
//	PWM_OUT(TIM3,PWM_OUTChannel1,20000,30);		//PWM设定
//	PWM_OUT(TIM4,PWM_OUTChannel1,20000,40);		//PWM设定
//	
//	PWM_OUT(TIM1,PWM_OUTChannel2,20000,50);		//PWM设定
//	PWM_OUT(TIM2,PWM_OUTChannel2,20000,500);	//PWM设定
	SetPWM_Num(TIM2,10);		//设置计数值

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
	PWM_CountServer();
	systime++;
	if(systime>=500000)
	{
		systime	=	0;
		SetPWM_Num(TIM2,10);		//设置计数值
	}
	
	

}


#endif
