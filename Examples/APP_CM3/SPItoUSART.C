#ifdef SPItoUSART

#include "SPItoUSART.H"
#include "stm32f10x_exti.h"


#include "STM32_SPI.H"
#include "STM32_PWM.H"
#include "STM32_GPIO.H"
#include "STM32_SYS.H"
#include "STM32_WDG.H"
#include "STM32_SYSTICK.H"
#include "STM32F10x_BitBand.H"


u8 ch[120]="USART_BASIC_Configuration(USART_TypeDef* USARTx,u32 USART_BaudRate,u8 NVICPreemptionPriority,u8 NVIC_SubPriority)\n";


u32	SYSTIME	=	0;

//u8 itf=0;
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void SPItoUSART_Configuration(void)
{
	SYS_Configuration();					//系统配置---打开系统时钟 STM32_SYS.H
	
	GPIO_DeInitAll();							//将所有的GPIO关闭----V20170605
	
	SysTick_Configuration(1000);	//系统嘀嗒时钟配置72MHz,单位为uS
	
	PWM_OUT(TIM2,PWM_OUTChannel1,1,20);

//	GPIO_Configuration(GPIOB,GPIO_Pin_4,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);			//GPIO配置
	
	
//	PWM_Configuration(TIM2,7200,10000,50);

//	USART_DMA_Configuration(USART1,115200,1,1,(u32*)Usart_Test_Buffer,(u32*)Usart_Test_Buffer,DMA1_BufferSize);	//USART_DMA配置

	STM32_SPI_ConfigurationNR(SPI1);
//	SPI_DMA_Configuration(SPI2,&SPI_InitStructure,(u32*)SPI_Buffer,(u32*)SPI_Buffer,SPI_BUFFERSIZE);		//SPI_DMA配置

	

//	PWM_Configuration(TIM2,7200,200,20);

}
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void SPItoUSART_Server(void)
{
	SYSTIME++;
	if(SYSTIME>=1000)
	{
		SYSTIME	=	0;
	}
	
	if(SYSTIME	==	0)
	{
		STM32_SPI_SendBuffer(SPI1,120,ch);
	}

}


#endif

