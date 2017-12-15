/******************************** User_library *********************************
* 文件名 	: SPI_FLASH.C
* 作者   	: wegam@sina.com
* 版本   	: V
* 日期   	: 2017/04/16
* 说明   	: 
********************************************************************************
SPI_FLASH使用功能测试
1）需要宏定义 SPI_FLASH
2）使用USB_TEST 板测试
3）需要宏定义SPI引脚

*******************************************************************************/
#ifdef IAP							//如果定义了A3987_TEST 此功能生效

#include "cortexm3_macro.h"

#include "STM32F10x_BitBand.H"


#include	"stdio.h"				//用于printf
#include	"string.h"			//用于printf
#include	"stdarg.h"			//用于获取不确定个数的参数
#include	"stdlib.h"			//malloc动态申请内存空间
	
#include	"stddef.h"
#include	"stdint.h"



#include "IAP.H"
#include "STM32_PWM.H"
#include "STM32_SYS.H"
#include "STM32_USART.H"
#include "STM32_SYSTICK.H"
#include "STM32_FLASH.H"








#define FLASH_APP1_ADDR		0x08006000  //用户程序的执行地址
#define FLASH_APP2_ADDR   0x08013000  //存储接收到的用户程序的地址
#define FLASH_DATA_ADDR   0x0801E004  //存储用户数据的地址





#define BufferSize	STM_SECTOR_SIZE/2
u16 RevBuffer[BufferSize]={0};
u16 RxdBuffer[BufferSize]={0};
u16 TxdBuffer[BufferSize]={0};


u8 PUL_FLG=0;
u16 a	=	0;
u16	b	=	0;
u16	c	=	0;

void SET_MSP(u32 addr) ;			//设置栈顶地址---调用汇编
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void IAP_Configuration(void)
{
	
	SYS_Configuration();											//系统配置 STM32_SYS.H	
	
//	PWM_Configuration(TIM2,7200,10000,50);		//运行指示灯配置
	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);
		
	SysTick_Configuration(500);							//系统嘀嗒时钟配置72MHz,单位为uS
	
	USART_DMA_ConfigurationNR	(USART1,115200,(u32*)RxdBuffer,BufferSize);	//USART_DMA配置--查询方式，不开中断
	
//	SET_MSP(0X21000000);
	
	a	=	3;
	b	=	2;
	
	c	=	a/b;
}

/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void IAP_Server(void)
{	
	u16	Num	=	0;
	Num	=	USART_ReadBufferIDLE(USART1,(u32*)RevBuffer,(u32*)RxdBuffer);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		memcpy(TxdBuffer,RevBuffer,Num);
		USART_DMASend(USART1,(u32*)TxdBuffer,Num);	//串口DMA发送程序
	}
}

/*******************************************************************************
*函数名			:	function
*功能描述		:	设置栈顶地址---调用汇编
*输入				: addr:栈顶地址
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void SET_MSP(u32 addr) 
{
    __MSR_MSP(addr);
}




































#endif
