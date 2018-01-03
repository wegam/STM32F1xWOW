#ifdef TIM_TEST
#include "TIM_TEST.H"

#include "string.h"
#include "math.h"


#include "STM32_EXTI.H"
#include "STM32_USART.H"
#include "STM32_TIM.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_GPIO.H"
#include "STM32F10x_BitBand.H"

#define RxBufferSize	16
#define Ncycle	0

PWM_TimDef 	PWM_Tim;
volatile GPIO_TypeDef* 	GPIOx;			//x=A/B/C/D/E/F/G
volatile TIM_TypeDef* 	TIMx;
volatile u32* p	=	0;
volatile u32 temp	=	0;
u16 systime	=	0;
volatile u16 Pin	=	0;

u16	RunFrequency			=	0;		//��ǰ����Ƶ��
/*******************************************************************************
* ������		:	
* ��������	:	 
* ����		:	
* ���		:
* ���� 		:
*******************************************************************************/
void TIM_TEST_Configuration(void)
{
	SYS_Configuration();				//ϵͳ����
	GPIO_DeInitAll();													//�����е�GPIO�ر�----V20170605
//	SysTick_Configuration(1000);	//ϵͳ���ʱ������72MHz,��λΪuS
	GPIO_Configuration_OPP50	(GPIOA,	GPIO_Pin_7);			//��GPIO��Ӧ�ܽ�����ΪAPP(��������)���ģʽ������ٶ�50MHz----V20170605
	TIM_ConfigurationFreq(TIM1,40000);	//��ʱ��Ƶ�����÷�ʽ����СƵ��0.01Hz,���100KHz
	
	TIM1->RCR = 100-1;
	
	TIM_Cmd(TIM1, ENABLE); 									//ʹ��TIM
	
	RunFrequency			=	500;		//��ǰ����Ƶ��
}
/*******************************************************************************
* ������		:	
* ��������	:	 
* ����		:	
* ���		:
* ���� 		:
*******************************************************************************/
void TIM_TEST_Server(void)
{
//	RunFrequency	++;		//��ǰ����Ƶ��
//	if(RunFrequency>=5000000)
//		RunFrequency	=500;
	temp	--;		//��ǰ����Ƶ��
	if(temp<2)
	{
		temp	=100;
//		TIM1->RCR = temp;
	}
	if(temp%2	==	0)
	{
		TIM1->RCR = temp-1;
	}
	
	RunFrequency	++;		//��ǰ����Ƶ��
	if(RunFrequency>=50000)
		RunFrequency	=500;
	
	if(RunFrequency%10	==	0)
	{
//		TIM1->RCR = temp;
	TIM_SetFreq(TIM1,RunFrequency);		//�趨Ƶ��
	}
	GPIO_Toggle	(GPIOA,	GPIO_Pin_7);		//��GPIO��Ӧ�ܽ������ת----V20170605
}


#endif