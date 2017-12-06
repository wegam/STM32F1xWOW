#ifdef PC018V10

#include "PC018V10.H"

#include "DS2401.h"	


#include "stm32f10x_exti.h"


#include "STM32_SPI.H"
#include "STM32_PWM.H"
#include "STM32_GPIO.H"
#include "STM32_SYS.H"
#include "STM32_WDG.H"
#include "STM32_SYSTICK.H"
#include "STM32F10x_BitBand.H"


#include	"stdio.h"			//用于printf
#include	"string.h"		//用于printf
#include	"stdarg.h"		//用于获取不确定个数的参数
#include	"stdlib.h"		//malloc动态申请内存空间


u8 ch[120]="USART_BASIC_Configuration(USART_TypeDef* USARTx,u32 USART_BaudRate,u8 NVICPreemptionPriority,u8 NVIC_SubPriority)\n";
u8 ch2[17]={0xC0,0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71};
u8 ch3[17]={0xC0,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
u8 ch4[17]={0xC0};
u8 SegArr[16]={0x00};

u32	SYSTIME	=	0;
u32	DATA	=	0;

/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
u8 Seg7_Code[]=
{
//	0xC0,			//A0:起始地址
	0x3F,			//A0:0
	0x06,			//A1:1
	0x5B,			//A2:2
	0x4F,			//A3:3
	0x66,			//A4:4
	0x6D,			//A5:5
	0x7D,			//A6:6
	0x07,			//A7:7
	0x7F,			//A8:8
	0x6F,			//A9:9
	0x77,			//A10:A
	0x7C,			//A11:B
	0x39,			//A12:C
	0x5E,			//A13:D
	0x79,			//A14:E
	0x71,			//A15:F
	0x50,			//A16:r
	0x5C,			//A17:o
	0x54,			//A18:n
	0x58,			//A19:c
	0x1C,			//A20:u
	0x38,			//A21:L
	0x73,			//A22:P
	0x40,			//A23:-
	0x30			//A24:I
};

//u8 itf=0;

/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC018V10_Configuration(void)
{
	SYS_Configuration();					//系统配置---打开系统时钟 STM32_SYS.H
	
	GPIO_DeInitAll();							//将所有的GPIO关闭----V20170605
	
	SysTick_Configuration(1000);	//系统嘀嗒时钟配置72MHz,单位为uS
	
	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);

//	GPIO_Configuration(GPIOB,GPIO_Pin_4,GPIO_Mode_Out_PP,GPIO_Speed_50MHz);			//GPIO配置
	
	
//	PWM_Configuration(TIM2,7200,10000,50);

//	USART_DMA_Configuration(USART1,115200,1,1,(u32*)Usart_Test_Buffer,(u32*)Usart_Test_Buffer,DMA1_BufferSize);	//USART_DMA配置

	STM32_SPI_ConfigurationNR(SPI2);
//	SPI_DMA_Configuration(SPI2,&SPI_InitStructure,(u32*)SPI_Buffer,(u32*)SPI_Buffer,SPI_BUFFERSIZE);		//SPI_DMA配置

	

//	PWM_Configuration(TIM2,7200,200,20);
	STM32_SPI_ReadWriteData(SPI2,0x8F);
	STM32_SPI_ReadWriteData(SPI2,0x40);
//	STM32_SPI_ReadWriteData(SPI1,0xC0);
	
	ch3[0]=0xC0;
	ch4[0]=0xC0;
	
	Dallas_Init();

}
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC018V10_Server(void)
{
	SYSTIME++;
	if(SYSTIME>=10)
	{
		SYSTIME	=	0;
		DATA++;
		if(DATA>9999)
			DATA	=0;
//		STM32_SPI_ReadWriteData(SPI2,0x40);
//		
//		ch4[1]=ch2[DATA/1000+1];
//		ch4[3]=ch2[DATA%1000/100+1];
//		ch4[5]=ch2[DATA%100/10+1];
//		ch4[7]=ch2[DATA%10+1];
//		
////		STM32_SPI_ReadWriteData(SPI2,0x8F);
////		STM32_SPI_ReadWriteData(SPI2,0x40);
//		STM32_SPI_SendBuffer(SPI2,8,ch4);
		WriteNumSeg7(DATA);		//向数码管写入数据
		Dallas_Init();
//		WriteStatus(3);			//向数码管写入数据
	}
	
//	if(SYSTIME	==	0)
//	{
//		STM32_SPI_SendBuffer(SPI1,120,ch);
//	}
	
//	STM32_SPI_ReadWriteData(SPI2,0x8F);
//	STM32_SPI_ReadWriteData(SPI2,0x40);
//	STM32_SPI_SendBuffer(SPI2,4,ch3);
	
//	if(SYSTIME%3	==	0)
//	{
//		STM32_SPI_SendBuffer(SPI2,8,ch2);
//	}
//	else if(SYSTIME%3	==	1)
//	{
//		STM32_SPI_SendBuffer(SPI2,8,ch3);
//	}
//	else if(SYSTIME%3	==	2)
//	{
//		STM32_SPI_SendBuffer(SPI2,8,ch4);
//	}

}

/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void WriteNumSeg7(u16 Num)		//向数码管写入数据
{
	STM32_SPI_ReadWriteData(SPI2,0x8F);		//亮度
	STM32_SPI_ReadWriteData(SPI2,0x40);		//地址自增
	ch4[0]	=	0xC0;												//起始地址
	if(Num/1000	!=	0)
	{
		ch4[1]=Seg7_Code[Num/1000];							//
		ch4[3]=Seg7_Code[Num%1000/100];
		ch4[5]=Seg7_Code[Num%100/10];
	}
	else if(Num/100	!= 0)
	{
		ch4[1]=0x00;							//
		ch4[3]=Seg7_Code[Num%1000/100];
		ch4[5]=Seg7_Code[Num%100/10];
	}
	else if(Num/10	!= 0)
	{
		ch4[1]=0x00;							//
		ch4[3]=0x00;
		ch4[5]=Seg7_Code[Num%100/10];
	}
	else
	{
		ch4[1]=0x00;							//
		ch4[3]=0x00;
		ch4[5]=0x00;
	}
	ch4[7]=Seg7_Code[Num%10];	
	
	STM32_SPI_SendBuffer(SPI2,8,ch4);			//发送数据

}
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
*******************************************************************************/
void WriteStatus(char StatusCode)		//向数码管写入状态
{
	
	if(StatusCode	==	0)
	{
		return;
	}
	//状态1-------------"P-on"	//上电，未读取地址
	else if(StatusCode	==	1)	
	{
		STM32_SPI_ReadWriteData(SPI2,0x8F);		//亮度
		STM32_SPI_ReadWriteData(SPI2,0x40);		//地址自增
		ch4[1]=Seg7_Code[22];							//
		ch4[3]=Seg7_Code[23];
		ch4[5]=Seg7_Code[17];
		ch4[7]=Seg7_Code[18];
		STM32_SPI_SendBuffer(SPI2,8,ch4);			//发送数据
	}
	//状态2-------------"ID-n"	//显示此窗口号
	else if(StatusCode	==	2)	
	{
		STM32_SPI_ReadWriteData(SPI2,0x8F);		//亮度
		STM32_SPI_ReadWriteData(SPI2,0x40);		//地址自增
		ch4[1]=Seg7_Code[24];							//
		ch4[3]=Seg7_Code[13];
		ch4[5]=Seg7_Code[23];
		ch4[7]=Seg7_Code[6];
		STM32_SPI_SendBuffer(SPI2,8,ch4);			//发送数据
	}
	//状态3-------------"uP--"	//升级中
	else if(StatusCode	==	3)	
	{
		STM32_SPI_ReadWriteData(SPI2,0x8F);		//亮度
		STM32_SPI_ReadWriteData(SPI2,0x40);		//地址自增
		ch4[1]=Seg7_Code[20];							//
		ch4[3]=Seg7_Code[22];
		ch4[5]=Seg7_Code[23];
		ch4[7]=Seg7_Code[23];
		STM32_SPI_SendBuffer(SPI2,8,ch4);			//发送数据
	}
	else
	{
		return;
	}
}



#endif

