#ifdef PL013V11

#include "PL013V11.H"
#include "CD4511.H"



#include "SWITCHID.H"
#include "STM32F10x_BitBand.H"
#include "STM32_GPIO.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_WDG.H"
#include "STM32_PWM.H"



#include "STM32_USART.H"
#include "STM32_TOOLS.H"		//数据校验工具


#include "string.h"				//串和内存操作函数头文件



//************485通讯数据


SWITCHID_CONF	SWITCHID;			//拔码开关
RS485_TypeDef	RS485_Bus;				//总线通讯485
sReceFarmeDef	sReceFarmeRxd;				//接收结构体---接收缓冲区
sReceFarmeDef	sReceFarmeRev;				//接收结构体---备份缓冲区
sReceFarmeDef	sReceFarmeSrv;				//接收结构体---待处理缓冲区

CmdDef				DspCmd;					//显示命令

//volatile	CD4511_Pindef CD4511_Pin1;		//第一位
//volatile	CD4511_Pindef CD4511_Pin2;		//第二位
//volatile	CD4511_Pindef CD4511_Pin3;		//第三位




u8 Bus485Rev[Bus485DataSize]={0};
u8 Bus485Rxd[Bus485DataSize]={0};
u8 Bus485Txd[Bus485DataSize]={0};


u8 SwitchID=0;				//拔码开关地址 //单元板ID----从右到左依次为增高，最右边为最低位，最高位为1表示测试模式，测试模式下从0到9循环显示

vu8	LongDsp		=	0;			//0--在显示时间内显示，1-常亮,不再显示倒计时
vu32	DSPTIME		=	0;			//显示总时间--单位mS
vu32	DSPCount	=	0;			//显示计时--单位mS
vu32	SYSTIME		=	0;			//循环计时变量
vu32	DSPNum		=	0;			//显示数据







/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PL013V11_Configuration(void)
{
	
	SYS_Configuration();					//系统配置---打开系统时钟 STM32_SYS.H
	
	GPIO_DeInitAll();							//将所有的GPIO关闭----V20170605
	
	DeleyuS(1);										//延时1S		
	
	SWITCHID_Configuration();			//拔码开关初始化及读数
	
	CD4511_Configuration();				//数码管驱动		
	
	RS485_Configuration();				//RS485配置
	
	PWM_OUT(TIM2,PWM_OUTChannel1,1,900);	//PWM设定-20161127版本
	
	IWDG_Configuration(1000);							//独立看门狗配置---参数单位ms
	
	SysTick_Configuration(200);					//系统嘀嗒时钟配置72MHz,单位为uS
	
	CD4511_DisplayClear();		//清除显示
}
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PL013V11_Server(void)
{
	//循环周期1mS
//	u8 status=0;	
	
	IWDG_Feed();											//独立看门狗喂狗	
	SYSTIME++;	
	if(SYSTIME>=10000)	//2秒
	{
		SYSTIME=0;	
		SwitchIdServer();		//检测拔码值有无变化，有则更新ID
	}
	
	RS485_Server();										//RS485接收数据
	
	DataServer();			//处理接收到的有效数据
	if(SYSTIME%5	==	0)	//1ms
	DisplayServer();	//显示服务程序，根据显示命令刷新
	
	
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	函数功能说明
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void SWITCHID_Configuration(void)			//拔码开关初始化及读数
{
	SWITCHID.NumOfSW	=	6;
	//SW1--PA4
	SWITCHID.SW1_PORT	=	GPIOB;
	SWITCHID.SW1_Pin	=	GPIO_Pin_8;
	//SW2--PA5
	SWITCHID.SW2_PORT	=	GPIOB;
	SWITCHID.SW2_Pin	=	GPIO_Pin_9;
	//SW3--PB12
	SWITCHID.SW3_PORT	=	GPIOB;
	SWITCHID.SW3_Pin	=	GPIO_Pin_10;
	//SW4--PB13
	SWITCHID.SW4_PORT	=	GPIOB;
	SWITCHID.SW4_Pin	=	GPIO_Pin_11;
	//SW5--PB14
	SWITCHID.SW5_PORT	=	GPIOB;
	SWITCHID.SW5_Pin	=	GPIO_Pin_12;
	//SW6--PB15
	SWITCHID.SW6_PORT	=	GPIOB;
	SWITCHID.SW6_Pin	=	GPIO_Pin_13;

	
	SWITCHID_Conf(&SWITCHID);		//
	SWITCHID_Read(&SWITCHID);		//
	
	SwitchID	=	(SWITCHID.nSWITCHID)&0x3F;
}
/*******************************************************************************
* 函数名			:	RS485_Configuration
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
void RS485_Configuration(void)			//RS485配置
{
	//============================总线通讯485
	RS485_Bus.USARTx	=	Bus485Port;	
	RS485_Bus.RS485_CTL_PORT	=	Bus485CtlPort;
	RS485_Bus.RS485_CTL_Pin		=	Bus485CtlPin;
	
	RS485_DMA_ConfigurationNR	(&RS485_Bus,Bus485BaudRate,(u32*)&sReceFarmeRxd,Bus485DataSize);	//USART_DMA配置--查询方式，不开中断,配置完默认为接收状态
}
/*******************************************************************************
* 函数名			:	RS485_Configuration
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
void CD4511_Configuration(void)			//数码管驱动
{
	//小数点控制脚配置
	GPIO_Configuration_OPP50	(DPPort,	DPPin);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_ResetBits(DPPort,	DPPin);
	
	GPIO_Configuration_OPP50	(PortA0,PinA0);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(PortA1,PinA1);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(PortA2,PinA2);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(PortA3,PinA3);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	
	GPIO_Configuration_OPP50	(PortEN1,PinEN1);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(PortEN2,PinEN2);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50	(PortEN3,PinEN3);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605	
	
}

/*******************************************************************************
*函数名		:	function
*功能描述	:	函数功能说明
*输入			: 
*输出			:	无
*返回值		:	无
*例程			:
*******************************************************************************/
void CD4511_DISPALY(u8 wei,u16 num)
{
	u32 Num	=	0;
	GPIO_ResetBits(PortEN1,PinEN1);
	GPIO_ResetBits(PortEN2,PinEN2);
	GPIO_ResetBits(PortEN3,PinEN3);
	
	GPIO_ResetBits(DPPort,DPPin);
	
	Num	=	0;
	while(Num++<1000);
	
	
	GPIO_ResetBits(PortA0,PinA0);
//	GPIO_ResetBits(PortA1,PinA1);
	GPIO_ResetBits(PortA2,PinA2);
//	GPIO_ResetBits(PortA3,PinA3);
	
	GPIO_SetBits(PortA1,PinA1);
	GPIO_SetBits(PortA3,PinA3);
	
	Num	=	0;
	while(Num++<1000);
	
	if(wei==0)		//小数点
	{
		if(num==0)		//小数点
		{
			GPIO_SetBits(PortA1,PinA1);
			GPIO_SetBits(PortA3,PinA3);
			GPIO_SetBits(DPPort,DPPin);
			GPIO_SetBits(PortEN1,PinEN1);
		}
		else			//关显示
		{
			GPIO_ResetBits(PortEN1,PinEN1);
			GPIO_ResetBits(PortEN2,PinEN2);
			GPIO_ResetBits(PortEN3,PinEN3);
			GPIO_SetBits(PortA1,PinA1);
			GPIO_SetBits(PortA3,PinA3);
			GPIO_ResetBits(DPPort,DPPin);			
		}
		return;
	}
	else if(wei==1)
	{
		num	=	num%10;
	}
	else if(wei==2)
	{
		num	=	(num%100)/10;
	}
	else if(wei==3)
	{
		num	=	(num%1000)/100;
	}
	
//	//A0
	if((num&0x01)==0x01)
	{
		GPIO_SetBits(PortA0,PinA0);
	}
	else
	{
		GPIO_ResetBits(PortA0,PinA0);
	}
	//A1
	if((num&0x02)==0x02)
	{
		GPIO_SetBits(PortA1,PinA1);
	}
	else
	{
		GPIO_ResetBits(PortA1,PinA1);
	}
	//A2
	if((num&0x04)==0x04)
	{
		GPIO_SetBits(PortA2,PinA2);
	}
	else
	{
		GPIO_ResetBits(PortA2,PinA2);
	}
	//A3
	if((num&0x08)==0x08)
	{
		GPIO_SetBits(PortA3,PinA3);
	}
	else
	{
		GPIO_ResetBits(PortA3,PinA3);
	}

	
	
	Num	=	0;
	while(Num++<1000);
		
	if(wei==3)		//百位
	{
		GPIO_SetBits(PortEN3,PinEN3);
	}
	else if(wei==2)	//十位
	{
		GPIO_SetBits(PortEN2,PinEN2);
	}
	else if(wei==1)	//个位
	{
		GPIO_SetBits(PortEN1,PinEN1);
	}
	else			//关显示
	{
		GPIO_ResetBits(PortEN1,PinEN1);
		GPIO_ResetBits(PortEN2,PinEN2);
		GPIO_ResetBits(PortEN3,PinEN3);
	}


}
/*******************************************************************************
*函数名		:	function
*功能描述	:	函数功能说明
*输入			: 
*输出			:	无
*返回值		:	无
*例程			:
*******************************************************************************/
void CD4511_DisplayClear(void)		//清除显示
{
	CD4511_DISPALY(0,1);
}
/*******************************************************************************
*函数名		:	function
*功能描述	:	函数功能说明
*输入			: 
*输出			:	无
*返回值		:	无
*例程			:
*******************************************************************************/
void CD4511_DisplayDp(void)		//个位显示小数点
{
	CD4511_DISPALY(0,0);				//个位显示小数点
}
/*******************************************************************************
*函数名		:	function
*功能描述	:	函数功能说明
*输入			: 
*输出			:	无
*返回值		:	无
*例程			:
*******************************************************************************/
void RS485_Server(void)
{
	u8	Num	=	0;
	Num	=	RS485_ReadBufferIDLE(&RS485_Bus,(u32*)&sReceFarmeRev,(u32*)&sReceFarmeRxd);
	if(Num	!=	0)	//无数据
	{
		if(sReceFarmeRev.desAddr	==	SwitchID)
		{
			memcpy((u8*)&sReceFarmeSrv,(u8*)&sReceFarmeRev,sizeof(sReceFarmeDef));
//			DataServer();		//处理接收到的有效数据
		}
//		else
//		{
//			memset((u8*)&sReceFarmeRev,0x00,Bus485DataSize);	//清除备份缓冲区
//		}
	}
	
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void DataServer(void)		//处理接收到的有效数据
{
	if(SwitchID==0	||	(SwitchID&0x20)==0x20)		//未拔码
	{
		return;
	}
	if(sReceFarmeSrv.desAddr	==	SwitchID)			//地址数据已加入，表示有新的有效数据
	{
		
		u32	Num	=	0;
		DSPCount	=	0;
		
		sReceFarmeSrv.desAddr	=0;		//删除地址标志
		
		DspCmd	=	sReceFarmeSrv.cmd;			//显示命令
		
		//==============获取显示数值
		Num	|=	sReceFarmeSrv.data[0];
		Num<<=8;
		Num	|=	sReceFarmeSrv.data[1];
		Num<<=8;
		Num	|=	sReceFarmeSrv.data[2];
		Num<<=8;
		Num	|=	sReceFarmeSrv.data[3];
		DSPNum	=		Num;		//

		//==============获取显示时间
		if(sReceFarmeSrv.cmd.DispTime	==0)
		{
			LongDsp	=	1;			//0--在显示时间内显示，1-常亮,不再显示倒计时
		}
		else
		{
			LongDsp	=	0;			//0--在显示时间内显示，1-常亮,不再显示倒计时
			Num	=	0;
			Num	|=	sReceFarmeSrv.data[4];
			Num<<=8;
			Num	|=	sReceFarmeSrv.data[5];
			Num<<=8;
			Num	|=	sReceFarmeSrv.data[6];
			Num<<=8;
			Num	|=	sReceFarmeSrv.data[7];
			DSPTIME	=		Num;		//
		}
		//==============数据取完，清除数据
//		memset((u8*)&sReceFarmeSrv,0x00,Bus485DataSize);	//清除备份缓冲区
	}
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void DisplayServer(void)		//显示服务程序，根据显示命令刷新
{
//	DSPNum	=	1;
//	CD4511_DISPALY((u8)SYSTIME%3+1,(u16)DSPNum);		//刷新数据
//	return;
//	CD4511_DisplayOFF(&CD4511_Pin1);			//关闭显示---关NPN三极管
//	CD4511_DisplayOFF(&CD4511_Pin2);			//关闭显示---关NPN三极管
//	CD4511_DisplayOFF(&CD4511_Pin3);			//关闭显示---关NPN三极管
	
	if(SwitchID==0	||	(SwitchID&0x20)==0x20)		//未拔码或者为测试模式sw6为1表示测试模式
	{
		DSPCount++;
		if(DSPCount>=900)
		{
			DSPCount	=	0;			
		}
		if(DSPCount%300	==	0)
		{
			u8	Num	=	0;
			DSPNum	=	DSPNum%10;
			if(DSPNum	==	9)
			{
				DSPNum	=	0;
			}
			else
			{
				DSPNum++;
				Num	=	DSPNum;
			}
			DSPNum	=	Num*100+Num*10+Num;
		}
		CD4511_DISPALY((u8)DSPCount%3+1,DSPNum);		//刷新数据
		GPIO_ResetBits(DPPort,DPPin);
		return;
	}
	else if(LongDsp	==	1)				//0--在显示时间内显示，1-常亮,不再显示倒计时，此时小数点不显示
	{
		DSPCount++;
		if(DSPCount>=1000)
		{
			DSPCount	=	0;
		}
		if(DspCmd.DispEnNum)		//显示数值	：	0-不显示，		1-显示，此时小数点不显示
		{
//			CD4511_DISPALY((u8)DSPCount%3+1,DSPNum);		//刷新数据
			if(DspCmd.DispMdNum)	//数值模式	：	0-静态显示，	1-0.5S闪烁
			{
				if((DSPCount%1000)>500)
				{
					CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//刷新数据
				}
				else
				{
					CD4511_DISPALY(0,0);						//不显示
				}
			}
			else
			{
				CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//刷新数据
			}
		}
		else if(DspCmd.DispEnDp)		//显示点		：	0-不显示，		1-显示
		{
			if(DspCmd.DispMdDp)			//点模式		：	0-静态显示，	1-0.5S闪烁
			{
				if((DSPCount%1000)<500)
				{
					CD4511_DISPALY(0,1);		//显示小数点
				}
				else
				{
					CD4511_DISPALY(0,0);		//关闭小数点
				}
			}
			else
			{
				CD4511_DisplayDp();		//个位显示小数点
			}
		}
	}
	else
	{
		if(DSPTIME	==0)
		{
			return;
		}
		if(DSPCount>=DSPTIME)		//显示时间已用完，需要关闭显示
		{
			DSPTIME	=	0;
			DSPCount=	0;
			CD4511_DisplayClear();		//清除显示
		}
		if((DspCmd.DispEnNum)&&DSPTIME)		//显示数值	：	0-不显示，		1-显示，此时小数点不显示
		{
			DSPCount	++;
			if(DspCmd.DispMdNum)	//数值模式	：	0-静态显示，	1-0.5S闪烁
			{
				if((DSPCount%1000)>500)
				{
					CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//刷新数据
				}
				else
				{
					CD4511_DISPALY(0,0);						//不显示
				}
			}
			else		//不闪烁
			{
				CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//刷新数据
			}
		}
		else if(DspCmd.DispEnDp==1)		//显示点		：	0-不显示，		1-显示
		{
			if(DspCmd.DispMdDp	&&	DSPCount%500==0)				//点模式		：	0-静态显示，	1-0.5S闪烁
			{
				GPIO_Toggle	(DPPort,	DPPin);		//将GPIO相应管脚输出翻转----V20170605
			}
		}
	}	
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void SwitchIdServer(void)		//检测拔码值有无变化
{
	SWITCHID_Read(&SWITCHID);		//
	if(SwitchID	!=	(SWITCHID.nSWITCHID)&0x3F)
	{
		SwitchID	=	(SWITCHID.nSWITCHID)&0x3F;
		CD4511_DisplayClear();		//清除显示
	}
}

#endif
