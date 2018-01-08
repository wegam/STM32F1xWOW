#ifdef PC016V20

#include "PC016V20.H"
#include "HC_PHY.H"


sCommunicationDef	gProtocolBuffer;

SWITCHID_CONF	SWITCHID;			//拔码开关
LockFlagDef	LockFlag;				//锁控制标志0--不处理，1--开锁操作
//==============485结体体
RS485_TypeDef	RS485_Bus;				//总线通讯485
RS485_TypeDef	RS485_Seg7;				//数码管显示485

//==============485缓冲区
sCommunicationDef	CommData;			//总线485数据结构体
//sCommunicationDef	CommData1;			//总线485数据结构体---端口1
//sCommunicationDef	CommData2;			//总线485数据结构体---端口2
//sCommunicationDef	CommData3;			//总线485数据结构体---端口3
//sCommunicationDef	CommData4;			//总线485数据结构体---端口4
u8 RS485BusTxd[Bus485BufferSize]	=	{0};
u8 RS485BusRxd[Bus485BufferSize]	=	{0};
u8 RS485BusRev[Bus485BufferSize]	=	{0};

//==============读卡器缓冲区
ReadCmdDef	ReadCmd	=		//读卡命令
{
	0xE1,		/* F-Head 	*/			//为帧头，上位机下发时固定为0XE1，下位机应答时固定为0XD2；
	0x24, 	/* CmdType 	*/			//命令类型
	0x01,		/* Sector 	*/			//1字节扇区号
	0x01, 	/* Block 		*/			//1字节块号
	0x60, 	/* KeyType	*/			//1字节密钥模式 KEYA（0x60）/KEYB(0x61)
	0xFF,		//6字节密码
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0xFF,
	0x1C,		/* Crc16-HIGN */
	0x0D,		/* Crc16-LOW */
	0x1E 		/* F-End*/						//为帧尾，上位机下发时固定为0X1E，下位机应答时固定为0X2D；
};

PortFlagDef	PortFlag;			//端口状态--：有卡/无卡
ICDataDef	ICData;					//4个端口IC数据

			
u8 ICCardReadRxd[ICCardReadBufferSize]={0};
u8 ICCardReadRev[ICCardReadBufferSize]={0};

ICBufferDef ICCardReadRxd1;
ICBufferDef ICCardReadRev1;

ICBufferDef ICCardReadRxd2;
ICBufferDef ICCardReadRev2;

ICBufferDef ICCardReadRxd3;
ICBufferDef ICCardReadRev3;

ICBufferDef ICCardReadRxd4;
ICBufferDef ICCardReadRev4;

u8 ICCardReadCount1	=	0;			//
u8 ICCardReadCount2	=	0;
u8 ICCardReadCount3	=	0;
u8 ICCardReadCount4	=	0;

//==============其它变量
u8 SwitchData	=	0;				//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
u32	SYSTIME	=	0;
u32	LockOnTime[4]	=	{0};	//锁吸合时间，如果时间为0则释放锁






//u8 itf=0;

/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC016V20_Configuration(void)
{
	SYS_Configuration();						//系统配置---打开系统时钟 STM32_SYS.H	
	Lock_Configuration();						//锁初始化
	SWITCHID_Configuration();				//拔码开关初始化及读数
	ICCardReader_Configuration();		//读卡器配置
	RS485_Configuration();					//RS485配置
	
	SysTick_DeleyS(2);							//SysTick延时nS	

	
	
	//==================上电读卡
	ICCardReader_Read(1);					//读相应端口的卡
	SysTick_DeleymS(100);					//SysTick延时nmS
	ICCardReader_Read(2);					//读相应端口的卡
	SysTick_DeleymS(100);					//SysTick延时nmS
	ICCardReader_Read(3);					//读相应端口的卡
	SysTick_DeleymS(100);					//SysTick延时nmS
	ICCardReader_Read(4);					//读相应端口的卡
	SysTick_DeleymS(100);					//SysTick延时nmS
	
	
	//==================初始化其它

	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);
	IWDG_Configuration(1000);					//独立看门狗配置---参数单位ms
	SysTick_Configuration(1000);	//系统嘀嗒时钟配置72MHz,单位为uS	
//	SYSTIME	=	sizeof(sCommunicationDef);	
}
/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC016V20_Server(void)
{
	if(ICCardReader_Server())							//RS485收发处理
	{
		return;
	}
	
	
	IWDG_Feed();								//独立看门狗喂狗
	SYSTIME++;
	if(SYSTIME>=1000)
	{
		SYSTIME	=	0;
		ResetData();						//重置相关变量
		memset((u8*)&ICCardReadRev4,0x00,22);
//		ICCardReader_Read(4);						//读相应端口的卡
//		ICCardReadRev[0]=0xA5;
//		ICCardReadRev[1]=0x01;
//		ICCardReadRev[2]=0x02;
//		USART_DMASend	(ICCardReadPort2,(u32*)ICCardReadRev,1);	//RS485-DMA发送程序
	}
	Lock_Server();
	RS485_Server();						//RS485收发处理
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
	RS485_Bus.USARTx	=	Bus485Port;
	
	RS485_Bus.RS485_CTL_PORT	=	Bus485CtlPort;
	RS485_Bus.RS485_CTL_Pin		=	Bus485CtlPin;
	
	RS485_DMA_ConfigurationNR	(&RS485_Bus,Bus485BaudRate,(u32*)RS485BusRxd,Bus485BufferSize);	//USART_DMA配置--查询方式，不开中断,配置完默认为接收状态
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
void ICCardReader_Configuration(void)			//选择相应端口读卡器配置
{
	USART_DMA_ConfigurationNR	(ICCardReadPort1,ICCardReadBaudRate,(u32*)&ICCardReadRxd,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort3,ICCardReadBaudRate,(u32*)&ICCardReadRxd3,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_ConfigurationIT(ICCardReadPort4,ICCardReadBaudRate,0,0);	//USART_配置---常规中断方式//UART5不支持DMA传输
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
	SWITCHID.NumOfSW	=	8;
	//SW1--PA4
	SWITCHID.SW1_PORT	=	GPIOA;
	SWITCHID.SW1_Pin	=	GPIO_Pin_4;
	//SW2--PA5
	SWITCHID.SW2_PORT	=	GPIOA;
	SWITCHID.SW2_Pin	=	GPIO_Pin_5;
	//SW3--PB12
	SWITCHID.SW3_PORT	=	GPIOA;
	SWITCHID.SW3_Pin	=	GPIO_Pin_6;
	//SW4--PB13
	SWITCHID.SW4_PORT	=	GPIOA;
	SWITCHID.SW4_Pin	=	GPIO_Pin_7;
	//SW5--PB14
	SWITCHID.SW5_PORT	=	GPIOC;
	SWITCHID.SW5_Pin	=	GPIO_Pin_4;
	//SW6--PB15
	SWITCHID.SW6_PORT	=	GPIOC;
	SWITCHID.SW6_Pin	=	GPIO_Pin_5;
	//SW7--PB0
	SWITCHID.SW7_PORT	=	GPIOB;
	SWITCHID.SW7_Pin	=	GPIO_Pin_0;
	//SW8--PB1
	SWITCHID.SW8_PORT	=	GPIOB;
	SWITCHID.SW8_Pin	=	GPIO_Pin_1;
	
	SWITCHID_Conf(&SWITCHID);		//
	SWITCHID_Read(&SWITCHID);		//
	
	SwitchData	=	(SWITCHID.nSWITCHID)&0xFF;
}
/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void Lock_Configuration(void)			//锁初始化
{
	GPIO_ResetBits(LockPort4,	LockPin4);
	GPIO_Configuration_OPP50(LockPort1,	LockPin1);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50(LockPort2,	LockPin2);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50(LockPort3,	LockPin3);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50(LockPort4,	LockPin4);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_ResetBits(LockPort1,	LockPin1);
	GPIO_ResetBits(LockPort2,	LockPin2);
	GPIO_ResetBits(LockPort3,	LockPin3);
	GPIO_ResetBits(LockPort4,	LockPin4);	
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
void Lock_Server(void)
{
	u8	Num	=	0;
	for(Num	=0;Num<4;Num++)
	{
		if(LockOnTime[Num]>0)
		{
			LockOnTime[Num]--;
			Lock_On(Num+1);			//开锁
		}
		else
		{
			Lock_Off(Num+1);		//释放锁
		}
	}
}

/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
void RS485_Server(void)							//RS485收发处理
{
	u16 Num	=	0;
	Num	=	RS485_ReadBufferIDLE(&RS485_Bus,(u32*)&CommData,(u32*)RS485BusRxd);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	//================获取指定IC号命令
	if(	Num	
			&&	CommData.sHead.eHead1	==eCommHead1	//识别符1	0xFA
			&&	CommData.sHead.eHead2	==eCommHead2	//识别符2	0xF5
			&&	CommData.sStart.eCmd	==eCommCmd17	//获取指定IC号命令
			&&	CommData.sAddr.Addr2 	==SwitchData	//层号
		)			//接收到数据并且标识符检测正确
	{
		ICCardDataPacket();											//数据打包--对获取IC卡号命令的处理
		RS485_DMASend(&RS485_Bus,(u32*)&CommData,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+CommData.sStart.nLength);	//RS485-DMA发送程序
	}
	//================开锁命令
	else if(Num	
					&&	CommData.sHead.eHead1	==eCommHead1		//识别符1	0xFA
					&&	CommData.sHead.eHead2	==eCommHead2		//识别符2	0xF5
					&&	CommData.sStart.eCmd	==eCommCmd18		//开锁取药箱命令
					&&	CommData.sAddr.Addr2 	==SwitchData+1	//层号：由于锁是跨层板控制，所以地址需要+1
					)			//接收到数据并且标识符检测正确
	{		
		LockOnTimeSet();												//设置锁吸合时间
		RS485_DMASend(&RS485_Bus,(u32*)&CommData,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+CommData.sStart.nLength);	//RS485-DMA发送程序
	}
}
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
u8 ICCardReader_Server(void)							//读取IC卡数据
{
	u8	Num	=	0;
	//==============================Port1
	Num	=	USART_ReadBufferIDLE(ICCardReadPort1,(u32*)&ICCardReadRev1,(u32*)&ICCardReadRxd1);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num	==	22)	//无卡
	{
		PortFlag.Port1Flg	=	1;
		memcpy(ICData.Port1Data,ICCardReadRev1.data,16);		//将卡号保存
		memset((u8*)&ICCardReadRev1,0x00,22);
	}
	else if(Num	==	6)
	{
		PortFlag.Port1Flg	=	0;
		memset(ICData.Port1Data,0x00,16);		//将卡号清除
	}
	//==============================Port2
	Num	=	USART_ReadBufferIDLE(ICCardReadPort2,(u32*)&ICCardReadRev2,(u32*)&ICCardReadRxd2);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num	==	22)	//无卡
	{
		PortFlag.Port2Flg	=	1;
		memcpy(ICData.Port2Data,ICCardReadRev2.data,16);		//将卡号保存
		memset((u8*)&ICCardReadRev2,0x00,22);
	}
	else if(Num	==	6)
	{
		PortFlag.Port2Flg	=	0;
		memset(ICData.Port2Data,0x00,16);		//将卡号清除
	}
	//==============================Port3
	Num	=	USART_ReadBufferIDLE(ICCardReadPort3,(u32*)&ICCardReadRev3,(u32*)&ICCardReadRxd3);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num	==	22)	//
	{
		PortFlag.Port3Flg	=	1;
		memcpy(ICData.Port3Data,ICCardReadRev3.data,16);		//将卡号保存
		memset((u8*)&ICCardReadRev3,0x00,22);
	}
	else if(Num	==	6)//无卡
	{
		PortFlag.Port3Flg	=	0;
		memset(ICData.Port3Data,0x00,16);		//将卡号清除
	}
	//==============================Port4
	if(USART_GetITStatus(ICCardReadPort4, USART_IT_RXNE))
  {
		USART_ClearITPendingBit(ICCardReadPort4, USART_IT_RXNE);
    ICCardReadRxd[ICCardReadCount4]	=	USART_ReceiveData(ICCardReadPort4);
		ICCardReadCount4++;
		
		if(ICCardReadCount4==6	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24	&&	ICCardReadRxd[5]==0x2D)
		{
			PortFlag.Port4Flg	=	0;
			memset(ICData.Port4Data,0x00,16);		//将卡号清除
			ICCardReadCount4=0;
		}
		if(ICCardReadCount4>=22)
		{
			PortFlag.Port4Flg	=	1;
			memcpy(ICData.Port4Data,ICCardReadRxd+3,16);		//将卡号保存
			memset((u8*)&ICCardReadRxd,0x00,22);
//			memcpy((u8*)&ICCardReadRev4,ICCardReadRxd,3);
//			memcpy((u8*)&ICCardReadRev4.data,ICCardReadRxd+3,16);
//			memcpy((u8*)&ICCardReadRev4.CRC16,ICCardReadRxd+19,2);
//			ICCardReadRev4.End	=	ICCardReadRxd[21];
			ICCardReadCount4=0;
		}
		return 1;
  }
	else if(USART_GetITStatus(ICCardReadPort4, USART_IT_TC))
  {   
    USART_ClearITPendingBit(ICCardReadPort4, USART_IT_TC);
		return 1;
  }	
	return 0;
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
void ICCardReader_Read(u8 Num)							//向相应端口发送读卡指令
{
	if(Num	==	0)
	{
		return;
	}
	else if(Num	==	1)
	{
		USART_DMASend	(ICCardReadPort1,(u32*)&ReadCmd,14);	//串口DMA发送程序
	}
	else if(Num	==	2)
	{
		USART_DMASend	(ICCardReadPort2,(u32*)&ReadCmd,14);	//串口DMA发送程序
	}
	else if(Num	==	3)
	{
		USART_DMASend	(ICCardReadPort3,(u32*)&ReadCmd,14);	//串口DMA发送程序
	}
	else if(Num	==	4)
	{
		USART_Send(ICCardReadPort4,(u8*)&ReadCmd,14);
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
void ICCardDataPacket(void)		//数据打包--对获取IC卡号命令的处理
{
	CommData.sStart.eCmd	=	(eCommCmdDef)eCommAck146;						//获取指定IC号
	CommData.sStart.nLength	=	4+0;		//数据段长度：地址+状态码+Data--->先清零数据段长度，如果有数据，则会增加ICCardDataSize长度数据
	
	//==============================复制数据
	if(CommData.sAddr.Addr3==1	&&	PortFlag.Port1Flg!=0)						//端口1IC卡号并且有卡号
	{
			memcpy(CommData.data,ICData.Port1Data,ICCardDataSize);	//复制卡号
			CommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data	
	}
	else if(CommData.sAddr.Addr3==2	&&	PortFlag.Port2Flg!=0)				//端口2IC卡号并且有卡号
	{
		memcpy(CommData.data,ICData.Port2Data,ICCardDataSize);	//复制卡号
		CommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data
	}
	else if(CommData.sAddr.Addr3==3	&&	PortFlag.Port3Flg!=0)				//端口3IC卡号并且有卡号
	{
		memcpy(CommData.data,ICData.Port3Data,ICCardDataSize);	//复制卡号
		CommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data
	}
	else if(CommData.sAddr.Addr3==4	&&	PortFlag.Port4Flg!=0)				//端口4IC卡号并且有卡号
	{
		memcpy(CommData.data,ICData.Port4Data,ICCardDataSize);	//复制卡号
		CommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data
	}
	//==============================根据有无数据更新状态码
	if(CommData.sStart.nLength>4)						//CommData.data内有数据
	{
		CommData.eErrorCode		=	eCommErr00;													//无错误
	}
	else																		//CommData.data内无数据
	{
		CommData.eErrorCode		=	eCommErr35;											//无药箱数据
	}
	//==============================数据校验
	CommData.sStart.nBcc8	=	BCC8((u8*)&(CommData.sAddr.Addr1),CommData.sStart.nLength);
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
void LockOnTimeSet(void)			//设置锁吸合时间
{
	//==============================根据相应端口设置相应时间
	if(CommData.data[0]	==	0)			//吸合时间为0表示使用默认值
	{
		LockOnTime[CommData.sAddr.Addr3]	=	DefaultOnTime;		//默认锁吸合时间
	}
	else if(CommData.data[0]	>	MaxOnTime/1000)					//吸合时间超出最大时间
	{
		LockOnTime[CommData.sAddr.Addr3]	=	MaxOnTime;		//锁吸合最大时间
	}
	else
	{
		LockOnTime[CommData.sAddr.Addr3]	=	((u32)CommData.data[0])*1000;		//锁吸合时间
	}
	//==============================根据相应端口设置相应锁标志：开锁后需要检测药箱有无提走
	if(CommData.sAddr.Addr3	==	1)
	{
		LockFlag.Lock1	=	1;
	}
	else if(CommData.sAddr.Addr3	==	2)
	{
		LockFlag.Lock2	=	1;
	}
	else if(CommData.sAddr.Addr3	==	3)
	{
		LockFlag.Lock3	=	1;
	}
	else if(CommData.sAddr.Addr3	==	4)
	{
		LockFlag.Lock4	=	1;
	}
	//==============================封装应答数据
	CommData.sStart.eCmd	=	(eCommCmdDef)eCommAck147;			//开锁取药箱
	CommData.eErrorCode		=	eCommErr00;										//无错误
	CommData.sStart.nLength	=	4+1;												//数据段长度：地址+状态码+Data
	CommData.sStart.nBcc8	=	BCC8((u8*)&(CommData.sAddr.Addr1),CommData.sStart.nLength);
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
void Lock_On(u8 Num)			//开锁
{
	if(Num	==	1)
	{
		GPIO_SetBits(LockPort1,	LockPin1);
	}
	else if(Num	==	2)
	{
		GPIO_SetBits(LockPort2,	LockPin2);
	}
	else if(Num	==	3)
	{
		GPIO_SetBits(LockPort3,	LockPin3);
	}
	else if(Num	==	4)
	{
		GPIO_SetBits(LockPort4,	LockPin4);
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
void Lock_Off(u8 Num)			//释放锁
{
	if(Num	==	1)
	{
		GPIO_ResetBits(LockPort1,	LockPin1);
	}
	else if(Num	==	2)
	{
		GPIO_ResetBits(LockPort2,	LockPin2);
	}
	else if(Num	==	3)
	{
		GPIO_ResetBits(LockPort3,	LockPin3);
	}
	else if(Num	==	4)
	{
		GPIO_ResetBits(LockPort4,	LockPin4);
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
void ResetData(void)			//重置相关变量
{
	ICCardReadCount1	=	0;
	ICCardReadCount2	=	0;
	ICCardReadCount3	=	0;
	ICCardReadCount4	=	0;
}

#endif

