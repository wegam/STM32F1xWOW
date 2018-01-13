#ifdef PC016V20

#include "PC016V20.H"
#include "HC_PHY.H"



//==============读卡命令
const u8 CardReaderCmd_ReadData[14]	=
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
//=================设置读卡器读卡区域命令	
const u8 CardReaderCmd_SetArea[14] =
{
	0xE1,   /* F-Head 	*/			//为帧头，上位机下发时固定为0XE1，下位机应答时固定为0XD2；
	0x44,  	/* CmdType 	*/			//命令类型
	0x01,  	/* Sector 	*/			//扇区：0,1,其中0为只读，为卡信息
	0x01,  	/* Block 	*/				//块号：0,1,2	//产品使用1
	0x60,   /* KeyType	*/			//1字节密钥模式 KEYA（0x60）/KEYB(0x61)
	0xFF,   /* PassWord0 */					//6字节密码
	0xFF,   /* PassWord1 */
	0xFF,   /* PassWord2 */
	0xFF,		/* PassWord3 */
	0xFF,   /* PassWord4 */
	0xFF,		/* PassWord5 */
	0x9F,		/* Crc16-HIGN  	*/	//1字节扇区号
	0x6F,   /* Crc16-LOW 		*/	//1字节块号
	0x1E   	/* F-End*/					//为帧尾，上位机下发时固定为0X1E，下位机应答时固定为0X2D；
};

SWITCHID_CONF	SWITCHID;			//拔码开关

sBoradDef			sBorad;					//端口/槽位所有信息结构体
//==============485结体体
RS485_TypeDef	RS485_Bus;				//总线通讯485
RS485_TypeDef	RS485_Seg7;				//数码管显示485

//==============485缓冲区
//sFarmeDef	sFarme;				//总线485数据结构体---待处理
sFarmeDef	sFarmeTxd;		//总线485数据结构体---发送
sFarmeDef	sFarmeRxd;		//总线485数据结构体---接收
sFarmeDef	sFarmeRev;		//总线485数据结构体---缓存
u8 RS485BusTxd[Bus485BufferSize];

//u8 Seg485Txd[Seg485BufferSize]	=	{0};
u8 Seg485Rxd[Seg485BufferSize]	=	{0};
//u8 Seg485Rev[Seg485BufferSize]	=	{0};

u8 CardData[CardLength];		//卡有效数据缓存
//ICDataDef	ICData;					//4个端口IC数据

			
u8 ICCardReadRxd[ICCardReadBufferSize]={0};
u8 ICCardReadRev[ICCardReadBufferSize]={0};

ICBufferDef ICCardReadRxd1;
ICBufferDef ICCardReadRev1;

ICBufferDef ICCardReadRxd2;
ICBufferDef ICCardReadRev2;

ICBufferDef ICCardReadRxd3;
ICBufferDef ICCardReadRev3;

//ICBufferDef ICCardReadRxd4;
//ICBufferDef ICCardReadRev4;

u8 ICCardReadCount1	=	0;			//
u8 ICCardReadCount2	=	0;
u8 ICCardReadCount3	=	0;
u8 ICCardReadCount4	=	0;

//==============其它变量

u8 SwitchData	=	0;				//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
vu16	DSPNum	=	0;	//测试显示值
//u32	SYSTIME	=	0;	//系统计时器
//TimeDef	SYSTime;		//系统运行相关计时器
u32	Port4TimeOut;	//端口4读卡超时，
//u32	LockOnTime[4]	=	{0};	//锁吸合时间，如果时间为0则释放锁



//=================================================================================================================主程序


/*******************************************************************************
* 函数名		:	
* 功能描述	:	 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC016V20_Configuration(void)
{
	//========================系统时钟初始化
	SYS_Configuration();							//系统配置---打开系统时钟 STM32_SYS.H	
	
	//========================延时1秒，等待上电稳定
	SysTick_DeleyS(1);								//SysTick延时nS
	
	//========================硬件初始化
	Lock_Configuration();							//锁初始化
	Switch_Configuration();						//拔码开关初始化及读数
	RS485_Configuration();						//RS485配置
	CardReader_Configuration();				//读卡器配置
	Data_Initialize();								//标准参数初始化

	//========================运行指示灯初始化：频率1秒，占空比500/1000
	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);
		
	//========================设置读卡扇区、块号、KEYA/KEYB、读卡密码0x44
	//---命令：0x44
	//---扇区1
	//---块号1
	//---密码：0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	USART_DMASend	(ICCardReadPort1,(u32*)CardReaderCmd_SetArea,14);	//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44
	USART_DMASend	(ICCardReadPort2,(u32*)CardReaderCmd_SetArea,14);	//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44
	USART_DMASend	(ICCardReadPort3,(u32*)CardReaderCmd_SetArea,14);	//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44
	USART_Send		(ICCardReadPort4,(u8*)CardReaderCmd_SetArea,14);	//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44

	//========================上电读卡
//	ICCardReader_ReadAll();			//发送4个端口读卡指令
	
//	//==================上电读卡
//	ICCardReader_Read(1);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
//	ICCardReader_Read(2);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
//	ICCardReader_Read(3);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
//	ICCardReader_Read(4);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS	
	
	//========================SysTick初始化：用作周期扫描PC016V20_Server
	SysTick_Configuration(1000);	//系统嘀嗒时钟配置72MHz,单位为uS
	
	//========================独立看门狗初始化：1秒	
	IWDG_Configuration(1000);					//独立看门狗配置---参数单位ms
}
/*******************************************************************************
* 函数名		:	PC016V20_Server
* 功能描述	:	1ms扫描周期 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC016V20_Server(void)
{
	u8	Num	=	0;
	
	//========================判断是否为串口中断进入PC016V20_Server
	//---由于串口5无DMA功能，串口中断进入PC016V20_Server，接收完数据不继续往下执行
	Num	=	BoardCardReaderServer();	//RS485收发处理
	if(Num	==1)									//RS485收发处理
	{
		return;					//串口5无DMA功能，串口中断进入PC016V20_Server，接收完数据不继续往下执行
	}
	//========================计时器
	sBorad.Time.TimeSYS++;
	if(sBorad.Time.TimeSYS>=2000)
	{
		sBorad.Time.TimeSYS		=	0;		//系统计时器
		sBorad.Time.TimeSEG		=	0;		//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		sBorad.Time.TimeBOX		=	0;		//药箱检测计时器	每个端口分配100mS时间检测----可用于检测读卡器通讯状态
		sBorad.Time.TimeBUS		=	0;		//外部总线时间
		sBorad.Time.TimeCard	=	0;		//读卡器时间	
	
		sBorad.Step.ReadCard	=	0;		//bit0:	0-未执行读卡,需要读卡，		1-已读完卡
		sBorad.Step.WriteSeg	=	1;		//bit1:	0-未更新数码管,需要更新，	1-已更新(读卡过程中不更新数码管，免得占用串口）		
		return;				//同步时间
	}
	//========================独立看门狗喂狗
	IWDG_Feed();						//独立看门狗喂狗
	
	//========================串口5接收超时：表示无新数据，无数据或者数据帧接收完成
	Port4TimeOut++;
	if(Port4TimeOut>=5)
	{
		Port4TimeOut	=	0;
		ICCardReadCount4	=	0;
	}
	
	//========================串口5接收超时：表示无新数据，无数据或者数据帧接收完成

	BoardSetSeg(&sBorad);		//根据标志设置数码管状态	

	BoardCardReaderReadAll(&sBorad);			//发送4个端口读卡指令	

	BoardGetBoxProcess(&sBorad);					//取药操作及取药计时
	RS485_Server();
	MessageProcess(&sFarmeTxd,&sBorad);				//外部总线消息处理
	
	//========================流水号0x01~0xFF
	if(sBorad.Nserial>=0xFF)
	{
		sBorad.Nserial	=	0x01;
	}

//	Num		=	0;

////	eHead
//	RS485BusTxd[0]	=	0x7E;					//b0识别符	0x7E
//	RS485BusTxd[1]	=	0x00;						//b1目标地址（暂时理解为单元柜地址）下发为接收地址，上传为0x00
//	RS485BusTxd[2]	=	0x06;			//b2源地址 下发为0x00，上传为本地址
//	RS485BusTxd[3]	=	0x50;			//b3流水号 0x01~0xFF 每次使用后加1
//	RS485BusTxd[4]	=	0x25;		//b4命令号：0x92-获取指定IC卡号，0x93取药或者主动上报	
//	RS485BusTxd[5]	=	0x40;	//b5用户码：不处理，原样返回
//	RS485BusTxd[6]	=	0x06;	//b6地址+异常码固定长度为4，卡数据长度为3
//	RS485BusTxd[7]	=	0x44;						//b7柜地址(单元柜号)
//	RS485BusTxd[8]	=	0x16;						//b8层地址
//	RS485BusTxd[9]	=	0x65;	//b9槽地址（端口号）
//	
//	//================================复制数据
//	if(RS485BusTxd[6]>4)
//	{
//		for(Num=0;Num<RS485BusTxd[6]-4;Num++)
//		{
//			RS485BusTxd[Num+10]	=	0x35;		//数据
//		}
//	}
//	Num=Num+10;
//	RS485BusTxd[Num++]	=	0x50;//错误码
//	RS485BusTxd[Num++]	=	BCC8((u8*)&RS485BusTxd[1],Num);		//异或校验	//计算范围为起始段到数据段(dAdd~data[n])到错误码
//	RS485BusTxd[Num++]	=	0x7F;
//	RS485_DMASend(&RS485_Bus,(u32*)RS485BusTxd,Num);	//RS485-DMA发送程序
	
}
/*******************************************************************************
* 函数名		:	PC016V20_Server
* 功能描述	:	1ms扫描周期 
* 输入		:	
* 输出		:
* 返回 		:
*******************************************************************************/
void PC016V20_Serverbac(void)
{
	u8	Num	=	0;
	
	//========================判断是否为串口中断进入PC016V20_Server
	//---由于串口5无DMA功能，串口中断进入PC016V20_Server，接收完数据不继续往下执行
	Num	=	BoardCardReaderServer();	//RS485收发处理
	if(Num	==1)									//RS485收发处理
	{
		return;					//串口5无DMA功能，串口中断进入PC016V20_Server，接收完数据不继续往下执行
	}
	//========================计时器
	sBorad.Time.TimeSYS++;
	if(sBorad.Time.TimeSYS>=2000)
	{
		sBorad.Time.TimeSYS		=	0;		//系统计时器
		sBorad.Time.TimeSEG		=	0;		//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		sBorad.Time.TimeBOX		=	0;		//药箱检测计时器	每个端口分配100mS时间检测----可用于检测读卡器通讯状态
		sBorad.Time.TimeBUS		=	0;		//外部总线时间
		sBorad.Time.TimeCard	=	0;		//读卡器时间
		

		sBorad.Step.ReadCard	=	0;		//bit0:	0-未执行读卡,需要读卡，		1-已读完卡
		sBorad.Step.WriteSeg	=	1;		//bit1:	0-未更新数码管,需要更新，	1-已更新(读卡过程中不更新数码管，免得占用串口）
	}
	//========================独立看门狗喂狗
	IWDG_Feed();						//独立看门狗喂狗
	
	//========================串口5接收超时：表示无新数据，无数据或者数据帧接收完成
	Port4TimeOut++;
	if(Port4TimeOut>=5)
	{
		Port4TimeOut	=	0;
		ICCardReadCount4	=	0;
	}
	
	//========================串口5接收超时：表示无新数据，无数据或者数据帧接收完成
	if(sBorad.Time.TimeSYS<=500)
	{
		BoardSetSeg(&sBorad);		//根据标志设置数码管状态	
	}
	else
	{
		BoardCardReaderReadAll(&sBorad);			//发送4个端口读卡指令	
	}
	Lock_Server(&sBorad);		//锁操作，根据_LockFlag相应标志判断是否需要打开锁
	RS485_Server();	
}
//======


//=================================================================================================================协议程序
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
	
//	SYSTime.TimeBUS++;
//	if(SYSTime.TimeBUS>=1000)
//	{
//		SYSTime.TimeBUS	=	0;
//	}
	
	Num	=	RS485_ReadBufferIDLE(&RS485_Bus,(u32*)&sFarmeRev,(u32*)&sFarmeRxd);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num	==	0)
	{
		return;
	}
//	BusFrameAck(&sFarmeRev);										//外部总线应答
	
	if((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x12)&&(sFarmeRev.dAdd ==SwitchData))			//获取指定IC卡号指令,获取IC卡号为本层操作
	{
		BusFrameAck(&sFarmeRev);										//外部总线应答
		BusFrameProcessGetID(&sFarmeRev,&sBorad);		//数据命令处理
	}
	else if(
					((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x13)&&(sFarmeRev.dAdd ==SwitchData-1))													//取药指令，本层负责开锁
				||((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x93)&&(sFarmeRev.sAdd ==SwitchData-1)&&(sFarmeRev.EC==eBoxOff))		//取药指令，药已取走，释放锁
					)
	{
		BusFrameProcessLock(&sFarmeRev,&sBorad);				//锁命令处理
	}
	else if((sFarmeRev.Head==0x7E)&&(((sFarmeRev.Cmd==0x13)&&(sFarmeRev.dAdd ==SwitchData))))			//取药指令，本层处理检查药箱有无取走
	{
		BusFrameAck(&sFarmeRev);										//外部总线应答
		BusFrameProcessGetBox(&sFarmeRev,&sBorad);			//数据命令处理---取药
	}
	//=======================执行操作

	//========================获取消息数据
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
void BusFrameAck(sFarmeDef*	sFarme)			//外部总线应答
{
	u8 Temp	=	0;
	u8 Temp1	=	0;
	sBorad.Time.TimeBUS	=	0;						//清零，以免数据回复时冲突
	RS485BusTxd[0]	=	0x7E;
	RS485BusTxd[1]	=	sFarme->sAdd;
	RS485BusTxd[2]	=	sFarme->dAdd;
	RS485BusTxd[3]	=	sFarme->SN;
	RS485BusTxd[4]	=	sFarme->Cmd;
	RS485BusTxd[5]	=	sFarme->Ucode;
	RS485BusTxd[6]	=	0x00;								//错误码
//	RS485BusTxd[7]	=	BCC8((u8*)&RS485BusTxd[1],6);		//8位数异或校验码：地址段+异常码+数据段校验
	RS485BusTxd[8]	=	0x7F;
	for(Temp=1;Temp<6;Temp++)
	{
		Temp1=Temp1^RS485BusTxd[Temp];
	}
	RS485BusTxd[7]	=	Temp1;
	
	RS485_DMASend(&RS485_Bus,(u32*)RS485BusTxd,9);	//RS485-DMA发送程序
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
void MessageProcess(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//外部总线消息处理
{
	BusFrameProcessSendBox(sFarme,sBorad);			//发送药箱数据
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
void BusFrameProcessGetID(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//保存获取ID请求标志
{
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);
	//================保存标识
	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(sFarme->Addr3	==	Num+1)		//端口地址
		{
			sBorad->Nserial	=	sFarme->SN;	//保存流水号
			Port->Status.GetID	=	1;			//bit13：获取ID请求：	1-无请求，		1-有请求	，需要上报ID
			break;			//退出For
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
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
void BusFrameProcessSendBox(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//发送药箱数据
{
	u8	Flag	=	0;				//处理标志
	u8 	Num		=	0;
	PortDef*	Port;
	Port	=	&(sBorad->Port1);
	//=========================检查有无数据需要上报
	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(
					(Port->Status.GetID		==1)	//bit13：获取ID请求：	1-无请求，		1-有请求	，需要上报ID
			||	(Port->Status.SendID	==1)	//bit14：主动上报ID标志：	0-无操作，		1-检测到药箱插入	，需要上报ID
			||	(Port->Status.SendSts	==1)	//bit10：0-无操作，	1-取药状态下取出药箱后需要上报药箱状态
			)
		{
			Flag	=	1;
			break;			//退出For
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
		}
	}
	if(Flag	==0)
	{
		return;	//退出，不往下执行
	}
	//=========================上报数据
	sBorad->Time.TimeBUS	++;							//外部总线时间	
	if((sBorad->Time.TimeBUS%20)	==	0)	//20ms发送一个帧
	{
		Port	=	&(sBorad->Port1);					//重新指向端口1
		for(Num=0;Num<MaxPortNum;Num++)
		{
			if(Port->Status.GetID	==1)			//bit13：获取ID请求：	1-无请求，		1-有请求	，需要上报ID
			{
				Port->Status.GetID	=	0;			//bit13：获取ID请求：	1-无请求，		1-有请求	，需要上报ID
				sFarme->Ucode	=	Port->Ucode;	//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
				sFarme->Cmd		=	eGetID;				//上位机主动获取ID请求应答命令
				break;			//退出For
			}
			else if(Port->Status.SendSts	==1)	//bit10：0-无操作，	1-取药状态下取出药箱后需要上报药箱状态
			{	
				Port->Status.SendSts	=	0;			//bit10：0-无操作，	1-取药状态下取出药箱后需要上报药箱状态
				if(Port->Status.BoxTake	==1)		//bit11：0-无操作，1-药箱被强制取走
				{
					sFarme->Ucode	=	0x00;					//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
					sFarme->Cmd		=	eLock;				//下位机主动上传ID号命令
				}
				else
				{
					sFarme->Ucode	=	Port->Ucode;	//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
					sFarme->Cmd		=	eLock;				//下位机主动上传ID号命令					
				}
				break;			//退出For
			}
			else if(Port->Status.SendID	==1)	//bit14：主动上报ID标志：	0-无操作，		1-检测到药箱插入	，需要上报ID
			{
				Port->Status.SendID	=	0;			//bit13：获取ID请求：	1-无请求，		1-有请求	，需要上报ID
				sFarme->Ucode	=	0x00;					//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
				sFarme->Cmd		=	eSendID;			//下位机主动上传ID号命令
				break;			//退出For
			}
			if(Num<MaxPortNum-1)
			{
				Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
			}
		}
	}
	else
	{
		return;		//时间未到，退出，不处理
	}
	//=========================写入数据
	//---有卡：获取卡号命令，主动上报两种方式
	if(Port->Status.BoxFlg	==	1)		//bit0：0-无卡,1-有卡
	{
		//---有药箱需要带数据
		sFarme->data[0]=Port->CardNumber/100;			//百位
		sFarme->data[1]=Port->CardNumber%100/10;	//十位
		sFarme->data[2]=Port->CardNumber%10;			//个位
		
		sFarme->Length	=	4+3;										//地址+异常码固定长度为4，卡数据长度为3
		
		if(Port->Status.BoxBack==	1)	//bit2：0-无操作，1-药箱收回,收回后需要上报药箱号（有BoxOff标志时设置此位）
		{
			Port->Status.BoxBack	=	0;	//bit2：0-无操作，1-药箱收回,收回后需要上报药箱号（有BoxOff标志时设置此位）
			sFarme->EC	=	eBoxBack;			//药箱收回			
		}
		else if(Port->Status.GetBox==	1)	//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
		{
			//=====超时
			if(Port->Status.BoxTout==	1)	//bit10：0-无操作，1-取药超时
			{
				Port->Status.GetBox	=	0;		//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
				Port->Status.BoxTout	=	0;	//bit10：0-无操作，1-取药超时
				sFarme->EC	=	eBoxTout;			//取药箱超时	
			}
			else		//状态查询进入此程序，未超时，跳过(Port->Status.SendSts	==1)	//bit10：0-无操作，	1-取药状态下取出药箱后需要上报药箱状态
			{
				return;
			}
		}
		else
		{
			sFarme->EC	=	eNoErr;				//无错误
		}
	}
	else				//无卡：取走，通讯不良
	{
		//---无药箱不需要带数据，只发状态码
		sFarme->Length	=	4;				//地址+异常码固定长度为4，卡数据长度为3
		if(Port->Status.BoxRead	==	1)//读卡器异常
		{
			sFarme->EC	=eReadErr;			//读卡器通讯异常
		}
		else if(Port->Status.BoxTake	==1)		//bit11：0-无操作，1-药箱被强制取走
		{
			Port->Status.BoxTake	=	0;			//bit11：0-无操作，1-药箱被强制取
			sFarme->EC	=eBoxTake;					//药箱取走			
		}
		else if(Port->Status.BoxOff==	1)	//bit5：0-无操作，1-药箱被取走（有BoxDel请求时设置此位）
		{
			Port->Lock.LockOnTime=0;		//锁吸合时间，最大值0x3FFF FFFF
			sFarme->EC	=eBoxOff;				//药箱取走			
		}
		else
		{
			sFarme->EC	=eNoData;				//无药箱数据
		}
	}
	sBorad->Nserial+=1;												//流水号加1
	sFarme->SN	=	sBorad->Nserial;						//流水号
	sFarme->Addr3	=	Port->PortNum;						//槽地址（端口号）
	BusFrameProcessPacketAndSend(sFarme);			//按照协议打包数据并发送
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
void BusFrameProcessPacketAndSend(sFarmeDef*	sFarme)			//按照协议打包数据并发送
{
	u8 	Num		=	0;
	u8	Temp	=	0;
	u8	Temp1	=	0;
	RS485BusTxd[0]	=	eHead;					//b0识别符	0x7E
	RS485BusTxd[1]	=	0x00;						//b1目标地址（暂时理解为单元柜地址）下发为接收地址，上传为0x00
	RS485BusTxd[2]	=	SwitchData;			//b2源地址 下发为0x00，上传为本地址
	RS485BusTxd[3]	=	sFarme->SN;			//b3流水号 0x01~0xFF 每次使用后加1
	RS485BusTxd[4]	=	sFarme->Cmd;		//b4命令号：0x92-获取指定IC卡号，0x93取药或者主动上报	
	RS485BusTxd[5]	=	sFarme->Ucode;	//b5用户码：不处理，原样返回
	RS485BusTxd[6]	=	sFarme->Length;	//b6地址+异常码固定长度为4，卡数据长度为3
	RS485BusTxd[7]	=	0xFF;						//b7柜地址(单元柜号)
	RS485BusTxd[8]	=	0xFF;						//b8层地址
	RS485BusTxd[9]	=	sFarme->Addr3;	//b9槽地址（端口号）
	
	//================================复制数据
	if(sFarme->Length>4)
	{
		for(Num=0;Num<RS485BusTxd[6]-4;Num++)
		{
			RS485BusTxd[Num+10]	=	sFarme->data[Num];		//数据
		}
	}
	Num=Num+10;
	RS485BusTxd[Num++]	=	sFarme->EC;//错误码
//	RS485BusTxd[Num++]	=	BCC8((u8*)(&RS485BusTxd[1]),Num);		//异或校验	//计算范围为起始段到数据段(dAdd~data[n])到错误码
	for(Temp=1;Temp<Num;Temp++)
	{
		Temp1=Temp1^RS485BusTxd[Temp];
	}
	RS485BusTxd[Num++]	=	Temp1;
	RS485BusTxd[Num++]	=	0x7F;				//结束符
	RS485_DMASend(&RS485_Bus,(u32*)RS485BusTxd,Num);	//RS485-DMA发送程序
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
void BusFrameProcessGetBox(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//数据命令处理---取药
{
	u8 WriteSegFlg	=	0;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);
	//================保存标识
	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(sFarme->Addr3	==	Num+1)		//端口地址
		{
			sBorad->Nserial	=	sFarme->SN;	//保存流水号
			Port->Status.GetBox		=	1;		//bit13：0-无请求，		1-有请求	，取药(数码管数值闪烁)
			Port->Status.BoxSeg		=	1;		//bit4：0-无操作，1-需要未将卡号发给数码管
			Port->Status.SendSts	=	1;		//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态或者在无数据情况下上报状态
			WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
			//========获取锁吸合/数码管闪烁时间
			if(sFarme->Length<6)		//未配置时间
			{
				Port->Lock.LockOnTime	=	DefaultOnTime;
			}
			else
			{
				if(sFarme->data[0]>(MaxOnTime/1000))		//大于最大吸合时间
				{
					Port->Lock.LockOnTime	=	MaxOnTime;
				}
				else if(sFarme->data[0]<(DefaultOnTime/1000))
				{
					Port->Lock.LockOnTime	=	DefaultOnTime;
				}
				else
				{
					Port->Lock.LockOnTime	=	sFarme->data[0]*1000;
				}
			}
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
		}
	}
	if(WriteSegFlg)				//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	{
		sBorad->Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
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
void BusFrameProcessLock(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//锁命令处理
{
	u32	Time	=	0;
	PortDef*	Port;
	if(sFarme->Addr3	==1)
	{
		Port=&(sBorad->Port1);
	}
	else if(sFarme->Addr3	==2)
	{
		Port=&(sBorad->Port2);
	}
	else if(sFarme->Addr3	==3)
	{
		Port=&(sBorad->Port3);
	}
	else if(sFarme->Addr3	==4)
	{
		Port=&(sBorad->Port4);
	}
	//=====药已取走消息
	if((sFarme->Cmd==eLock)&&(sFarmeRev.EC==eBoxOff))
	{
		Port->Lock.On	=	0;			//0-不处理，1-开锁
		Port->Lock.LockOnTime	=	0;
		GPIO_ResetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//释放锁
	}
	else
	{
		//=====检查锁吸合时间	
		if(sFarme->Length	==4||sFarme->data[0]==0)//未配置时间或者吸合时间为0表示使用默认值
		{
			Time	=	DefaultOnTime;		//默认锁吸合时间
		}
		else if(sFarme->data[0]>	MaxOnTime/1000)
		{
			Time	=	MaxOnTime;				//锁吸合最大时间
		}
		else
		{
			Time	=	sFarme->data[0]*1000;
		}
		
		//======设置数值
		Port->Lock.On	=	1;			//需要开锁
		Port->Lock.LockOnTime	=	Time;		//吸合时间
	}
}

//=================================================================================================================取药
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
void BoardGetBoxProcess(sBoradDef*	sBorad)		//取药时的时间管理
{
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);

	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(Port->Lock.On)
		{
			if(Port->Lock.LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
			{
				Port->Lock.LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
				GPIO_SetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//开锁
			}
			else
			{
				Port->Lock.On	=0;					//0-不处理，1-开锁
				GPIO_ResetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//释放锁
			}		
		}
		else if(Port->Lock.LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
		{
			Port->Lock.LockOnTime-=1;		//锁吸合时间，最大值0x3FFF FFFF
			if(Port->Lock.LockOnTime	==	0)	//取药超时
			{
				Port->Status.BoxTout	=	1;	//bit11：0-无操作，1-取药超时
				Port->Status.SendSts	=	1;	//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态或者在无数据情况下上报状态
			}
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));				//指向下一端口
		}
	}
}


//======

//======


//=================================================================================================================读卡器程序

/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
u8 BoardCardReaderServer(void)							//读取IC卡数据
{
	u16	Num	=	0;
	sBoxStatusDef*	Status;				//箱子状态
	ICBufferDef*		CardBuffer;		//卡完整数据	
	
//	sBoxStatusDef	*TempStus;	//获取状态参数地址
	//==============================Port1
	Num	=	USART_ReadBufferIDLE(ICCardReadPort1,(u32*)&ICCardReadRev1,(u32*)&ICCardReadRxd1);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		Status	=	&(sBorad.Port1.Status);			//箱子状态
		CardBuffer	=	&ICCardReadRev1;				//卡完整数据		
		if(Num	==	22)	//有卡
		{			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
			Status->RecData	=	1;					//bit7：0-无接收到卡数据，1-有接收到卡数据		
			BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
			
		}
		else if(Num	==	6)//无卡
		{
			Status->ClrData	=	1;					//bit8：0-无操作，1-请求清除药箱在线状态
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
		}
	}
	//==============================Port2
	Num	=	USART_ReadBufferIDLE(ICCardReadPort2,(u32*)&ICCardReadRev2,(u32*)&ICCardReadRxd2);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		Status	=	&(sBorad.Port2.Status);			//箱子状态
		CardBuffer	=	&ICCardReadRev2;				//卡完整数据		
		if(Num	==	22)	//有卡
		{			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
			Status->RecData	=	1;					//bit7：0-无接收到卡数据，1-有接收到卡数据		
			BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
			
		}
		else if(Num	==	6)//无卡
		{
			Status->ClrData	=	1;					//bit8：0-无操作，1-请求清除药箱在线状态
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
		}
	}
	//==============================Port3
	Num	=	USART_ReadBufferIDLE(ICCardReadPort3,(u32*)&ICCardReadRev3,(u32*)&ICCardReadRxd3);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		Status	=	&(sBorad.Port3.Status);			//箱子状态
		CardBuffer	=	&ICCardReadRev3;				//卡完整数据		
		if(Num	==	22)	//有卡
		{			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
			Status->RecData	=	1;					//bit7：0-无接收到卡数据，1-有接收到卡数据		
			BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
			
		}
		else if(Num	==	6)//无卡
		{
			Status->ClrData	=	1;					//bit8：0-无操作，1-请求清除药箱在线状态
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
		}
	}
	//==============================Port4
	if(USART_GetITStatus(ICCardReadPort4, USART_IT_RXNE))
  {
		Port4TimeOut	=	0;		//端口4连续读数超时---清除
		USART_ClearITPendingBit(ICCardReadPort4, USART_IT_RXNE);
    ICCardReadRxd[ICCardReadCount4]	=	USART_ReceiveData(ICCardReadPort4);
		ICCardReadCount4++;
		
		if(ICCardReadCount4==6	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24	&&	ICCardReadRxd[5]==0x2D)
		{
			Status	=	&(sBorad.Port4.Status);			//箱子状态
			Status->ClrData	=	1;					//bit8：0-无操作，1-请求清除药箱在线状态
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			
			ICCardReadCount4=0;
		}
		if(ICCardReadCount4>=22)//有卡
		{
			Status	=	&(sBorad.Port4.Status);			//箱子状态
			CardBuffer	=	(ICBufferDef*)ICCardReadRxd;				//卡完整数据		
			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
			Status->RecData	=	1;																	//bit7：0-无接收到卡数据，1-有接收到卡数据		
			BoardSaveCardData(&sBorad);														//根据状态保存卡号及设置相关状态
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
void BoardCardReaderReadAll(sBoradDef*	sBorad)							//向相应端口发送读卡指令
{
	if(sBorad->Step.WriteSeg	==	0)		//数码管更新标志为未更新，需要更新
	{
		return;
	}
	if(sBorad->Step.ReadCard	==	0)		//数码管更新标志为未更新，需要更新	
	{
		sBorad->Time.TimeCard	++;							//读卡器时间
		if(sBorad->Time.TimeCard==15)					//等待15ms后开始，由于数码管串口端口与读卡器复用，需要等待
		{
			HALUsartRemapDisable();					//关闭串口复用
		}
		else if(sBorad->Time.TimeCard>=20)
		{
			USART_DMASend	(ICCardReadPort1,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
			USART_DMASend	(ICCardReadPort2,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
			USART_DMASend	(ICCardReadPort3,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
			USART_Send	(ICCardReadPort4,(u8*)CardReaderCmd_ReadData,14);			//串口5发送程序
			
			sBorad->Step.ReadCard	=	1;
			sBorad->Time.TimeCard	=	0;				//读卡器时间
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
void ICCardReader_ReadAllbac(void)							//向相应端口发送读卡指令
{
		sBorad.Time.TimeCard	++;								//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		if(sBorad.Time.TimeCard==15)			//等待15ms后开始，由于数码管串口端口与读卡器复用，需要等待
		{
			HALUsartRemapDisable();					//关闭串口复用
		}
		if(sBorad.Time.TimeCard>1000)
		{
			return;
		}
		if((sBorad.Time.TimeCard%1000)==250)
		{
			return;
		}
		if((sBorad.Time.TimeCard%1000)	==	10)
		{
	//		HALUsartRemapDisable();												//关闭串口复用
	//		USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	//		GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);				//I/O口重映射开启
		}
		else if((sBorad.Time.TimeCard%1000)	==	50)
		{
			USART_DMASend	(ICCardReadPort1,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
		}
		else if((sBorad.Time.TimeCard%1000)	==	100)
		{
			HALUsartRemapDisable();												//关闭串口复用
			
			USART_DMASend	(ICCardReadPort2,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
		}
		else if((sBorad.Time.TimeCard%1000)	==	150)
		{
			USART_DMASend	(ICCardReadPort3,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
		}
		else if((sBorad.Time.TimeCard%1000)	==	190)
		{
			USART_Send	(ICCardReadPort4,(u8*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
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
void BoardSaveCardData(sBoradDef*	sBorad)		//根据状态保存卡号及设置相关状态
{
	u8 WriteSegFlg	=	0;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	u8 Num	=	0;	
	PortDef*	Port	=	&(sBorad->Port1);		
	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(Port->Status.RecData==1)			//bit7：0-无接收到卡数据，1-有接收到卡数据
		{
			Port->Status.RecData	=	0;		//bit7：0-无接收到卡数据，1-有接收到卡数据==清除标志
			if(Port->Status.BoxFlg==0)		//bit0：0-无卡,1-有卡
			{
				Port->Status.BoxFlg		=	1;	//bit0：0-无卡,1-有卡;
				Port->Status.BoxRead	=	0;	//bit3：0-读卡器正常，1-读卡器通讯异常
				Port->Status.BoxSeg		=	1;	//bit4：0-无操作，1-需要未将卡号发给数码管
				Port->Status.RecData	=	0;	//bit7：0-无接收到卡数据，1-有接收到卡数据==清除标志
				Port->Status.ClrData	=	0;	//bit8：0-无操作，1-请求清除药箱在线状态
				Port->Status.SendID		=	1;	//bit14：主动上报ID标志：	0-无操作，		1-检测到药箱插入，需要上报ID
				
				if(Port->Status.BoxOff	==	1)//bit5：0-无操作，1-药箱被取走（有BoxDel请求时设置此位）
				{
					Port->Status.BoxBack	=	1;	//bit2：0-无操作，1-药箱收回,收回后需要上报药箱号（有BoxOff标志时设置此位）
					Port->Status.BoxOff		=	0;	//bit5：0-无操作，1-药箱被取走（有BoxDel请求时设置此位）
					Port->Status.SendSts	=	1;	//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态
					Port->Status.SendID		=	0;	//bit14：0-无操作，	1-检测到药箱插入，主动上报ID，如果是取药，取消主动上报，使用状态上报			
				}
				//=====================保存卡号
				memcpy(Port->CardData,CardData,CardLength);		//复制卡号信息
				Port->CardNumber	=	(((Port->CardData[0]>>4)&0x0F)*10000000)	
													+	(((Port->CardData[0])&0x0F)*1000000)
				
													+	(((Port->CardData[1]>>4)&0x0F)*100000)
													+	(((Port->CardData[1])&0x0F)*10000)
				
													+	(((Port->CardData[2]>>4)&0x0F)*1000)
													+	(((Port->CardData[2])&0x0F)*100)
				
													+	(((Port->CardData[3]>>4)&0x0F)*10)
													+	(((Port->CardData[3])&0x0F));		//将卡号数据合并为十进制数
				WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管												
			}
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));				//指向下一端口
		}		
	}
	if(WriteSegFlg)				//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	{
		sBorad->Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
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
void BoardClrCardData(sBoradDef*	sBorad)		//删除卡号数据及设置相关状态
{
	u8 WriteSegFlg	=	0;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);		
	for(Num=0;Num<MaxPortNum;Num++)
	{	
		if(Port->Status.ClrData	==	1)				//bit8：0-无操作，1-删除数据请求(药箱不在）需要删除数据及更新数码管
		{	
			Port->Status.ClrData	=	0;					//bit8：0-无操作，1-删除数据请求(药箱不在）需要删除数据及更新数码管
			
			if(Port->Status.BoxFlg	==	1)			//bit0：0-无卡,1-有卡
			{
				//========根据标志位设置相关状态
				Port->Status.BoxFlg		=	0;				//bit0：0-无卡,1-有卡;
				Port->Status.BoxRead	=	0;				//bit3：0-读卡器正常，1-读卡器通讯异常
				Port->Status.BoxSeg		=	1;				//bit4：0-无操作，1-需要未将卡号发给数码管
				Port->Status.RecData	=	0;				//bit7：0-无接收到卡数据，1-有接收到卡数据

				if(Port->Status.GetBox	==	1)		//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
				{	
					Port->Status.GetBox		=	0;			//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
					Port->Status.BoxOff		=	1;			//bit5：0-无操作，1-药箱被取走（有BoxDel请求时设置此位）
					Port->Status.SendSts	=	1;			//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态					
				}
				else
				{
					Port->Status.BoxTake	=	1;			//bit11：0-无操作，1-药箱被强制取
					Port->Status.SendSts	=	1;			//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态
				}
				//========清除数据
				Port->CardNumber	=	0;
				memset(Port->Seg.data,0x00,8);
				memset(Port->CardData,0x00,CardLength);		//删除卡号信息
				WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
			}
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));				//指向下一端口
		}
	}
	if(WriteSegFlg)				//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	{
		sBorad->Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	}
//		if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//		{			
//			Port	=	&sBorad->Port2;					//指向端口1
//		}
//		if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//		{			
//			Port	=	&sBorad->Port3;					//指向端口1
//		}
//		if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//		{			
//			Port	=	&sBorad->Port4;					//指向端口1
//		}
//		if(Port->Status.ClrData	==	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//		{
//			//========清除数据
//			Port->CardNumber	=	0;
//			memset(Port->Seg.data,0x00,8);
//			//========根据标志位设置相关状态
//			Port->Status.ClrData	=	0;				//bit8：0-无操作，1-请求清除药箱在线状态
//			Port->Status.BoxFlg		=	0;				//bit0：0-无卡,1-有卡;
//			Port->Status.BoxRead	=	0;				//bit3：0-读卡器正常，1-读卡器通讯异常
//			Port->Status.RecData	=	0;				//bit7：0-无接收到卡数据，1-有接收到卡数据
//			Port->Status.BoxPro		=	1;				//bit1：0-药箱无操作记录，1-药箱有操作记录（取药）:之前操作为取药，重新获取到卡表示药箱已收回
//			Port->Status.BoxSeg		=	1;				//bit4：0-无操作，1-需要未将卡号发给数码管
//			
//			if(Port->Status.BoxDel	==	1)		//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
//			{	
//				Port->Status.BoxDel	=	0;				//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
//				Port->Status.BoxSta	=	0;				//bit2：0-药箱取走，1-药箱收回,收回后需要上报药箱号
//				
//			}
//			memset(Port->CardData,0x00,CardLength);		//删除卡号信息
//			sBorad->Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新		
//		}
	
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
void BoardClrCardDatabac(sBoradDef*	sBorad)		//删除卡号数据及设置相关状态
{
//	PortDef*	Port	=	&sBorad->Port1;					//指向端口1
//	
//	if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//	{			
//		Port	=	&sBorad->Port2;					//指向端口1
//	}
//	if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//	{			
//		Port	=	&sBorad->Port3;					//指向端口1
//	}
//	if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//	{			
//		Port	=	&sBorad->Port4;					//指向端口1
//	}
//	if(Port->Status.ClrData	==	1)	//bit8：0-无操作，1-请求清除药箱在线状态
//	{
//		//========清除数据
//		Port->CardNumber	=	0;
//		memset(Port->Seg.data,0x00,8);
//		//========根据标志位设置相关状态
//		Port->Status.ClrData	=	0;				//bit8：0-无操作，1-请求清除药箱在线状态
//		Port->Status.BoxFlg		=	0;				//bit0：0-无卡,1-有卡;
//		Port->Status.BoxRead	=	0;				//bit3：0-读卡器正常，1-读卡器通讯异常
//		Port->Status.RecData	=	0;				//bit7：0-无接收到卡数据，1-有接收到卡数据
//		Port->Status.BoxPro		=	1;				//bit1：0-药箱无操作记录，1-药箱有操作记录（取药）:之前操作为取药，重新获取到卡表示药箱已收回
//		Port->Status.BoxSeg		=	1;				//bit4：0-无操作，1-需要未将卡号发给数码管
//		
//		if(Port->Status.BoxDel	==	1)		//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
//		{	
//			Port->Status.BoxDel	=	0;				//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
//			Port->Status.BoxSta	=	0;				//bit2：0-药箱取走，1-药箱收回,收回后需要上报药箱号
//			
//		}
//		memset(Port->CardData,0x00,CardLength);		//删除卡号信息
//		sBorad->Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新		
//	}
}
//======


//=================================================================================================================数码管控制程序
/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void BoardSetSeg(sBoradDef*	sBorad)			//根据标志设置数码管状态：卡有变化、取药请求、周期更新数码管状态
{
	if(sBorad->Step.WriteSeg	==	0)		//数码管更新标志为未更新，需要更新	
	{
		sBorad->Time.TimeSEG++;						//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		if(sBorad->Time.TimeSEG==15)			//等待15ms后开始，由于数码管串口端口与读卡器复用，需要等待
		{
			HALUsartRemapEnable();												//使能串口复用
		}
		if((sBorad->Time.TimeSEG%20)	==	0)	//20mS更新一个：通过485接口
		{
			u32	Time;						//点亮时间，单位mS
			u8 Num		=	0;
//			u8 SegFlg	=	0;			//0--无数据需要发送，可以把串口复用关闭，1--需要发送数据，需要等待20ms后关闭才可以根据条件关闭串口复用
			PortDef*	Port	=	&(sBorad->Port1);
			for(Num=0;Num<MaxPortNum;Num++)
			{
				if(Port->Status.BoxSeg	==	1)	//bit4：0-无操作，1-需要更新数码管
				{
					//============根据情况发送相关数码管命令
					//============插入药箱常亮30S；取药箱请求：一定时间内闪烁；无药箱：小数点一直闪烁；读卡通讯故障：小数点常亮
					if(Port->Status.BoxFlg	==	1)		//bit0：0-无卡,1-有卡
					{
						if(Port->Status.GetBox	==	1)	//bit13：0-无请求，		1-有请求	，取药(数码管数值闪烁),闪烁时间最大MaxOnTime
						{
							Port->Seg.cmd.DispEnNum	=	1;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.cmd.DispMdNum	=	1;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.cmd.DispEnDp	=	0;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.cmd.DispTime	=	1;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
							Time	=	Port->Lock.LockOnTime;		//闪烁时间根据上位机发送的需要锁的吸合时间
						}
						else			//非取药请求，则表示新插入药箱：常亮
						{
							Port->Seg.cmd.DispEnNum	=	1;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.cmd.DispMdNum	=	0;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.cmd.DispEnDp	=	0;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.cmd.DispTime	=	1;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
							Time	=	SegOnTime;								//插入药箱后数码管常亮时间30S
						}
						//=================写入数据
						//==数据,高位在前,data[0~3]为显示内容,data[4~7]为闪烁时间
						//=================写入卡呺
						Port->Seg.data[0]	=	((Port->CardNumber)>>24)&0xFF;
						Port->Seg.data[1]	=	((Port->CardNumber)>>16)&0xFF;
						Port->Seg.data[2]	=	((Port->CardNumber)>>8)&0xFF;
						Port->Seg.data[3]	=	((Port->CardNumber)>>0)&0xFF;
						//=================写入时间
						Port->Seg.data[4]	=	(Time>>24)&0xFF;
						Port->Seg.data[5]	=	(Time>>16)&0xFF;
						Port->Seg.data[6]	=	(Time>>8)&0xFF;
						Port->Seg.data[7]	=	(Time>>0)&0xFF;
					}
					else		//bit0：0-无卡,1-有卡//药箱被取走或者读卡器坏
					{
						if(Port->Status.BoxRead	==	1)			//bit3：0-读卡器正常，1-读卡器通讯异常
						{
							Port->Seg.cmd.DispEnNum	=	0;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.cmd.DispMdNum	=	0;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.cmd.DispEnDp	=	1;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.cmd.DispMdDp	=	0;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
							Port->Seg.cmd.DispTime	=	0;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
						}
						else
						{
							Port->Seg.cmd.DispEnNum	=	0;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.cmd.DispMdNum	=	0;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.cmd.DispEnDp	=	1;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.cmd.DispMdDp	=	1;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
							Port->Seg.cmd.DispTime	=	0;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
						}
					}
					HALSendSeg((u32*)&Port->Seg);		//向数码管发送数据
					Port->Status.BoxSeg	=	0;					//bit4：0-无操作，1-需要更新数码管//清除标志
					return;				//退出，发送数据，等待下一个延时时间再做剩余的数码管更新
				}
				if(Num<MaxPortNum-1)
				{
					Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
				}
			}
			
			//执行到这表示以上未检测出数码管更新标志，
			sBorad->Step.WriteSeg	=	1;			//bit1:	0-未更新数码管,需要更新，	1-已更新
			HALUsartRemapDisable();					//关闭串口复用
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
void BoardSetSegbac(sBoradDef*	sBorad)			//根据标志设置数码管状态
{

	u8 DesAddr	=	0;				//数码管地址
//	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);

	sBorad->Time.TimeSEG	++;					//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	if(sBorad->Time.TimeSEG>=1000)
	{
		return;
	}
	if((sBorad->Time.TimeSEG%1000)==250)
	{
		GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);				//I/O口重映射关闭
		USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
		return;
	}
	
	if((sBorad->Time.TimeSEG%1000)	==	5)
	{
		GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);				//I/O口重映射开启
		RS485_DMA_ConfigurationNRRemap(&RS485_Seg7,Seg485BaudRate,(u32*)Seg485Rxd,Seg485BufferSize);//USART_DMA配置(映射)--查询方式，不开中断,配置完默认为接收状态
		return;
	}
	if((sBorad->Time.TimeSEG%1000)	==	50)
	{
		DesAddr	=	1;				//数码管地址
		Port	=	&(sBorad->Port1);
	}
	else if((sBorad->Time.TimeSEG%1000)	==	100)
	{
		DesAddr	=	2;				//数码管地址
		Port	=	&(sBorad->Port2);
	}
	else if((sBorad->Time.TimeSEG%1000)	==	150)
	{
		DesAddr	=	3;				//数码管地址
		Port	=	&(sBorad->Port3);
	}
	else if((sBorad->Time.TimeSEG%1000)	==	200)
	{
		DesAddr	=	4;				//数码管地址
		Port	=	&(sBorad->Port4);
	}
		
	if((Port->Status.BoxSeg==1)&&(DesAddr!=0))		//bit4：0-无操作，1-需要未将卡号发给数码管
	{
		Port->Status.BoxSeg	=	0;				//bit4：0-无操作，1-需要未将卡号发给数码管
		Port->Seg.desAddr		=	DesAddr;	//数码管地址
		Port->Seg.len	=	8;							//长度
		//========================无卡
		if(Port->Status.BoxFlg	==	0)		//bit0：0-无卡,1-有卡
		{
			//------------设置模式:显示点
			Port->Seg.cmd.DispEnNum	=	0;	//bit0显示数值	：	0-不显示，		1-显示
			Port->Seg.cmd.DispEnDp	=	1;	//bit1显示点		：	0-不显示，		1-显示
			Port->Seg.cmd.DispTime	=	0;	//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
			//----------------------无卡&读卡异常(数字不亮，小数点常亮）
			if(Port->Status.BoxRead	==	1)	//bit3：0-读卡器正常，1-读卡器通讯异常
			{
				Port->Seg.cmd.DispMdDp	=	0;	//bit3点模式		：	0-静态显示，	1-0.5S闪烁				
			}
			//----------------------无卡&读卡正常常(数字不亮，小数点闪烁）
			else		
			{
				Port->Seg.cmd.DispMdDp	=	1;	//bit3点模式		：	0-静态显示，	1-0.5S闪烁
			}
		}
		//========================有卡
		else if(Port->Status.BoxFlg	==	1)		//bit0：0-无卡,1-有卡
		{
			u32	Time;											//点亮时间，单位mS
			Port->Seg.cmd.DispEnNum	=	1;	//bit0显示数值	：	0-不显示，		1-显示
			Port->Seg.cmd.DispEnDp	=	0;	//bit1显示点		：	0-不显示，		1-显示
			Port->Seg.cmd.DispTime	=	1;	//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
			
			//----------------------插入药箱：常亮10秒
			if(Port->Status.GetBox	==	0)	//bit6：0-无待处理操作，1-移除药箱请求(数码管数值闪烁)
			{
				Time	=	10000;		//10秒
				Port->Seg.cmd.DispMdNum	=	0;	//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
				
			}
			//----------------------取出药箱：闪烁60秒
			else if(Port->Status.GetBox	==	1)	//bit6：0-无待处理操作，1-移除药箱请求(数码管数值闪烁)
			{
				Time	=	60000;		//10秒
				Port->Seg.cmd.DispMdNum	=	1;	//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
			}
			
			//----------------------写入数值
				Port->Seg.data[0]	=	((Port->CardNumber)>>24)&0xFF;
				Port->Seg.data[1]	=	((Port->CardNumber)>>16)&0xFF;
				Port->Seg.data[2]	=	((Port->CardNumber)>>8)&0xFF;
				Port->Seg.data[3]	=	((Port->CardNumber)>>0)&0xFF;
			//----------------------写入时间
				Port->Seg.data[4]	=	(Time>>24)&0xFF;
				Port->Seg.data[5]	=	(Time>>16)&0xFF;
				Port->Seg.data[6]	=	(Time>>8)&0xFF;
				Port->Seg.data[7]	=	(Time>>0)&0xFF;					
		}
		RS485_DMASend(&RS485_Seg7,(u32*)&(Port->Seg),14);	//RS485-DMA发送程序
	}
}


//======

//=================================================================================================================锁程序
/*******************************************************************************
*函数名			:	Lock_Server
*功能描述		:	锁操作，根据_LockFlag相应标志判断是否需要打开锁
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void Lock_Server(sBoradDef*	sBorad)		//锁操作，根据_LockFlag相应标志判断是否需要打开锁
{
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);

	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(Port->Lock.On)
		{
			if(Port->Lock.LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
			{
				Port->Lock.LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
				GPIO_SetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//开锁
			}
			else
			{
				Port->Lock.On	=0;					//0-不处理，1-开锁
				GPIO_ResetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//释放锁
			}		
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));				//指向下一端口
		}
	}	
}
//======


//=================================================================================================================硬件驱动程序

/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void HALUsartRemapEnable(void)												//使能串口复用
{
	RS485_DMA_ConfigurationNRRemap(&RS485_Seg7,Seg485BaudRate,(u32*)Seg485Rxd,Seg485BufferSize);//USART_DMA配置(映射)--查询方式，不开中断,配置完默认为接收状态
	GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);			//I/O口重映射开启
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
void HALUsartRemapDisable(void)												//关闭串口复用
{
	USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);				//I/O口重映射开启
	GPIO_ResetBits(RS485_Seg7.RS485_CTL_PORT,	RS485_Seg7.RS485_CTL_Pin);			//释放总线
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
void HALSendSeg(u32* Buffer)		//向数码管发送数据
{
//	HALUsartRemapEnable();											//使能串口复用
	RS485_DMASend(&RS485_Seg7,Buffer,sizeof(sSegDef));	//RS485-DMA发送程序
}
//======


//=================================================================================================================配置程序




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
	GPIO_Configuration_OPP50(LockPort1,	LockPin1);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50(LockPort2,	LockPin2);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50(LockPort3,	LockPin3);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	GPIO_Configuration_OPP50(LockPort4,	LockPin4);			//将GPIO相应管脚配置为PP(推挽)输出模式，最大速度50MHz----V20170605
	LockOff(1);
	LockOff(2);
	LockOff(3);
	LockOff(4);	
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
void Switch_Configuration(void)			//拔码开关初始化及读数
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
	
	SWITCHID_Conf(&SWITCHID);				//
	SWITCHID_Read(&SWITCHID);				//读拔码地址
	
	SwitchData	=	(SWITCHID.nSWITCHID)&0xFF;
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
	
	RS485_DMA_ConfigurationNR	(&RS485_Bus,Bus485BaudRate,(u32*)&sFarmeRxd,Bus485BufferSize);	//USART_DMA配置--查询方式，不开中断,配置完默认为接收状态
	
	//============================数码管显示485
	RS485_Seg7.USARTx	=	Seg485PortRmap;	
	RS485_Seg7.RS485_CTL_PORT	=	Seg485CtlPort;
	RS485_Seg7.RS485_CTL_Pin		=	Seg485CtlPin;
	
	RS485_DMA_ConfigurationNRRemap(&RS485_Seg7,Seg485BaudRate,(u32*)Seg485Rxd,Seg485BufferSize);//USART_DMA配置(映射)--查询方式，不开中断,配置完默认为接收状态
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
void CardReader_Configuration(void)			//选择相应端口读卡器配置
{
	USART_DMA_ConfigurationNR	(ICCardReadPort1,ICCardReadBaudRate,(u32*)&ICCardReadRxd,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort3,ICCardReadBaudRate,(u32*)&ICCardReadRxd3,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_ConfigurationIT(ICCardReadPort4,ICCardReadBaudRate,0,0);	//USART_配置---常规中断方式//UART5不支持DMA传输
	
//	USART_DMA_ConfigurationNRRemap	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置(映射)--查询方式，不开中断
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
void Data_Initialize(void)						//参数初始化
{
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad.Port1);
	//========================各端口锁绑定
	sBorad.Port1.Lock.GPIOx	=	LockPort1;
	sBorad.Port1.Lock.GPIO_Pin_n	=	LockPin1;
	
	sBorad.Port2.Lock.GPIOx	=	LockPort2;
	sBorad.Port2.Lock.GPIO_Pin_n	=	LockPin2;
	
	sBorad.Port3.Lock.GPIOx	=	LockPort3;
	sBorad.Port3.Lock.GPIO_Pin_n	=	LockPin3;
	
	sBorad.Port4.Lock.GPIOx	=	LockPort4;
	sBorad.Port4.Lock.GPIO_Pin_n	=	LockPin4;
	
	//========================端口编号
	Port	=	&(sBorad.Port1);
	for(Num=0;Num<MaxPortNum;Num++)
	{
		Port->PortNum	=	Num+1;			//分配端口号：1~4
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));		//指向下一端口
		}
	}
	
	//========================各端口数码管地址分配
	Port	=	&(sBorad.Port1);
	for(Num=0;Num<MaxPortNum;Num++)
	{
		Port->Seg.desAddr	=	Num+1;			//分配地址：1~4
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));		//指向下一端口
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
void ResetData(void)			//重置相关变量
{
	ICCardReadCount1	=	0;
	ICCardReadCount2	=	0;
	ICCardReadCount3	=	0;
	ICCardReadCount4	=	0;
//	memset((u8*)&ICCardReadRev4,0x00,ICCardReadBufferSize);
//	memset((u8*)&ICData,0x00,MaxPortNum*ICDataNum);					//清除卡号，重新获取，窗口数*单个卡号大小
}

#endif

