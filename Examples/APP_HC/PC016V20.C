#ifdef PC016V20

#include "PC016V20.H"
#include "HC_PHY.H"

//#define PC016V20Test

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
	0x00,   /* KeyType	*/			//1字节密钥模式 KEYA（0x60）/KEYB(0x61) 00为关闭主动上传功能
	0xFF,   /* PassWord0 */			//6字节密码
	0xFF,   /* PassWord1 */
	0xFF,   /* PassWord2 */
	0xFF,		/* PassWord3 */
	0xFF,   /* PassWord4 */
	0xFF,		/* PassWord5 */
	0x9F,		/* Crc16-HIGN  	*/	//1字节扇区号
	0x6F,   /* Crc16-LOW 		*/	//1字节块号
	0x1E   	/* F-End*/					//为帧尾，上位机下发时固定为0X1E，下位机应答时固定为0X2D；
};
//=================写卡数据	
u8 CardReaderCmd_WriteData[30] =
{
	0xE1,   /* F-Head 	*/			//为帧头，上位机下发时固定为0XE1，下位机应答时固定为0XD2；
	0x25,  	/* CmdType 	*/			//命令类型
	0x01,  	/* Sector 	*/			//扇区：0,1,其中0为只读，为卡信息
	0x01,  	/* Block 	*/				//块号：0,1,2	//产品使用1
	0x60,   /* KeyType	*/			//1字节密钥模式 KEYA（0x60）/KEYB(0x61)
	0xFF,   /* PassWord0 */			//6字节密码
	0xFF,   /* PassWord1 */
	0xFF,   /* PassWord2 */
	0xFF,		/* PassWord3 */
	0xFF,   /* PassWord4 */
	0xFF,		/* PassWord5 */
	0x00,
	0x00,
	0x01,
	0x68,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
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
sMs485FrmDef	sFarmeTxd;		//总线485数据结构体---发送
sMs485FrmDef	sFarmeRxd;		//总线485数据结构体---接收
sMs485FrmDef	sFarmeRev;		//总线485数据结构体---缓存
u8 RS485BusTxd[Bus485BufferSize];

//u8 Seg485Txd[Seg485BufferSize]	=	{0};
u8 Seg485Rxd[Seg485BufferSize]	=	{0};
//u8 Seg485Rev[Seg485BufferSize]	=	{0};

u8 CardData[CardLength];		//卡有效数据缓存
//ICDataDef	ICData;					//4个端口IC数据

u8 PowerUpFlag	=	0;				//上电标志，0未上电完，1上电完
u16	PowerTime	=	0;					//上电时间	
u8 WriteCardFlag	=	0;			//写卡标志，

u8 ICCardReadRxd[CardBufferSize]={0};
u8 ICCardReadRev[CardBufferSize]={0};

ICBufferDef ICCardReadRxd1;
ICBufferDef ICCardReadRev1;

ICBufferDef ICCardReadRxd2;
ICBufferDef ICCardReadRev2;

ICBufferDef ICCardReadRxd3;
ICBufferDef ICCardReadRev3;


u16 CardReaderTimeCount1	=	0;		//读卡超时计时
u16 CardReaderTimeCount2	=	0;		//读卡超时计时
u16 CardReaderTimeCount3	=	0;		//读卡超时计时
u16 CardReaderTimeCount4	=	0;		//读卡超时计时

u8 ICCardReadCount1	=	0;			//
u8 ICCardReadCount2	=	0;
u8 ICCardReadCount3	=	0;
u8 ICCardReadCount4	=	0;

//防止取卡时读卡不稳定误认为已收回卡：取卡后0.5秒再收到卡数据当作收回
u16	CKtime	=	200;		//读卡防抖时间
u16 ICCardReadTime1	=	0;			//
u16 ICCardReadTime2	=	0;
u16 ICCardReadTime3	=	0;
u16 ICCardReadTime4	=	0;

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
	SysTick_DeleymS(50);					//SysTick延时nmS
	
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
	SysTick_DeleymS(100);					//SysTick延时nmS
	//========================上电读卡
	
	USART_DMASend	(ICCardReadPort1,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序

	USART_DMASend	(ICCardReadPort2,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序

	USART_DMASend	(ICCardReadPort3,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序

	USART_Send		(ICCardReadPort4,(u8*)CardReaderCmd_ReadData,14);			//串口5发送程序
		
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
#ifdef PC016V20Test
	IWDG_Feed();						//独立看门狗喂狗
	sBorad.sPlu.Time.TimeSYS++;
	if(sBorad.sPlu.Time.TimeSYS>=100)
	{
		sBorad.sPlu.Time.TimeSYS=0;
		USART_DMASend	(ICCardReadPort3,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
		RS485_DMASend(&RS485_Bus,(u32*)CardReaderCmd_ReadData,14);	//RS485-DMA发送程序
	}
	return;
#endif
	//========================判断是否为串口中断进入PC016V20_Server
	//---由于串口5无DMA功能，串口中断进入PC016V20_Server，接收完数据不继续往下执行
//	Num	=	BoardCardReaderServer();	//RS485收发处理
	Num	=	CardReaderPortn();	//RS485收发处理
	if(Num	==1)									//RS485收发处理
	{
		return;					//串口5无DMA功能，串口中断进入PC016V20_Server，接收完数据不继续往下执行
	}
	if(PowerTime<PowerT)
	{
		PowerTime++;
	}
	else
	{
		PowerUpFlag	=	1;				//上电标志，0未上电完，1上电完
	}
	//========================计时器
	sBorad.sPlu.Time.TimeSYS++;
	if(sBorad.sPlu.Time.TimeSYS>=2000)
	{
		sBorad.sPlu.Time.TimeSYS		=	0;		//系统计时器
		sBorad.sPlu.Time.TimeSEG		=	0;		//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		sBorad.sPlu.Time.TimeBOX		=	0;		//药箱检测计时器	每个端口分配100mS时间检测----可用于检测读卡器通讯状态
		sBorad.sPlu.Time.TimeBUS		=	0;		//外部总线时间
		sBorad.sPlu.Time.TimeCard		=	0;		//读卡器时间	
	
		sBorad.sPlu.Step.ReadCard	=	0;		//bit0:	0-未执行读卡,需要读卡，		1-已读完卡
		sBorad.sPlu.Step.WriteSeg	=	1;		//bit1:	0-未更新数码管,需要更新，	1-已更新(读卡过程中不更新数码管，免得占用串口）		
		return;				//同步时间
	}
	//========================独立看门狗喂狗
	IWDG_Feed();						//独立看门狗喂狗
	
	CardReadTimeCount();			//取卡后倒计时，如果倒计时未完成再收回卡数据，读卡不稳定时的误动作
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

	CardReaderError();			//重置相关变量	

	BoardGetBoxProcess(&sBorad);					//取药操作及取药计时
	RS485_Server();
	MessageProcess(&sFarmeTxd,&sBorad);				//外部总线消息处理
	
	//========================流水号0x01~0xFF
	if(sBorad.Nserial>=0xFF)
	{
		sBorad.Nserial	=	0x01;
	}
}

//======


//=================================================================================================================RS485外部总线处理
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	消息处理:外部RS485总线数据接收与发送管理 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
//void MessageProcess(void)								//消息处理:外部RS485总线数据接收与发送管理
//{
//	MessageProcessRecevie();				//消息接收处理
//	MessageProcessSend();						//消息发送处理
//}
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
void MessageProcessRecevie(void)				//消息接收处理
{
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
void MessageProcessSend(void)						//消息发送处理
{
}

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
//	BusFrameAck(&sFarmeRev);					//外部总线应答
	MessageAnalysis(&sFarmeRev);				//解析消息
	if((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x12)&&(sFarmeRev.dAdd ==SwitchData))			//获取指定IC卡号指令,获取IC卡号为本层操作
	{
		sBorad.Nserial	=	sFarmeRev.nSerial;		//保存流水号
		BusFrameAck(&sFarmeRev);										//外部总线应答
		BusFrameProcessGetID(&sFarmeRev,&sBorad);		//数据命令处理
	}
	else if(
					((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x13)&&(sFarmeRev.dAdd ==SwitchData-1))													//取药指令，本层负责开锁
//				||((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x93)&&(sFarmeRev.sAdd ==SwitchData-1)&&((sFarmeRev.EC==eBoxOff)||(sFarmeRev.EC==eBoxTout)||(sFarmeRev.EC==eNoData)))		//取药指令，药已取走，释放锁
				||((sFarmeRev.Head==0x7E)&&(sFarmeRev.Cmd==0x93)&&(sFarmeRev.sAdd ==SwitchData-1)&&((sFarmeRev.StsCode==eBoxOff)))		//取药指令，药已取走，释放锁
					)
	{
		BusFrameProcessLock(&sFarmeRev,&sBorad);				//锁命令处理
	}
	else if((sFarmeRev.Head==0x7E)&&(((sFarmeRev.Cmd==0x13)&&(sFarmeRev.dAdd ==SwitchData))))			//取药指令，本层处理检查药箱有无取走
	{
		sBorad.Nserial	=	sFarmeRev.nSerial;				//保存流水号
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
void BusFrameAck(sMs485FrmDef*	sFarme)			//外部总线应答
{
	u8 Temp	=	0;
	u8 Temp1	=	0;
	sBorad.sPlu.Time.TimeBUS	=	0;						//清零，以免数据回复时冲突
	RS485BusTxd[0]	=	0x7E;
	RS485BusTxd[1]	=	sFarme->sAdd;
	RS485BusTxd[2]	=	sFarme->dAdd;
	RS485BusTxd[3]	=	sFarme->nSerial;
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
void MessageProcess(sMs485FrmDef*	sFarme,sBoradDef*	sBorad)			//外部总线消息处理
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
void MessageAnalysis(sMs485FrmDef*	sFarme)				//解析消息
{
	if(sFarme->Length==FixedAddrLenth)		//数据长度为FixedAddrLenth表示无数据//固定数据长度nLength中地址和状态码的长度
	{
		sFarme->Bcc8	=	sFarme->data[0];
		sFarme->End		=	sFarme->data[1];
	}
	else
	{
		sFarme->Bcc8	=	sFarme->data[sFarme->Length-FixedAddrLenth];
		sFarme->End		=	sFarme->data[sFarme->Length-FixedAddrLenth+1];
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
void MessagePacket(sMs485FrmDef*	sFarme)			//打包消息
{
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
void BusFrameProcessGetID(sMs485FrmDef*	sFarme,sBoradDef*	sBorad)			//保存获取ID请求标志
{
	u8 WriteSegFlg	=	0;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);
	//================保存标识
	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(sFarme->Addr.Addr3	==	Num+1)			//端口地址
		{
//			sBorad->Nserial	=	sFarme->SN;		//保存流水号
			//========数码管：每次获取ID指令就点亮数码管 30秒
			Port->Seg.sSegSts.SegFlg	=	1;	//bit0：0-无操作,	1-有操作请求(需要更新数码管状态）
//			sBorad->Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
			WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
			
			Port->sBus.BusqSts.BusFlg	=	1;	//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
			Port->sBus.BusqSts.GetID	=	1;	//bit1：0-无请求，	1-有请求	，获取ID请求需要上报ID
			break;			//退出For
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
		}
	}
	if(WriteSegFlg)				//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	{
		sBorad->sPlu.Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->sPlu.Time.TimeSEG		=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
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
void BusFrameProcessSendBox(sMs485FrmDef*	sFarme,sBoradDef*	sBorad)			//发送药箱数据
{
	u8	Flag	=	0;				//处理标志
	u8 	Num		=	0;
	u8 	Temp	=	0;
	u8 	Temp1	=	0;
	PortDef*	Port;
	Port	=	&(sBorad->Port1);
	//=========================检查有无数据需要上报
//	for(Num=0;Num<MaxPortNum;Num++)
//	{
//		if(Port->sBus.BusqSts.BusFlg	==	1)	//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
//		{
////			Port->sBus.BusqSts.BusFlg	=	0;			//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
//			Flag	=	1;
//			break;			//退出For
//		}
//		if(Num<MaxPortNum-1)
//		{
//			Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
//		}
//	}
//	if(Flag	==0)
//	{
//		return;	//退出，不往下执行
//	}
	//=========================上报数据
	sBorad->sPlu.Time.TimeBUS	++;							//外部总线时间	
	if(((sBorad->sPlu.Time.TimeBUS)%30)	==	0)	//20ms发送一个帧
	{
		Port	=	&(sBorad->Port1);					//重新指向端口1
		for(Num=0;Num<MaxPortNum;Num++)
		{
			if(Port->sBus.BusqSts.BusFlg	==	1)	//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
			{
				Flag	=	1;
				Port->sBus.BusqSts.BusFlg	=	0;			//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
				if(Port->sBus.BusqSts.GetID	==1)	//bit1：0-无请求，	1-有请求	，获取ID请求需要上报ID
				{
					Port->sBus.BusqSts.GetID	=	0;	//bit1：0-无请求，	1-有请求	，获取ID请求需要上报ID
					Port->sBus.sBusFarme.Ucode	=	Port->sBus.Ucode;			//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
					Port->sBus.sBusFarme.Cmd		=	eGetID;					//上位机主动获取ID请求应答命令
					if(Port->sBox.BoxSts.ReadErr	==1)		//bit4：0-读卡器正常，	1-读卡器通讯异常
					{
						Port->sBus.sBusFarme.StsCode	=	eReadErr;						//读卡器通讯异常
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth;
					}
					else if(Port->sBox.BoxSts.BoxOn	==0)	//bit1：0-无药箱,		1-有药箱
					{
						Port->sBus.sBusFarme.StsCode	=	eNoData;						//无药箱数据
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth;
					}
					else
					{
						Port->sBus.sBusFarme.StsCode	=	eNoErr;							//无错误
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth+FixedDataLenth;
					}
					break;			//退出For
				}
				//=======取药：无药箱数据时应答无数据及清除相关标志位
				else if(Port->sBus.BusqSts.GetBox	==1)	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
				{
					//读卡器坏
					if(Port->sBox.BoxSts.ReadErr	==	1)	//bit4：0-读卡器正常，	1-读卡器通讯异常
					{
						Port->sBus.BusqSts.GetBox		=	0;		//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
						Port->sBus.BusqSts.GotBox		=	0;		//bit3：0-无操作，	1-药箱已被取走
						Port->sBus.BusqSts.TakeBox	=	0;		//bit4：0-无操作，	1-药箱被强制取
						Port->sBus.BusqSts.BoxBack	=	0;		//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
						Port->sBus.BusqSts.TimeOut	=	0;		//bit6：0-无操作，	1-取药超时
						Port->sBus.BusqSts.SendID		=	0;		//bit7：0-无操作，	1-检测到药箱插入，主动上报ID，如果是取药，取消主动上报，使用状态上报

						
						Port->sBus.sBusFarme.Ucode	=	Port->sBus.Ucode;	//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
						Port->sBus.sBusFarme.Cmd		=	eGetBox;			//取药命令上报
						Port->sBus.sBusFarme.StsCode			=	eReadErr;				//药箱取走
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth;
						
						break;			//退出For
					}
					//读卡器正常：无药箱
					else if(Port->sBox.BoxSts.BoxOn	==	0)		//bit2：0-读卡器正常，1-读卡器通讯异常
					{
						Port->sBus.BusqSts.GetBox		=	0;		//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
						Port->sBus.BusqSts.GotBox		=	0;		//bit3：0-无操作，	1-药箱已被取走
						Port->sBus.BusqSts.TakeBox	=	0;		//bit4：0-无操作，	1-药箱被强制取
						Port->sBus.BusqSts.BoxBack	=	0;		//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
						Port->sBus.BusqSts.TimeOut	=	0;		//bit6：0-无操作，	1-取药超时
						Port->sBus.BusqSts.SendID		=	0;		//bit7：0-无操作，	1-检测到药箱插入，主动上报ID，如果是取药，取消主动上报，使用状态上报
						
						Port->sBus.sBusFarme.Ucode	=	Port->sBus.Ucode;	//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
						Port->sBus.sBusFarme.Cmd		=	eGetBox;			//取药命令上报
						Port->sBus.sBusFarme.StsCode			=	eNoData;				//药箱取走
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth;
						
						break;			//退出For
					}
					else
					{
						Flag	=	0;
					}
					break;			//退出For
				}
				//=======以下三条注意先后顺序
				else if(Port->sBus.BusqSts.BoxBack	==1)	//bit5：0-无操作，	1-药箱收回
				{
					Port->sBus.BusqSts.BoxBack	=	0;		//bit5：0-无操作，	1-药箱收回
					//根据以上两条标志确定上报类型
					if(Port->sBus.BusqSts.GotBox	==1)	//bit3：0-无操作，	1-药箱已被取走（上传卡号）用户码为0x00
					{
						Port->sBus.BusqSts.GetBox		=	0;		//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
						Port->sBus.BusqSts.TimeOut	=	0;		//bit6：0-无操作，	1-取药超时
						
						Port->sBus.BusqSts.GotBox	=	0;		//bit3：0-无操作，	1-药箱已被取走
						Port->sBus.sBusFarme.Ucode	=	0x00;			//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
						Port->sBus.sBusFarme.Cmd		=	eLock;						//下位机主动上传ID号命令
						Port->sBus.sBusFarme.StsCode			=	eBoxBack;					//药箱收回
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth+FixedDataLenth;					
					}
					else if(Port->sBus.BusqSts.TakeBox	==1)	//bit4：0-无操作，	1-药箱被强制取
					{
						Port->sBus.BusqSts.TakeBox	=	0;				//bit4：0-无操作，	1-药箱被强制取
						Port->sBus.sBusFarme.Ucode	=	0x00;							//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
						Port->sBus.sBusFarme.Cmd		=	eSendID;					//下位机主动上传ID号命令
						Port->sBus.sBusFarme.StsCode			=	eBoxBack;					//药箱收回
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth+FixedDataLenth;
					}
					else
					{
						Port->sBus.BusqSts.TakeBox	=	0;		//bit4：0-无操作，	1-药箱被强制取
						Port->sBus.sBusFarme.Ucode	=	0x00;								//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
						Port->sBus.sBusFarme.Cmd		=	eSendID;					//下位机主动上传ID号命令
						Port->sBus.sBusFarme.StsCode			=	eBoxBack;							//药箱收回
						Port->sBus.sBusFarme.Length	=	FixedAddrLenth+FixedDataLenth;
					}				
					break;			//退出For
				}
				else if(Port->sBus.BusqSts.GotBox	==1)	//bit3：0-无操作，	1-药箱已被取走
				{
					//此标志不能清除，需要等待药箱收回
	//				Port->sBus.BusqSts.GotBox	=	0;				//bit3：0-无操作，	1-药箱已被取走
					Port->sBus.sBusFarme.Ucode	=	Port->sBus.Ucode;			//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
					Port->sBus.sBusFarme.Cmd		=	eLock;				//下位机主动上传ID号命令
					Port->sBus.sBusFarme.StsCode		=	eBoxOff;			//药箱取走
					Port->sBus.sBusFarme.Length	=	FixedAddrLenth;
					break;			//退出For
				}
				else if(Port->sBus.BusqSts.TakeBox	==1)	//bit4：0-无操作，	1-药箱被强制取
				{
					//此标志不能清除，需要等待药箱收回
	//				Port->sBus.BusqSts.TakeBox	=	0;				//bit4：0-无操作，	1-药箱被强制取
					Port->sBus.sBusFarme.Ucode	=	0x00;			//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
					Port->sBus.sBusFarme.Cmd		=	eLock;			//下位机主动上传ID号命令
					Port->sBus.sBusFarme.StsCode		=	eBoxTake;			//药箱被强制取走
					Port->sBus.sBusFarme.Length	=	FixedAddrLenth;
					break;			//退出For
				}
				
				else if(Port->sBus.BusqSts.TimeOut	==1)	//bit6：0-无操作，	1-取药超时(只上传状态，不上传卡号）
				{	
					Port->sBus.BusqSts.TimeOut	=	0;				//bit10：0-无操作，	1-取药状态下取出药箱后需要上报药箱状态
					Port->sBus.sBusFarme.Ucode	=	Port->sBus.Ucode;	//数据帧到达底层后，回复的数据帧总是将其按原值返回。若数据帧是底层设备自主发出，则用户码设为0，因此规定工控主机不能将下行数据帧的用户码设为0。
					Port->sBus.sBusFarme.Cmd		=	eLock;				//下位机主动上传ID号命令
					Port->sBus.sBusFarme.StsCode			=	eBoxTout;			//药箱收回
//					Port->sBus.sBusFarme.Length	=	FixedAddrLenth+FixedDataLenth;		//带卡号上传
					Port->sBus.sBusFarme.Length	=	FixedAddrLenth;		//不带卡号上传
					break;			//退出For
				}
				else
				{
					Flag	=	0;
				}
//				break;			//退出For
			}
			if(Num<MaxPortNum-1)
			{
				Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
			}
//			else if(Num==MaxPortNum-1)
//			{
//				Port->sBus.BusqSts.BusFlg	=	0;			//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
//			}
		}
	}
	if(Flag	==0)
	{
		return;	//退出，不往下执行
	}
//	else
//	{
//		return;		//时间未到，退出，不处理
//	}
	//=========================写入数据

	sBorad->Nserial+=1;												//流水号加1
//	Port->sBus.sBusFarme.nSerial	=	sBorad->Nserial;
	Port->sBus.sBusFarme.nSerial	=	0x00;					//流水号:不能用7F 7E帧头帧尾会冲突
	Port->sBus.sBusFarme.Head		=	ePro485Head;		//头标识
	Port->sBus.sBusFarme.dAdd		=	0x00;
	Port->sBus.sBusFarme.sAdd		=	SwitchData;
	Port->sBus.sBusFarme.Addr.Addr2	=	SwitchData;			//层地址
	Port->sBus.sBusFarme.Addr.Addr3	=	Port->PortNum;						//槽地址（端口号）
	//=======卡号
	Port->sBus.sBusFarme.data[0]	=	Port->sBox.CardData[2]&0x0F;
	Port->sBus.sBusFarme.data[1]	=	Port->sBox.CardData[3]>>4&0x0F;
	Port->sBus.sBusFarme.data[2]	=	Port->sBox.CardData[3]&0x0F;
	
	memcpy(RS485BusTxd,(u8*)&Port->sBus.sBusFarme,14);
//	memcpy(Port->sBus.sBusFarme.data,&(Port->sBox.CardData[1]),3);
	//BCC8
//		u8 	Num		=	0;
//	u8 	Temp		=	0;
	for(Temp=1;Temp<7+Port->sBus.sBusFarme.Length;Temp++)
	{
		Temp1=Temp1^RS485BusTxd[Temp];
	}
	Port->sBus.sBusFarme.Bcc8	=	Temp1;
	
//	Port->sBus.sBusFarme.Bcc8		=	BCC8((u8*)&(Port->sBus.sBusFarme.dAdd),6+Port->sBus.sBusFarme.Length);		//异或校验	//计算范围为起始段到数据段(dAdd~data[n])到错误码

	Port->sBus.sBusFarme.End		=	ePro485End;			//结束标识
//	if(Port->sBus.sBusFarme.Length>4)
//	{
//		Port->sBus.sBusFarme.data[3]	=	Port->sBus.sBusFarme.Bcc8;
//		Port->sBus.sBusFarme.data[4]	=	Port->sBus.sBusFarme.End;
//	}
//	else
//	{
//		Port->sBus.sBusFarme.data[0]	=	Port->sBus.sBusFarme.Bcc8;
//		Port->sBus.sBusFarme.data[1]	=	Port->sBus.sBusFarme.End;
//	}
	if(Port->sBus.sBusFarme.Length>4)
	{
		RS485BusTxd[14]	=	Port->sBus.sBusFarme.Bcc8;
		RS485BusTxd[15]	=	Port->sBus.sBusFarme.End;
	}
	else
	{
		RS485BusTxd[11]	=	Port->sBus.sBusFarme.Bcc8;
		RS485BusTxd[12]	=	Port->sBus.sBusFarme.End;
	}
	RS485_DMASend(&RS485_Bus,(u32*)RS485BusTxd,6+Port->sBus.sBusFarme.Length+3);	//RS485-DMA发送程序
//	BusFrameProcessPacketAndSend(sFarme);			//按照协议打包数据并发送
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
void BusFrameProcessPacketAndSend(sMs485FrmDef*	sFarme)			//按照协议打包数据并发送
{
	u8 	Num		=	0;
	u8	Temp	=	0;
	u8	Temp1	=	0;
	RS485BusTxd[0]	=	ePro485Head;				//b0识别符	0x7E
	RS485BusTxd[1]	=	0x00;						//b1目标地址（暂时理解为单元柜地址）下发为接收地址，上传为0x00
	RS485BusTxd[2]	=	SwitchData;			//b2源地址 下发为0x00，上传为本地址
	RS485BusTxd[3]	=	sFarme->nSerial;			//b3流水号 0x01~0xFF 每次使用后加1
	RS485BusTxd[4]	=	sFarme->Cmd;		//b4命令号：0x92-获取指定IC卡号，0x93取药或者主动上报	
	RS485BusTxd[5]	=	sFarme->Ucode;	//b5用户码：不处理，原样返回
	RS485BusTxd[6]	=	sFarme->Length;	//b6地址+异常码固定长度为4，卡数据长度为3
	RS485BusTxd[7]	=	0xFF;						//b7柜地址(单元柜号)
	RS485BusTxd[8]	=	SwitchData;						//b8层地址
	RS485BusTxd[9]	=	sFarme->Addr.Addr3;	//b9槽地址（端口号）
	RS485BusTxd[10]	=	sFarme->StsCode;//错误码
	
	//================================复制数据
	if(sFarme->Length>4)
	{
		for(Num=0;Num<RS485BusTxd[6]-4;Num++)
		{
			RS485BusTxd[Num+11]	=	sFarme->data[Num];		//数据
		}
	}
	Num=Num+11;
//	RS485BusTxd[Num++]	=	sFarme->EC;//错误码
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
void BusFrameProcessGetBox(sMs485FrmDef*	sFarme,sBoradDef*	sBorad)			//数据命令处理---取药
{
	u8 WriteSegFlg	=	0;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);
	//================保存标识
	for(Num=0;Num<MaxPortNum;Num++)
	{
		if(sFarme->Addr.Addr3	==	Num+1)					//端口地址
		{
//			sBorad->Nserial	=	sFarme->SN;				//保存流水号
			Port->sBus.Ucode	=	sFarme->Ucode;		//保存用户码
			Port->sBus.BusqSts.BusFlg		=	1;			//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
			Port->sBus.BusqSts.GetBox		=	1;			//bit2：0-无请求，		1-有请求	，取药(数码管数值同时闪烁)
			Port->Seg.sSegSts.SegFlg		=	1;			//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
//			Port->Status.SendSts	=	1;		//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态或者在无数据情况下上报状态
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
		sBorad->sPlu.Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->sPlu.Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
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
void BusFrameProcessLock(sMs485FrmDef*	sFarme,sBoradDef*	sBorad)			//锁命令处理
{
	u32	Time	=	0;
	PortDef*	Port;
	if(sFarme->Addr.Addr3	==1)
	{
		Port=&(sBorad->Port1);
	}
	else if(sFarme->Addr.Addr3	==2)
	{
		Port=&(sBorad->Port2);
	}
	else if(sFarme->Addr.Addr3	==3)
	{
		Port=&(sBorad->Port3);
	}
	else if(sFarme->Addr.Addr3	==4)
	{
		Port=&(sBorad->Port4);
	}
	//=========================药已取走消息
//	if((sFarme->Cmd==eLock)&&((sFarmeRev.EC==eBoxOff)||(sFarmeRev.EC==eBoxTout)||(sFarmeRev.EC==eNoData)))
	if((sFarme->Cmd==eLock)&&((sFarmeRev.StsCode==eBoxOff)))
	{
//		Port->Lock.Status=0;										//0-锁未开，1-已开eNoData
		if(Port->Lock.sLockSts.LockOn)
		{
			Port->Lock.LockOnTime	=	LockOffTime;	
		}
//		Port->Lock.sLockSts.LockOn	=	1;
//		Port->Lock.LockOnTime	=	LockOffTime;		
//		GPIO_ResetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//释放锁
	}
	else if(sFarme->Cmd==0x13)
	{
		//=====检查锁吸合时间	
		if(sFarme->Length	==4||sFarme->data[0]==0)//未配置时间或者吸合时间为0表示使用默认值
		{
			Time	=	DefaultOnTime-500;		//默认锁吸合时间
		}
		else if(sFarme->data[0]>	MaxOnTime/1000)
		{
			Time	=	MaxOnTime-500;				//锁吸合最大时间
		}
		else
		{
			Time	=	sFarme->data[0]*1000-500;
		}
		
		//======设置数值
		Port->Lock.sLockSts.LockOn	=	1;	//bit1：0-释放锁，1-开锁
		Port->Lock.LockOnTime	=	Time;			//吸合时间
	}
}
//======

//=================================================================================================================药箱
/*******************************************************************************
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
void BoardBoxProcess(sBoradDef*	sBorad)				//药箱处理程序
{
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
void BoardGetBoxProcess(sBoradDef*	sBorad)		//取药时的时间管理,包括超时
{
	u8 Num	=	0;
	PortDef*	Port	=	&(sBorad->Port1);

	for(Num=0;Num<MaxPortNum;Num++)
	{
		//===============================================================取药
		if(Port->Lock.LockOnTime>0)			//锁吸合时间，最大值0x3FFF FFFF
		{
			Port->Lock.LockOnTime-=1;			//锁吸合时间，最大值0x3FFF FFFF---倒计时
			if(Port->Lock.LockOnTime==0)	//倒计时完成
			{
				if(Port->Lock.sLockSts.LockOn)						//锁控制层
				{
					Port->Lock.sLockSts.LockFlg		=	0;	//bit0：0-无操作, 1-有操作请求(控制锁的状态）
					Port->Lock.sLockSts.LockOn		=	0;	//bit1：0-释放锁，1-开锁
					Port->Lock.sLockSts.LockSts		=	0;	//bit2：0-已释放，1-已开锁
					Port->Lock.sLockSts.LockTout	=	0;	//bit3：0-未超时，1-已超时(取药超时)
					GPIO_ResetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//释放锁
				}
				else												//药箱层
				{
					Port->Lock.sLockSts.LockFlg		=	0;	//bit0：0-无操作, 1-有操作请求(控制锁的状态）
					Port->Lock.sLockSts.LockSts		=	0;	//bit2：0-已释放，1-已开锁
					Port->Lock.sLockSts.LockTout	=	1;	//bit3：0-未超时，1-已超时(取药超时)
					
					//===========关闭数码管
					Port->Seg.sSegSts.SegFlg			=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
					Port->Seg.sSegSts.SegTimeOut	=	1;	//bit1：0-无操作,		1-取药超时，需要关闭数码管
					//===========总线
					Port->sBus.BusqSts.BusFlg			=	1;	//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
					Port->sBus.BusqSts.GetBox			=	0;	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
					Port->sBus.BusqSts.GotBox			=	0;	//bit3：0-无操作，	1-药箱已被取走
					Port->sBus.BusqSts.TakeBox		=	0;	//bit4：0-无操作，	1-药箱被强制取
					Port->sBus.BusqSts.BoxBack		=	0;	//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
					Port->sBus.BusqSts.TimeOut		=	1;	//bit6：0-无操作，	1-取药超时
					Port->sBus.BusqSts.SendID			=	0;	//bit7：0-无操作，	1-检测到药箱插入，主动上报ID，如果是取药，取消主动上报，使用状态上报
//					Port->Status.SendSts	=	1;	//bit10：0-无操作，1-取药状态下药箱变化后需要上报药箱状态或者在无数据情况下上报状态
				}
			}
			else	if((Port->Lock.sLockSts.LockOn==1)&&(Port->Lock.sLockSts.LockSts==0))					//需要开锁
			{
				Port->Lock.sLockSts.LockSts	=	1;			//0-锁未开，1-已开
				GPIO_SetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//开锁
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
	sBoxDef*	Status;				//箱子状态
	ICBufferDef*		CardBuffer;		//卡完整数据	
	
//	sBoxStatusDef	*TempStus;	//获取状态参数地址
	//==============================Port1
	Num	=	USART_ReadBufferIDLE(ICCardReadPort1,(u32*)&ICCardReadRev1,(u32*)&ICCardReadRxd1);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		u8 temp	=	BoardCardDataAnalysis(&ICCardReadRev1);					//读取IC卡数据校验，返回0失败，作废，返回1正确
		if(temp	!=0)
		{
			Status	=	&(sBorad.Port1.sBox);			//箱子状态
			CardBuffer	=	&ICCardReadRev1;				//卡完整数据		
			if(Num	==	22)	//有卡
			{	
				CardReaderTimeCount1	=	0;
				memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				
			}
			else if(Num	==	6)//无卡
			{
				CardReaderTimeCount1	=	0;
				ICCardReadTime1	=500;					//防止取卡时读卡不稳定误认为已收回卡
				Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
				BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			}
		}		
	}
	//==============================Port2
	Num	=	USART_ReadBufferIDLE(ICCardReadPort2,(u32*)&ICCardReadRev2,(u32*)&ICCardReadRxd2);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		u8 temp	=	BoardCardDataAnalysis(&ICCardReadRev2);					//读取IC卡数据校验，返回0失败，作废，返回1正确
		if(temp	!=0)
		{
			Status	=	&(sBorad.Port2.sBox);			//箱子状态
			CardBuffer	=	&ICCardReadRev2;				//卡完整数据		
			if(Num	==	22)	//有卡
			{	
				CardReaderTimeCount2	=	0;				
				memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				
			}
			else if(Num	==	6)//无卡
			{
				CardReaderTimeCount2	=	0;
				Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
				BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			}
		}
	}
	//==============================Port3
	Num	=	USART_ReadBufferIDLE(ICCardReadPort3,(u32*)&ICCardReadRev3,(u32*)&ICCardReadRxd3);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num)
	{
		u8 temp	=	BoardCardDataAnalysis(&ICCardReadRev3);					//读取IC卡数据校验，返回0失败，作废，返回1正确
		if(temp	!=0)
		{
			Status	=	&(sBorad.Port3.sBox);			//箱子状态
			CardBuffer	=	&ICCardReadRev3;				//卡完整数据		
			if(Num	==	22)	//有卡
			{	
				CardReaderTimeCount3	=	0;
				memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				
			}
			else if(Num	==	6)//无卡
			{
				CardReaderTimeCount3	=	0;
				Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
				BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			}
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
			CardReaderTimeCount4	=	0;
			Status	=	&(sBorad.Port4.sBox);			//箱子状态
			Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			
			ICCardReadCount4=0;
		}
		if(ICCardReadCount4>=22	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24)//有卡
		{
			CardReaderTimeCount4	=	0;
			Status	=	&(sBorad.Port4.sBox);			//箱子状态
			CardBuffer	=	(ICBufferDef*)ICCardReadRxd;				//卡完整数据		
			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
			Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
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
* 函数名			:	function
* 功能描述		:	函数功能说明 
* 输入			: void
* 返回值			: void
* 修改时间		: 无
* 修改内容		: 无
* 其它			: 
*******************************************************************************/
u8 CardReaderPortn(void)	//DMA接收读卡器数据
{
	u16	Num	=	0;
	sBoxDef*	Status;				//箱子状态
	ICBufferDef*		CardBuffer;		//卡完整数据	
	
//	sBoxStatusDef	*TempStus;	//获取状态参数地址
	//==============================Port1
	Num	=	USART_ReadBufferIDLE(ICCardReadPort1,(u32*)&ICCardReadRev1,(u32*)&ICCardReadRxd1);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num&&(ICCardReadTime1==0))
	{
		u8 temp	=	BoardCardDataAnalysis(&ICCardReadRev1);					//读取IC卡数据校验，返回0失败，作废，返回1正确
		if(temp	!=0)
		{
			Status	=	&(sBorad.Port1.sBox);			//箱子状态
			CardBuffer	=	&ICCardReadRev1;			//卡完整数据		
			if(Num	==	22)	//有卡
			{	
				CardReaderTimeCount1	=	0;
				ICCardReadTime1	=	CKtime;				//取卡后500ms读到卡的数据当作无效
				memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
				if(!((CardData[0]==0x00)&&(CardData[1]==0x00)&&(CardData[2]==0x00)&&(CardData[3]==0x00)))
				{
					Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
					BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				}
				
//				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
//				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				
			}
			else if(Num	==	6)//无卡
			{
				CardReaderTimeCount1	=	0;
				ICCardReadTime1	=	CKtime;				//取卡后500ms读到卡的数据当作无效
				Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
				BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			}
		}		
	}
	//==============================Port2
	Num	=	USART_ReadBufferIDLE(ICCardReadPort2,(u32*)&ICCardReadRev2,(u32*)&ICCardReadRxd2);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num&&(ICCardReadTime2==0))
	{
		u8 temp	=	BoardCardDataAnalysis(&ICCardReadRev2);					//读取IC卡数据校验，返回0失败，作废，返回1正确
		if(temp	!=0)
		{
			Status	=	&(sBorad.Port2.sBox);			//箱子状态
			CardBuffer	=	&ICCardReadRev2;				//卡完整数据		
			if(Num	==	22)	//有卡
			{	
				CardReaderTimeCount2	=	0;
				ICCardReadTime2	=	CKtime;				//取卡后500ms读到卡的数据当作无效				
				memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
//				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
//				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				if(!((CardData[0]==0x00)&&(CardData[1]==0x00)&&(CardData[2]==0x00)&&(CardData[3]==0x00)))
				{
					Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
					BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				}
				
			}
			else if(Num	==	6)//无卡
			{
				CardReaderTimeCount2	=	0;
				ICCardReadTime2	=	CKtime;				//取卡后500ms读到卡的数据当作无效
				Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
				BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			}
		}
	}
	//==============================Port3
	Num	=	USART_ReadBufferIDLE(ICCardReadPort3,(u32*)&ICCardReadRev3,(u32*)&ICCardReadRxd3);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num&&(ICCardReadTime3==0))
	{
		u8 temp	=	BoardCardDataAnalysis(&ICCardReadRev3);					//读取IC卡数据校验，返回0失败，作废，返回1正确
		if(temp	!=0)
		{
			Status	=	&(sBorad.Port3.sBox);			//箱子状态
			CardBuffer	=	&ICCardReadRev3;				//卡完整数据		
			if(Num	==	22)	//有卡
			{	
				CardReaderTimeCount3	=	0;
				ICCardReadTime3	=	CKtime;				//取卡后500ms读到卡的数据当作无效
				memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
//				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
//				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				if(!((CardData[0]==0x00)&&(CardData[1]==0x00)&&(CardData[2]==0x00)&&(CardData[3]==0x00)))
				{
					Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
					BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
				}
				
			}
			else if(Num	==	6)//无卡
			{
				CardReaderTimeCount3	=	0;
				ICCardReadTime3	=	CKtime;				//取卡后500ms读到卡的数据当作无效
				Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
				BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			}
		}
	}
	//==============================Port4
	if(USART_GetITStatus(ICCardReadPort4, USART_IT_RXNE))
  {
		if(ICCardReadTime4)		//取卡后未到500ms读到数据
		{
			USART_ClearITPendingBit(ICCardReadPort4, USART_IT_RXNE);
			return 1;	//退出，不接收数据
		}
		Port4TimeOut	=	0;		//端口4连续读数超时---清除
		USART_ClearITPendingBit(ICCardReadPort4, USART_IT_RXNE);
    ICCardReadRxd[ICCardReadCount4]	=	USART_ReceiveData(ICCardReadPort4);
		ICCardReadCount4++;
		
		if(ICCardReadCount4==6	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24	&&	ICCardReadRxd[5]==0x2D)
		{
			CardReaderTimeCount4	=	0;
			ICCardReadTime4	=	CKtime;				//取卡后500ms读到卡的数据当作无效
			Status	=	&(sBorad.Port4.sBox);			//箱子状态
			Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			
			ICCardReadCount4=0;
		}
		if(ICCardReadCount4>=22	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24)//有卡
		{
			CardReaderTimeCount4	=	0;
			ICCardReadTime4	=	CKtime;				//取卡后500ms读到卡的数据当作无效
			Status	=	&(sBorad.Port4.sBox);			//箱子状态
			CardBuffer	=	(ICBufferDef*)ICCardReadRxd;				//卡完整数据		
			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
//			Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
//			BoardSaveCardData(&sBorad);														//根据状态保存卡号及设置相关状态
			if(!((CardData[0]==0x00)&&(CardData[1]==0x00)&&(CardData[2]==0x00)&&(CardData[3]==0x00)))
			{
				Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
				BoardSaveCardData(&sBorad);		//根据状态保存卡号及设置相关状态
			}
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
*函数名			:	CardReaderPort4
*功能描述		:	端口4读卡---端口4用UART5，无法使用DMA送数据，只能用中断接收
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
u8 CardReaderPort4(void)	//端口4读卡---端口4用UART5，无法使用DMA送数据，只能用中断接收
{
	u16	Num	=	0;
	sBoxDef*	Status;				//箱子状态
	ICBufferDef*		CardBuffer;		//卡完整数据
	//==============================Port4
	if(USART_GetITStatus(ICCardReadPort4, USART_IT_RXNE))
  {
		Port4TimeOut	=	0;		//端口4连续读数超时---清除
		USART_ClearITPendingBit(ICCardReadPort4, USART_IT_RXNE);
    ICCardReadRxd[ICCardReadCount4]	=	USART_ReceiveData(ICCardReadPort4);
		ICCardReadCount4++;
		
		if(ICCardReadCount4==6	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24	&&	ICCardReadRxd[5]==0x2D)
		{
			Status	=	&(sBorad.Port4.sBox);			//箱子状态
			Status->BoxSts.NoID	=	1;			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
			BoardClrCardData(&sBorad);		//删除卡号数据及设置相关状态
			
			ICCardReadCount4=0;
		}
		if(ICCardReadCount4>=22	&&	ICCardReadRxd[0]==0xD2	&&	ICCardReadRxd[1]==0x24)//有卡
		{
			Status	=	&(sBorad.Port4.sBox);			//箱子状态
			CardBuffer	=	(ICBufferDef*)ICCardReadRxd;				//卡完整数据		
			
			memcpy(CardData,&(CardBuffer->data[CardStartByte]),CardLength);		//复制卡号
			Status->BoxSts.ReadID	=	1;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)		
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
*注释				:	wegam@sina.com
*******************************************************************************/
void CardReadTimeCount(void)			//取卡后倒计时，如果倒计时未完成再收回卡数据，读卡不稳定时的误动作
{	
////防止取卡时读卡不稳定误认为已收回卡：取卡后0.5秒再收到卡数据当作收回
	if(ICCardReadTime1>0)
		ICCardReadTime1--;
	if(ICCardReadTime2>0)
		ICCardReadTime2--;
	if(ICCardReadTime3>0)
		ICCardReadTime3--;
	if(ICCardReadTime4>0)
		ICCardReadTime4--;
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
u8	BoardCardDataAnalysis(ICBufferDef* CardData)					//读取IC卡数据校验，返回0失败，作废，返回1正确
{
	if(CardData->Head	!=0xD2	||	CardData->CmdType	!=0x24)
	{
		return 0;
	}
	else
	{
		return 1;
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
void BoardCardReaderReadAll(sBoradDef*	sBorad)							//向相应端口发送读卡指令
{
	if(sBorad->sPlu.Step.WriteSeg	==	0)		//数码管更新标志为未更新，需要更新
	{
		return;
	}
	if(sBorad->sPlu.Step.ReadCard	==	0)		//数码管更新标志为未更新，需要更新	
	{
		sBorad->sPlu.Time.TimeCard	++;							//读卡器时间
		if(sBorad->sPlu.Time.TimeCard==15)					//等待15ms后开始，由于数码管串口端口与读卡器复用，需要等待
		{
			HALUsartRemapDisable();					//关闭串口复用
		}
		else if(WriteCardFlag==1)//写卡标志，
		{
//			CardReaderCmd_WriteData;
//			USART_DMASend	(CardReadPort1,(u32*)CardReaderCmd_WriteData,sizeof(CardReaderCmd_WriteData));	//串口DMA发送程序
			WriteCardFlag	=	0;			//写卡标志，
		}
		else if(sBorad->sPlu.Time.TimeCard>=20)
		{
//			if(sBorad->Port1.sBus.BusqSts.GetBox	==	1)	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
			USART_DMASend	(ICCardReadPort1,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
//			if(sBorad->Port2.sBus.BusqSts.GetBox	==	1|| sBorad->Port2.sBox.BoxSts.BoxOn ==	0)	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
			USART_DMASend	(ICCardReadPort2,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
//			if(sBorad->Port3.sBus.BusqSts.GetBox	==	1)	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
			USART_DMASend	(ICCardReadPort3,(u32*)CardReaderCmd_ReadData,14);	//串口DMA发送程序
//			if(sBorad->Port4.sBus.BusqSts.GetBox	==	1)	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
			USART_Send		(ICCardReadPort4,(u8*)CardReaderCmd_ReadData,14);			//串口5发送程序
			
			sBorad->sPlu.Step.ReadCard	=	1;				//关读卡
			sBorad->sPlu.Time.TimeCard	=	0;				//读卡器时间
			
			if(CardReaderTimeCount1<100)
			{
				CardReaderTimeCount1++;
			}
			if(CardReaderTimeCount2<100)
			{
				CardReaderTimeCount2++;
			}
			if(CardReaderTimeCount3<100)
			{
				CardReaderTimeCount3++;
			}
			if(CardReaderTimeCount4<100)
			{
				CardReaderTimeCount4++;
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
*注释				:	wegam@sina.com
*******************************************************************************/
void SendReadCardCmd(USART_TypeDef* USARTx)				//发送读卡命令使读卡器发回数据
{

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
		//========药箱：如果原来都为无药箱状态，表示已处理过至少一次删除请求，则不往下执行
		if(Port->sBox.BoxSts.ReadID==1)			//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
		{
			Port->sBox.BoxSts.ReadID	=	0;		//bit2：0-无操作,		1-读到ID(需要增加相关数据及标志)
			if(
						(Port->sReader.Retry++>=1)
					&&((Port->sBox.BoxSts.BoxOn==0)		//bit1：0-无药箱,		1-有药箱
					||(memcmp(Port->sBox.CardData,CardData,CardLength)!=0))	//返回0--比较结果一样，不需要更新卡数据
				)
			{
				//========清除重试
				Port->sReader.Retry	=	0;
				//========药箱：如果原来都为无药箱状态，表示已处理过至少一次删除请求，则不往下执行
				Port->sBox.BoxSts.BoxOn		=	1;	//bit1：0-无药箱,		1-有药箱				
				
				//========数码管
				Port->Seg.sSegSts.SegFlg	=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
				//========读卡器
				Port->sBox.BoxSts.ReadErr		=	0;	//bit4：0-读卡器正常，	1-读卡器通讯异常
				
				//========总线
				if(PowerUpFlag	!=	0)				//上电标志，0未上电完，1上电完
				{
					Port->sBus.BusqSts.BusFlg		=	1;			//bit0：0-无操作,	1-有待处理事件
					Port->sBus.BusqSts.BoxBack	=	1;	//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
				}
				
				//========锁
				Port->Lock.sLockSts.LockOn	=	0;
				Port->Lock.sLockSts.LockSts	=	0;
				Port->Lock.sLockSts.LockTout	=	0;
				Port->Lock.LockOnTime	=	0;
				
				//=====================保存卡号
				memcpy(Port->sBox.CardData,CardData,CardLength);					//复制卡号信息到药箱缓存
				
				memcpy(Port->sBus.CardData,&CardData[1],CardLength-1);		//复制卡号信息到总线缓存---由于卡号上报只有三位，卡缓存为4位，只取后三位有效位
				
				Port->sBox.CardNumber	=	(((Port->sBox.CardData[0]>>4)&0x0F)*10000000)	
															+	(((Port->sBox.CardData[0])&0x0F)*1000000)
				
															+	(((Port->sBox.CardData[1]>>4)&0x0F)*100000)
															+	(((Port->sBox.CardData[1])&0x0F)*10000)
				
															+	(((Port->sBox.CardData[2]>>4)&0x0F)*1000)
															+	(((Port->sBox.CardData[2])&0x0F)*100)
				
															+	(((Port->sBox.CardData[3]>>4)&0x0F)*10)
															+	(((Port->sBox.CardData[3])&0x0F));		//将卡号数据合并为十进制数
															
//				memcpy((u8*)(Port->Seg.SegFarme.data),(u8*)&(Port->sBox.CardNumber),CardLength);			//复制卡号信息到数码管缓存
				Port->Seg.SegFarme.data[0]	=	(Port->sBox.CardNumber>>24)&0xFF;
				Port->Seg.SegFarme.data[1]	=	(Port->sBox.CardNumber>>18)&0xFF;
				Port->Seg.SegFarme.data[2]	=	(Port->sBox.CardNumber>>8)&0xFF;
				Port->Seg.SegFarme.data[3]	=	(Port->sBox.CardNumber>>0)&0xFF;
				
				
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
		sBorad->sPlu.Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->sPlu.Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
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
		//判断BoxSts.NoID
		if(Port->sBox.BoxSts.NoID	==	1)			//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
		{	
			Port->sBox.BoxSts.NoID	=	0;				//bit3：0-无操作,		1-卡号未读到(需要删除相关数据及标志)
			
			//========清除重试
				Port->sReader.Retry	=	0;
			
			//========修改各状态
			//========药箱：如果原来都为无药箱状态，表示已处理过至少一次删除请求，则不往下执行
			if(Port->sBox.BoxSts.BoxOn	==	0)		//bit1：0-无药箱,		1-有药箱	
			{
				return;
			}
			
			Port->sBox.BoxSts.BoxOn	= 0;				//bit1：0-无药箱,		1-有药箱				
			
			//========数码管
			Port->Seg.sSegSts.SegFlg	=	1;			//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
			
			//========读卡器
			Port->sBox.BoxSts.ReadErr	=	0;			//bit4：0-读卡器正常，	1-读卡器通讯异常
			
			//========总线
			Port->sBus.BusqSts.BusFlg	=	1;			//bit0：0-无操作,	1-有待处理事件
			if(Port->sBus.BusqSts.GetBox	==	1)	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
			{
				Port->sBus.BusqSts.GetBox	=	0;			//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
				Port->sBus.BusqSts.GotBox	=	1;			//bit3：0-无操作，	1-药箱已被取走
				Port->sBus.BusqSts.TakeBox	=	0;		//bit4：0-无操作，	1-药箱被强制取
				Port->sBus.BusqSts.BoxBack	=	0;		//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
			}
			else
			{
				Port->sBus.BusqSts.BoxBack	=	0;		//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
				Port->sBus.BusqSts.TakeBox	=	1;		//bit4：0-无操作，	1-药箱被强制取
			}	

			//========清除数据
			memset(Port->sBox.CardData,0x00,CardLength);		//删除药箱缓存卡号信息
			memset(Port->Seg.SegFarme.data,0x00,8);					//删除数码管卡号数据
			memset(Port->sBus.CardData,0x00,3);							//删除总线缓存卡号数据---由于卡号上报只有三位，卡缓存为4位，只取后三位有效位
			Port->sBox.CardNumber	=	0;											//删除十进制卡号
			
			WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管			
		}
		if(Num<MaxPortNum-1)
		{
			Port=(PortDef*)((u32)Port+sizeof(PortDef));				//指向下一端口
		}
	}
	if(WriteSegFlg)				//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	{
		sBorad->sPlu.Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad->sPlu.Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	}
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
	if(sBorad->sPlu.Step.WriteSeg	==	0)		//数码管更新标志为未更新，需要更新	
	{
		sBorad->sPlu.Time.TimeSEG++;						//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		if(sBorad->sPlu.Time.TimeSEG==15)			//等待15ms后开始，由于数码管串口端口与读卡器复用，需要等待
		{
			HALUsartRemapEnable();												//使能串口复用
		}
		if((sBorad->sPlu.Time.TimeSEG%20)	==	0)	//20mS更新一个：通过485接口
		{
			u32	Time;						//点亮时间，单位mS
			u8 Num		=	0;
//			u8 SegFlg	=	0;			//0--无数据需要发送，可以把串口复用关闭，1--需要发送数据，需要等待20ms后关闭才可以根据条件关闭串口复用
			PortDef*	Port	=	&(sBorad->Port1);
			for(Num=0;Num<MaxPortNum;Num++)
			{
				if(Port->Seg.sSegSts.SegFlg	==	1)	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
				{
					//============根据情况发送相关数码管命令
					//============插入药箱常亮30S；取药箱请求：一定时间内闪烁；无药箱：小数点一直闪烁；读卡通讯故障：小数点常亮
					if(Port->sBox.BoxSts.BoxOn	==	1)		//bit1：0-无药箱,		1-有药箱
					{
						if(Port->sBox.BoxSts.ReadErr	==	1)			//bit4：0-读卡器正常，	1-读卡器通讯异常
						{
							Port->Seg.SegFarme.cmd.DispEnNum	=	1;		//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdNum	=	1;		//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispEnDp	=	0;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdDp	=	0;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispTime	=	0;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
							memset((Port->Seg.SegFarme.data),0x00,4);		//000闪烁
						}
						else if(Port->sBus.BusqSts.GetBox	==	1)			//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
						{
							Port->Seg.SegFarme.cmd.DispEnNum	=	1;	//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdNum	=	1;	//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispEnDp	=	0;		//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispTime	=	1;		//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
							
							Time	=	Port->Lock.LockOnTime;					//闪烁时间根据上位机发送的需要锁的吸合时间
						}
						else if(Port->Seg.sSegSts.SegTimeOut	==1)		//bit1：0-无操作,		1-取药超时，需要关闭数码管
						{
							Port->Seg.sSegSts.SegTimeOut	=	0;			//bit1：0-无操作,		1-取药超时，需要关闭数码管
							Port->Seg.SegFarme.cmd.DispEnNum	=	0;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdNum	=	0;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispEnDp	=	0;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdDp	=	0;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispTime	=	1;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
						}
						else		//非取药请求，则表示新插入药箱：常亮
						{
							Port->Seg.SegFarme.cmd.DispEnNum	=	1;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdNum	=	0;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispEnDp	=	0;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispTime	=	1;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
							Time	=	SegOnTime;								//插入药箱后数码管常亮时间30S							
						}
						//=================写入数据
						//==数据,高位在前,data[0~3]为显示内容,data[4~7]为闪烁时间
						//=================写入卡呺:已在读卡器接收中将数据存入数码管缓存

						//=================写入时间
						memcpy((u8*)&(Port->Seg.SegFarme.data[4]),&Time,4);
//						Port->Seg.SegFarme.data[4]	=	(Time>>24)&0xFF;
//						Port->Seg.SegFarme.data[5]	=	(Time>>16)&0xFF;
//						Port->Seg.SegFarme.data[6]	=	(Time>>8)&0xFF;
//						Port->Seg.SegFarme.data[7]	=	(Time>>0)&0xFF;
					}
					else		//bit0：0-无卡,1-有卡//药箱被取走或者读卡器坏
					{
						if(Port->sBox.BoxSts.ReadErr	==	1)			//bit4：0-读卡器正常，	1-读卡器通讯异常
						{
							Port->Seg.SegFarme.cmd.DispEnNum	=	1;		//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdNum	=	1;		//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispEnDp	=	0;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdDp	=	0;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispTime	=	0;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
							memset((Port->Seg.SegFarme.data),0x00,4);		//000闪烁
							
//							Port->Seg.SegFarme.cmd.DispEnNum	=	0;		//bit0显示数值	：	0-不显示，		1-显示
//							Port->Seg.SegFarme.cmd.DispMdNum	=	0;		//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
//							Port->Seg.SegFarme.cmd.DispEnDp	=	1;			//bit1显示点		：	0-不显示，		1-显示
//							Port->Seg.SegFarme.cmd.DispMdDp	=	0;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
//							Port->Seg.SegFarme.cmd.DispTime	=	0;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
						}
						else
						{
							Port->Seg.SegFarme.cmd.DispEnNum	=	0;			//bit0显示数值	：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdNum	=	0;			//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispEnDp	=	1;			//bit1显示点		：	0-不显示，		1-显示
							Port->Seg.SegFarme.cmd.DispMdDp	=	1;			//bit3点模式		：	0-静态显示，	1-0.5S闪烁
							Port->Seg.SegFarme.cmd.DispTime	=	0;			//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
						}
					}
					HALSendSeg((u32*)&Port->Seg.SegFarme,sizeof(sSegFarmeDef));		//向数码管发送数据
					Port->Seg.sSegSts.SegFlg	=	0;					//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
					return;				//退出，发送数据，等待下一个延时时间再做剩余的数码管更新
				}
				if(Num<MaxPortNum-1)
				{
					Port=(PortDef*)((u32)Port+sizeof(PortDef));			//指向下一端口
				}
			}
			
			//执行到这表示以上未检测出数码管更新标志，
			sBorad->sPlu.Step.WriteSeg	=	1;			//bit1:	0-未更新数码管,需要更新，	1-已更新
			sBorad->sPlu.Step.ReadCard	=	0;			//关读卡---更新完数码管继续读卡检测
			HALUsartRemapDisable();					//关闭串口复用
		}
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
//void Lock_Server(sBoradDef*	sBorad)		//锁操作，根据_LockFlag相应标志判断是否需要打开锁
//{
//	u8 Num	=	0;
//	PortDef*	Port	=	&(sBorad->Port1);

//	for(Num=0;Num<MaxPortNum;Num++)
//	{
//		if(Port->Lock.sLockSts.LockOn	==	1)		//bit1：0-释放锁，1-开锁
//		{
//			if(Port->Lock.LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
//			{
//				Port->Lock.LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
//				Port->Lock.sLockSts.LockSts	=1;		//bit2：0-已释放，1-已开锁
//				GPIO_SetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//开锁
//			}
//			else
//			{
//				Port->Lock.sLockSts.LockOn	=0;		//bit1：0-释放锁，1-开锁
//				Port->Lock.sLockSts.LockSts	=0;		//bit2：0-已释放，1-已开锁
//				GPIO_ResetBits(Port->Lock.GPIOx,	Port->Lock.GPIO_Pin_n);			//释放锁
//			}		
//		}
//		if(Num<MaxPortNum-1)
//		{
//			Port=(PortDef*)((u32)Port+sizeof(PortDef));				//指向下一端口
//		}
//	}	
//}
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
	USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,CardBufferSize);	//USART_DMA配置--查询方式，不开中断
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
void HALSendSeg(u32* Buffer,u16 Length)		//向数码管发送数据
{
//	HALUsartRemapEnable();											//使能串口复用
	RS485_DMASend(&RS485_Seg7,Buffer,Length);	//RS485-DMA发送程序
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
	USART_DMA_ConfigurationNR	(ICCardReadPort1,ICCardReadBaudRate,(u32*)&ICCardReadRxd,CardBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,CardBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort3,ICCardReadBaudRate,(u32*)&ICCardReadRxd3,CardBufferSize);	//USART_DMA配置--查询方式，不开中断
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
		Port->Seg.SegFarme.desAddr	=	Num+1;			//分配地址：1~4
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
/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	
*******************************************************************************/
void CardReaderError(void)			//重置相关变量
{
//	sBorad
	u8 WriteSegFlg	=	0;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	if((CardReaderTimeCount1==6)	&&(sBorad.Port1.sBox.BoxSts.ReadErr==0))
	{
		sBorad.Port1.sBox.BoxSts.ReadErr	=	1;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port1.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	else if((CardReaderTimeCount1<5)&&(sBorad.Port1.sBox.BoxSts.ReadErr==1))
	{
		sBorad.Port1.sBox.BoxSts.ReadErr	=	0;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port1.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	
	if((CardReaderTimeCount2==6)	&&(sBorad.Port2.sBox.BoxSts.ReadErr==0))
	{
		sBorad.Port2.sBox.BoxSts.ReadErr	=	1;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port2.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	else if((CardReaderTimeCount2<5)&&(sBorad.Port2.sBox.BoxSts.ReadErr==1))
	{
		sBorad.Port2.sBox.BoxSts.ReadErr	=	0;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port2.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	
	if((CardReaderTimeCount3==6)	&&(sBorad.Port3.sBox.BoxSts.ReadErr==0))
	{
		sBorad.Port3.sBox.BoxSts.ReadErr	=	1;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port3.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	else if((CardReaderTimeCount3<5)&&(sBorad.Port3.sBox.BoxSts.ReadErr==1))
	{
		sBorad.Port3.sBox.BoxSts.ReadErr	=	0;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port3.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	
	if((CardReaderTimeCount4==6)	&&(sBorad.Port4.sBox.BoxSts.ReadErr==0))
	{
		sBorad.Port4.sBox.BoxSts.ReadErr	=	1;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port4.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}
	else if((CardReaderTimeCount4<5)&&(sBorad.Port4.sBox.BoxSts.ReadErr==1))
	{
		sBorad.Port4.sBox.BoxSts.ReadErr	=	0;	//bit4：0-读卡器正常，	1-读卡器通讯异常
		sBorad.Port4.Seg.sSegSts.SegFlg		=	1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		WriteSegFlg	=	1;		//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	}

	
	if(WriteSegFlg)				//需要更新数码管标志，只要有一个数据有更新，变更新数码管
	{
		sBorad.sPlu.Step.WriteSeg	=	0;			//bit1:	0-未更新数码管,需要更新，	1-已更新
		sBorad.sPlu.Time.TimeSEG	=	0;			//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	}

}
//================

#endif

