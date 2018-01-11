#ifdef PC016V20

#include "PC016V20.H"
#include "HC_PHY.H"


//sCommunicationDef	gProtocolBuffer;
SegFlgDef	SegFlg;	//更新数码管结构体
//=================设置读卡器读卡区域	
const u8 ICCARD_CMD_SetReaderArea[] =
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
sSegDef	sSeg	=	
{
	0x01,//uint8_t desAddr;	//拔码地址	/*目的地址*/
	0x00,//uint8_t srcAddr;		/*源地址*/
	0x01,//uint8_t runningNumber;		/*流水号*/
	0x01,//uint8_t cmd;		//01	/*命令*/
	0x08,//uint8_t len;		//08	/*长度*/
	0x01,//uint8_t data[8];		/*数据,高位在前,data[0~3]为显示内容,data[4~7]为闪烁时间,0表示不闪烁*/
	0x03,
	0x04,
	0x05,
	0x00,
	0x00,
	0x00,
	0x00
};			//数码管数据结构体
SWITCHID_CONF	SWITCHID;			//拔码开关

//LockFlagDef	LockFlag;				//锁控制标志0--不处理，1--开锁操作
//==============485结体体
RS485_TypeDef	RS485_Bus;				//总线通讯485
RS485_TypeDef	RS485_Seg7;				//数码管显示485

//==============485缓冲区
sFarmeDef	sFarme;				//总线485数据结构体---待处理
sFarmeDef	sFarmeTxd;		//总线485数据结构体---发送
sFarmeDef	sFarmeRxd;		//总线485数据结构体---接收
sFarmeDef	sFarmeRev;		//总线485数据结构体---缓存
u8 RS485BusTxd[Bus485BufferSize];

sCommunicationDef	sCommData;				//总线485数据结构体---待处理
sCommunicationDef	sCommDataTxd;			//总线485数据结构体---发送
sCommunicationDef	sCommDataRxd;			//总线485数据结构体---接收
sCommunicationDef	sCommDataRev;			//总线485数据结构体---缓存
//sCommunicationDef	CommData4;			//总线485数据结构体---端口4
//u8 RS485BusTxd[Bus485BufferSize]	=	{0};
//u8 RS485BusRxd[Bus485BufferSize]	=	{0};
//u8 RS485BusRev[Bus485BufferSize]	=	{0};

u8 Seg485Txd[Seg485BufferSize]	=	{0};
u8 Seg485Rxd[Seg485BufferSize]	=	{0};
u8 Seg485Rev[Seg485BufferSize]	=	{0};

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
//sGetIDflagDef	sGetID;					//获取ID请求标志
//PortFlagDef	PortFlag;				//端口状态--：有卡/无卡
//sBoxStatusDef	sBoxStatus;			//各药箱状态
sBoradDef			sBorad;					//端口/槽位所有信息结构体
sBoxStatusDef	sBoxStatus1;	//药箱1状态
sBoxStatusDef	sBoxStatus2;	//药箱2状态
sBoxStatusDef	sBoxStatus3;	//药箱3状态
sBoxStatusDef	sBoxStatus4;	//药箱4状态

u8 CardData[CardLength];		//卡有效数据缓存
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
eRunStepDef	eRunStep;		//运行状态
u8 SwitchData	=	0;				//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
vu16	DSPNum	=	0;	//测试显示值
u32	SYSTIME	=	0;	//系统计时器
TimeDef	SYSTime;		//系统运行相关计时器
u32	Port4TimeOut;	//端口4读卡超时，
//u32	LockOnTime[4]	=	{0};	//锁吸合时间，如果时间为0则释放锁






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
	
	

	SysTick_DeleyS(1);							//SysTick延时nS	
	CardReader_Set();							//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44
	SysTick_DeleyS(2);							//SysTick延时nS
	ICCardReader_ReadAll();			//发送4个端口读卡指令

	
//	//==================上电读卡
//	ICCardReader_Read(1);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
//	ICCardReader_Read(2);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
//	ICCardReader_Read(3);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
//	ICCardReader_Read(4);					//读相应端口的卡
//	SysTick_DeleymS(100);					//SysTick延时nmS
	
	
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
	SYSTIME++;
	if(SYSTIME>=1000)
	{
		SYSTIME	=	0;
		ICCardReader_ReadAll();			//发送4个端口读卡指令
	}
	Port4TimeOut++;
	if(Port4TimeOut>=5)
	{
		Port4TimeOut	=	0;
		ICCardReadCount4	=	0;
	}
	
	IWDG_Feed();			//独立看门狗喂狗


	BoardSetSeg(&sBorad);			//根据标志设置数码管状态	

		
//		ResetData();						//重置相关变量
//		
//		ICCardReader_ReadAll();
//		for(Num=0;Num<4;Num++)
//		{
//			ICCardReader_CheckCard();							//
//		}
//		ICCardReader_Read(4);						//读相应端口的卡
//		ICCardReadRev[0]=0xA5;
//		ICCardReadRev[1]=0x01;
//		ICCardReadRev[2]=0x02;
//		USART_DMASend	(ICCardReadPort2,(u32*)ICCardReadRev,1);	//RS485-DMA发送程序


	Lock_Server(&sBorad);		//锁操作，根据_LockFlag相应标志判断是否需要打开锁
////	RS485_DMASend(&RS485_Bus,(u32*)&CommData,11);	//RS485-DMA发送程序
	RS485_Server();											//RS485收发处理
	ProtocolFrameAnalysis(&sCommData,&sCommDataTxd);	//根据协议分析数据
//	ResetData();						//重置相关变量
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
void ProtocolFrameServer(void)		//协议服务
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
void ProtocolFrameAnalysis(sCommunicationDef* sCommData,sCommunicationDef* sCommDataTxd)	//根据协议分析数据
{
//	if(!(IS_eCommHead(sCommData->sHead.eHead1)&&IS_eCommHead(sCommData->sHead.eHead2)))						//检查头标识符文件
//	{
//		return;			//头标识符已经被清除，当前数据处理完
//	}
//	if((sCommData->sStart.eCmd==eCommCmd17)&&(sCommData->sAddr.Addr2==SwitchData))				//获取指定IC卡号指令
//	{
//		memcpy((u8*)sCommDataTxd,(u8*)sCommData,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef));
//		memset((u8*)sCommData,0x00,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef));
//		BoardGetCardData(sCommDataTxd);			//获取IC卡数据或者状态
//	}
//	else if((sCommData->sStart.eCmd==eCommCmd18)&&(sCommData->sAddr.Addr2==SwitchData))			//开锁取药箱---本层操作：需要判断药箱是否取走
//	{
//		memcpy((u8*)sCommDataTxd,(u8*)sCommData,CommProtocolLenth+sCommData->sStart.nLength-CommBasicLenth);
//		memset((u8*)sCommData,0x00,CommProtocolLenth+sCommData->sStart.nLength-CommBasicLenth);
//		BoardSetLockTime(sCommDataTxd);				//设置锁吸合时间
//		BoardSetBoxStatus1(sCommDataTxd);			//设置药箱检测标志位
//		SysTick_DeleymS(100);									//SysTick延时nmS
//	}
//	else if((sCommData->sStart.eCmd==eCommCmd18)&&(sCommData->sAddr.Addr2+1 ==SwitchData))	//开锁取药箱：由于锁是由上层板控制，所以，开锁指令地址需要加1
//	{
//		memcpy((u8*)sCommDataTxd,(u8*)sCommData,CommProtocolLenth+sCommData->sStart.nLength-CommBasicLenth);
//		memset((u8*)sCommData,0x00,CommProtocolLenth+sCommData->sStart.nLength-CommBasicLenth);
//		BoardSetLockTime(sCommDataTxd);														//设置锁吸合时间
//		BoardSetBoxStatus2(sCommDataTxd);													//设置开锁标志位
//		sCommDataTxd->sStart.eCmd	=(eCommCmdDef)eCommAck147;				//开锁取药箱
//		sCommDataTxd->eErrorCode		=	eCommErr00;											//无错误
//		sCommDataTxd->sStart.nLength	=	CommBasicLenth;								//数据段长度：地址+状态码+Data
//	//	sCommData->data[0]	=	LockOnTime[sCommData->sAddr.Addr3-1]/1000;	//锁需要吸合时间
//	}
//	ProtocolFrameSend(sCommDataTxd);				//根据协议打包发送帧数据
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
void ProtocolFrameGet(void)	//根据协议分析数据
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
void ProtocolFrameSet(void)	//根据协议分析数据
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
void ProtocolFrameSend(sCommunicationDef* sCommData)	//按照协议打包数据
{
	sCommData->sStart.nBcc8	=	BCC8((u8*)&(sCommData->sAddr.Addr1),sCommData->sStart.nLength);		//8位数异或校验码：地址段+异常码+数据段校验
	RS485_DMASend(&RS485_Bus,(u32*)sCommData,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+sCommData->sStart.nLength);				//RS485-DMA发送程序
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
void BusFrameAck(sFarmeDef*	sFarme)			//外部总线应答
{
	SYSTime.TimeBUS	=	0;
	RS485BusTxd[0]	=	0x7E;
	RS485BusTxd[1]	=	sFarme->sAdd;
	RS485BusTxd[2]	=	sFarme->dAdd;
	RS485BusTxd[3]	=	sFarme->SN;
	RS485BusTxd[4]	=	sFarme->Cmd;
	RS485BusTxd[5]	=	sFarme->Ucode;
	RS485BusTxd[6]	=	0x00;				//错误码
	RS485BusTxd[7]	=	BCC8((u8*)RS485BusTxd,6);		//8位数异或校验码：地址段+异常码+数据段校验
	RS485BusTxd[8]	=	0x7F;
	
	RS485_DMASend(&RS485_Bus,(u32*)RS485BusTxd,9);	//RS485-DMA发送程序
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
void BusFrameProcess(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//外部总线应答
{
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
	Port->Cmd	=	sFarme->Cmd;
	Port->Ucode	=	sFarme->Ucode;
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
void BusFrameProcessData(sFarmeDef*	sFarme,sBoradDef*	sBorad)			//数据命令处理
{
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
	Port->Cmd	=	sFarme->Cmd;
	Port->Ucode	=	sFarme->Ucode;
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


/*******************************************************************************
*函数名			:	function
*功能描述		:	function
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:	wegam@sina.com
*******************************************************************************/
void BoardServer(sBoradDef*	sBorad)																//获取IC卡数据
{

}



/*******************************************************************************
*函数名			:	BoardGetCardData
*功能描述		:	获取IC卡数据，根据需要获取的端口号及相应端口状态复制相应IC卡号及设置相应标志
*输入				: 
*返回值			:	无
*修改时间		:	无
*修改说明		:	无
*注释				:
*******************************************************************************/
void BoardGetCardData(sCommunicationDef* sCommData)			//获取IC卡数据或者状态
{
	sBoxStatusDef	sBoxStatus;		//药箱状态
	u8* ICCardNum;							//IC卡号
	sCommData->sStart.eCmd	=	(eCommCmdDef)eCommAck146;		//获取指定IC卡号应答
	if(sCommData->sAddr.Addr3	==ePort1)						//==========请求端口1
	{
		sBoxStatus	=	sBoxStatus1;
		ICCardNum		=	ICData.Port1Data;
	}
	else if(sCommData->sAddr.Addr3	==ePort2)			//==========请求端口2
	{
		sBoxStatus	=	sBoxStatus2;
		ICCardNum		=	ICData.Port2Data;
	}
	else if(sCommData->sAddr.Addr3	==ePort3)			//==========请求端口3
	{
		sBoxStatus	=	sBoxStatus3;
		ICCardNum		=	ICData.Port3Data;
	}
	else if(sCommData->sAddr.Addr3	==ePort4)			//==========请求端口4
	{
		sBoxStatus	=	sBoxStatus4;
		ICCardNum		=	ICData.Port4Data;
	}
	else
	{
		return;
	}
	//==============================根据药箱状态修改相关返回参数
	if(sBoxStatus.BoxRead==1)				//bit3：0-读卡器正常，1-读卡器通讯异常
	{
		sCommData->eErrorCode		=	eCommErr39;		//读卡器通讯异常
		sCommData->sStart.nLength	=	sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)+0;
	}
	else if(sBoxStatus.BoxFlg==0)		//bit0：0无卡,1有卡
	{
		if(sBoxStatus.BoxPro==0)				//bit1：0药箱无操作记录，1药箱有操作记录
		{
			sCommData->eErrorCode		=	eCommErr35;		//无药箱数据/无卡
			sCommData->sStart.nLength	=	sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)+0;
		}
		else
		{
			sCommData->eErrorCode		=	eCommErr37;		//药箱被取走
			sCommData->sStart.nLength	=	sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)+0;
		}
	}
	else if(sBoxStatus.BoxFlg==1)		//bit0：0无卡,1有卡
	{
		if(sBoxStatus.BoxPro==0)				//bit1：0药箱无操作记录，1药箱有操作记录
		{
			sCommData->eErrorCode		=	eCommErr00;		//无错误
			sCommData->sStart.nLength	=	sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)+ICDataNum;
			memcpy(&(sCommData->data),ICCardNum,ICDataNum);
		}
		else			//bit1：0药箱无操作记录，1药箱有操作记录
		{
			sCommData->eErrorCode		=	eCommErr38;		//药箱收回
			sCommData->sStart.nLength	=	sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)+ICDataNum;
			memcpy(&(sCommData->data),ICCardNum,ICDataNum);
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
void BoardSaveCardData(sBoradDef*	sBorad)		//根据状态保存卡号及设置相关状态
{
	PortDef*	Port	=	&sBorad->Port1;					//指向端口1
	
	if(Port->Status.RecData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		Port	=	&sBorad->Port2;					//指向端口1
	}
	if(Port->Status.RecData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		Port	=	&sBorad->Port3;					//指向端口1
	}
	if(Port->Status.RecData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		Port	=	&sBorad->Port4;					//指向端口1
	}
	if(Port->Status.RecData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{	
		return;
	}
	//========根据标志位设置相关状态
	
	Port->Status.RecData	=	0;				//bit7：0-无接收到卡数据，1-有接收到卡数据
	Port->Status.ClrData	=	0;				//bit8：0-无操作，1-请求清除药箱在线状态
	Port->Status.BoxFlg		=	1;				//bit0：0-无卡,1-有卡;
	Port->Status.BoxRead	=	0;				//bit3：0-读卡器正常，1-读卡器通讯异常
	
	if(Port->Status.BoxPro	==	1)		//bit1：0-药箱无操作记录，1-药箱有操作记录（取药）:之前操作为取药，重新获取到卡表示药箱已收回
	{
		Port->Status.BoxPro	=	0;				//bit1：0-药箱无操作记录，1-药箱有操作记录（取药）:之前操作为取药，重新获取到卡表示药箱已收回
		Port->Status.BoxSta	=	1;				//bit2：0-药箱取走，1-药箱收回,收回后需要上报药箱号
		Port->Status.BoxSeg	=	1;				//bit4：0-无操作，1-需要未将卡号发给数码管
	}
	memcpy(Port->CardData,CardData,CardLength);		//复制卡号信息
	Port->CardNumber	=	(((Port->CardData[0]>>4)&0x0F)*10000000)	
										+	(((Port->CardData[0])&0x0F)*1000000)
	
										+	(((Port->CardData[1]>>4)&0x0F)*100000)
										+	(((Port->CardData[1])&0x0F)*10000)
	
										+	(((Port->CardData[2]>>4)&0x0F)*1000)
										+	(((Port->CardData[2])&0x0F)*100)
	
										+	(((Port->CardData[3]>>4)&0x0F)*10)
										+	(((Port->CardData[3])&0x0F));		//将卡号数据合并为十进制数
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
void BoardClrCardData(sBoradDef*	sBorad)		//删除卡号数据及设置相关状态
{
	PortDef*	Port	=	&sBorad->Port1;					//指向端口1
	
	if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		Port	=	&sBorad->Port2;					//指向端口1
	}
	if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		Port	=	&sBorad->Port3;					//指向端口1
	}
	if(Port->Status.ClrData	!=	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		Port	=	&sBorad->Port4;					//指向端口1
	}
	if(Port->Status.ClrData	==	1)	//bit8：0-无操作，1-请求清除药箱在线状态
	{			
		//========根据标志位设置相关状态
		Port->Status.ClrData	=	0;				//bit8：0-无操作，1-请求清除药箱在线状态
		Port->Status.BoxFlg		=	0;				//bit0：0-无卡,1-有卡;
		Port->Status.BoxRead	=	0;				//bit3：0-读卡器正常，1-读卡器通讯异常
		Port->Status.RecData	=	0;				//bit7：0-无接收到卡数据，1-有接收到卡数据
		Port->Status.BoxPro		=	1;				//bit1：0-药箱无操作记录，1-药箱有操作记录（取药）:之前操作为取药，重新获取到卡表示药箱已收回
		Port->Status.BoxSeg		=	1;				//bit4：0-无操作，1-需要未将卡号发给数码管
		if(Port->Status.BoxDel	==	1)		//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
		{	
			Port->Status.BoxDel	=	0;				//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
			Port->Status.BoxSta	=	0;				//bit2：0-药箱取走，1-药箱收回,收回后需要上报药箱号
			
		}
		memset(Port->CardData,0x00,CardLength);		//删除卡号信息	
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
void BoardSetSeg(sBoradDef*	sBorad)			//根据标志设置数码管状态
{
	//	Seg485Txd[0]=
//	u32 Num=0;
//	DSPNum++;
//	if(DSPNum>5000)
//	{
//		DSPNum	=	0;
	u8 DesAddr	=	0;				//数码管地址
	PortDef*				Port;

	SYSTime.TimeSEG	++;					//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	if(SYSTime.TimeSEG%100	==	10)
	{
		GPIO_PinRemapConfig(GPIO_Remap_USART1,ENABLE);				//I/O口重映射开启
		return;
	}
	else if(SYSTime.TimeSEG%100	==	20)
	{
		DesAddr	=	1;				//数码管地址
		Port	=	&(sBorad->Port1);
	}
	else if(SYSTime.TimeSEG%100	==	40)
	{
		DesAddr	=	2;				//数码管地址
		Port	=	&(sBorad->Port2);
	}
	else if(SYSTime.TimeSEG%100	==	60)
	{
		DesAddr	=	3;				//数码管地址
		Port	=	&(sBorad->Port3);
	}
	else if(SYSTime.TimeSEG%100	==	80)
	{
		DesAddr	=	4;				//数码管地址
		Port	=	&(sBorad->Port4);
	}
	else if((SYSTime.TimeSEG%100==0)&&(SYSTime.TimeSEG>=1000))
	{
		SYSTime.TimeSEG	=	0;	//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
		GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);				//I/O口重映射关闭
		return;
	}
	
	if(Port->Status.BoxSeg	==	1)		//bit4：0-无操作，1-需要未将卡号发给数码管
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
			if(Port->Status.BoxDel	==	0)	//bit6：0-无待处理操作，1-移除药箱请求(数码管数值闪烁)
			{
				Time	=	10000;		//10秒
				Port->Seg.cmd.DispMdNum	=	0;	//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
				
			}
			//----------------------取出药箱：闪烁60秒
			else if(Port->Status.BoxDel	==	1)	//bit6：0-无待处理操作，1-移除药箱请求(数码管数值闪烁)
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
		RS485_DMASend(&RS485_Seg7,(u32*)&Port->Seg,sizeof(sSegDef));	//RS485-DMA发送程序
	}
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
void ICCardReader_Configuration(void)			//选择相应端口读卡器配置
{
	USART_DMA_ConfigurationNR	(ICCardReadPort1,ICCardReadBaudRate,(u32*)&ICCardReadRxd,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_DMA_ConfigurationNR	(ICCardReadPort3,ICCardReadBaudRate,(u32*)&ICCardReadRxd3,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
	USART_ConfigurationIT(ICCardReadPort4,ICCardReadBaudRate,0,0);	//USART_配置---常规中断方式//UART5不支持DMA传输
	
//	USART_DMA_ConfigurationNRRemap	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置(映射)--查询方式，不开中断
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
	u8	Num	=	1;
	LockFlagDef*	Lock;				//锁信息
	
	Lock	=	&(sBorad->Port1.Lock);	
	if(Lock->On	==1)				//0-不处理，1-开锁
	{
		if(Lock->LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
		{
			Lock->LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
			LockOn(1);						//开锁
		}
		else
		{
			Lock->On	=0;					//0-不处理，1-开锁
			LockOff(1);						//释放锁
		}
	}
	
	Lock	=	&(sBorad->Port2.Lock);	
	if(Lock->On	==1)				//0-不处理，1-开锁
	{
		if(Lock->LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
		{
			Lock->LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
			LockOn(2);						//开锁
		}
		else
		{
			Lock->On	=0;					//0-不处理，1-开锁
			LockOff(2);						//释放锁
		}
	}
	
	Lock	=	&(sBorad->Port3.Lock);	
	if(Lock->On	==1)				//0-不处理，1-开锁
	{
		if(Lock->LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
		{
			Lock->LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
			LockOn(3);						//开锁
		}
		else
		{
			Lock->On	=0;					//0-不处理，1-开锁
			LockOff(3);						//释放锁
		}
	}
	
	Lock	=	&(sBorad->Port4.Lock);	
	if(Lock->On	==1)				//0-不处理，1-开锁
	{
		if(Lock->LockOnTime>0)	//锁吸合时间，最大值0x3FFF FFFF
		{
			Lock->LockOnTime--;		//锁吸合时间，最大值0x3FFF FFFF
			LockOn(4);						//开锁
		}
		else
		{
			Lock->On	=0;					//0-不处理，1-开锁
			LockOff(4);						//释放锁
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
	
	SYSTime.TimeBUS++;
	if(SYSTime.TimeBUS>=1000)
	{
		SYSTime.TimeBUS	=	0;
	}
	
	Num	=	RS485_ReadBufferIDLE(&RS485_Bus,(u32*)&sFarmeRev,(u32*)&sFarmeRxd);	//串口空闲模式读串口接收缓冲区，如果有数据，将数据拷贝到RevBuffer,并返回接收到的数据个数，然后重新将接收缓冲区地址指向RxdBuffer
	if(Num	==	0)
	{
		return;
	}
	if((sFarmeRev.Head!=0x7E)||(sFarmeRev.dAdd!=SwitchData)||((sFarmeRev.dAdd!=SwitchData+1)))		//地址不对//开锁取药箱需要两层操作，由于锁是由上层板控制，所以，开锁指令地址需要加1；本层需要检查药箱是否取走
	{
		return;
	}
	if(!((sFarmeRev.Cmd==eCommCmd17)&&(sCommDataRev.sAddr.Addr2 ==SwitchData)))			//获取指定IC卡号指令,获取IC卡号为本层操作
	{
		return;
	}
	if(!((sFarmeRev.Cmd==eCommCmd18)&&(sCommDataRev.sAddr.Addr2 ==SwitchData+1)))		//取药指令，取药指令由上层开锁
	{
		return;
	}
	//=======================执行操作
	if((sFarmeRev.Cmd==eCommCmd18)&&(sCommDataRev.sAddr.Addr2 ==SwitchData+1))			//开锁取药箱需要两层操作，由于锁是由上层板控制，所以，开锁指令地址需要加1；本层需要检查药箱是否取走
	{
		BusFrameProcessLock(&sFarmeRev,&sBorad);	//锁命令处理	
	}	
	else
	{
		BusFrameAck(&sFarmeRev);										//外部总线应答
		BusFrameProcessData(&sFarmeRev,&sBorad);		//数据命令处理
	}
	//========================获取消息数据
	
	
//	if(	(IS_eCommHead(sCommDataRev.sHead.eHead1)&&IS_eCommHead(sCommDataRev.sHead.eHead2))			//检查头识别符
//		&&(((sCommDataRev.sStart.eCmd==eCommCmd17)&&(sCommDataRev.sAddr.Addr2 	==SwitchData))		//获取指定IC卡号指令
//		||((sCommDataRev.sStart.eCmd==eCommCmd18)&&((sCommDataRev.sAddr.Addr2+1 ==SwitchData)||(sCommDataRev.sAddr.Addr2==SwitchData))))		//开锁取药箱需要两层操作，由于锁是由上层板控制，所以，开锁指令地址需要加1；本层需要检查药箱是否取走
//		&&((sCommDataRev.sAddr.Addr3>=MinPortNum)&&(sCommDataRev.sAddr.Addr3		<=MaxPortNum))		//检查端口号有无超出范围
//		)
//	{
//		memcpy((u8*)&sCommData,(u8*)&sCommDataRev,Num);			//复制数据到待处理区域
//	}
//	else
//	{
////		memset((u8*)&sCommDataRev,0x00,Num);								//数据无效，清除
//	}
	


//	//================获取指定IC号命令
//	if(	
//					CommData.sHead.eHead1	==eCommHead1	//识别符1	0xFA
//			&&	CommData.sHead.eHead2	==eCommHead2	//识别符2	0xF5
//			&&	CommData.sStart.eCmd	==eCommCmd17	//获取指定IC号命令
//			&&	CommData.sAddr.Addr2 	==SwitchData	//层号
//		)			//接收到数据并且标识符检测正确
//	{
//		ICCardDataPacket();											//数据打包--对获取IC卡号命令的处理
//		RS485_DMASend(&RS485_Bus,(u32*)&CommData,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+CommData.sStart.nLength);	//RS485-DMA发送程序
//	}
//	//================开锁命令
//	else if(	
//							CommData.sHead.eHead1	==eCommHead1		//识别符1	0xFA
//					&&	CommData.sHead.eHead2	==eCommHead2		//识别符2	0xF5
//					&&	CommData.sStart.eCmd	==eCommCmd18		//开锁取药箱命令
//					&&	CommData.sAddr.Addr2 	==SwitchData-1	//层号：由于锁是跨层板控制，所以地址需要+1
//					&&	CommData.sAddr.Addr3	!=0							//端口号不为0（端口为1~4）
//					)			//接收到数据并且标识符检测正确
//	{		
//		LockOnTimeSet();												//设置锁吸合时间
//		RS485_DMASend(&RS485_Bus,(u32*)&CommData,sizeof(sCommHeadDef)+sizeof(sCommStartDef)+CommData.sStart.nLength);	//RS485-DMA发送程序
//	}	
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
	u16	Num	=	0;
	sBoxStatusDef*	Status;				//箱子状态
	ICBufferDef*		CardBuffer;		//卡完整数据	
	
	sBoxStatusDef	*TempStus;	//获取状态参数地址
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
void ICCardReader_Read(u8 Num,u32* Cmd)							//向相应端口发送读卡指令
{
	if(Num	==	0)
	{
		return;
	}
	else if(Num	==	1)
	{
		USART_DMASend	(ICCardReadPort1,Cmd,14);	//串口DMA发送程序
	}
	else if(Num	==	2)
	{
		GPIO_PinRemapConfig(GPIO_Remap_USART1,DISABLE);				//I/O口重映射开启
		USART_DMA_ConfigurationNR	(ICCardReadPort2,ICCardReadBaudRate,(u32*)&ICCardReadRxd2,ICCardReadBufferSize);	//USART_DMA配置--查询方式，不开中断
		USART_DMASend	(ICCardReadPort2,Cmd,14);	//串口DMA发送程序
	}
	else if(Num	==	3)
	{
		USART_DMASend	(ICCardReadPort3,Cmd,14);	//串口DMA发送程序
	}
	else if(Num	==	4)
	{
		USART_Send(ICCardReadPort4,(u8*)Cmd,14);
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
void ICCardReader_ReadAll(void)							//向相应端口发送读卡指令
{
	u8 Num	=	0;
	for(Num=0;Num<4;Num++)
	{
		ICCardReader_Read(Num+1,(u32*)&ReadCmd);					//读相应端口的卡
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
void CardReader_Set(void)							//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44
{
	u8 Num	=	0;
	for(Num=0;Num<4;Num++)
	{
		ICCardReader_Read(Num+1,(u32*)&ICCARD_CMD_SetReaderArea);					//读相应端口的卡
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
void ICCardReader_CheckCard(void)							//向相应端口发送读卡指令
{

//	if(LockFlag.Lock1)
//	{
//		ICCardReader_Read(1,(u32*)&ReadCmd);					//读相应端口的卡
//	}
//	if(LockFlag.Lock2)
//	{
//		ICCardReader_Read(2,(u32*)&ReadCmd);					//读相应端口的卡
//	}
//	if(LockFlag.Lock3)
//	{
//		ICCardReader_Read(3,(u32*)&ReadCmd);					//读相应端口的卡
//	}
//	if(LockFlag.Lock4)
//	{
//		ICCardReader_Read(4,(u32*)&ReadCmd);					//读相应端口的卡
//	}
}
///*******************************************************************************
//*函数名			:	function
//*功能描述		:	function
//*输入				: 
//*返回值			:	无
//*修改时间		:	无
//*修改说明		:	无
//*注释				:		
//*******************************************************************************/
//void ICCardDataPacket(void)		//数据打包--对获取IC卡号命令的处理
//{
//	sCommData.sStart.eCmd	=	(eCommCmdDef)eCommAck146;						//获取指定IC号
//	sCommData.sStart.nLength	=	4+0;		//数据段长度：地址+状态码+Data--->先清零数据段长度，如果有数据，则会增加ICCardDataSize长度数据
//	
//	//==============================复制数据
//	if(sCommData.sAddr.Addr3==1	&&	PortFlag.Port1Flg!=0)						//端口1IC卡号并且有卡号
//	{
//			memcpy(sCommData.data,ICData.Port1Data,ICCardDataSize);	//复制卡号
//			sCommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data	
//	}
//	else if(sCommData.sAddr.Addr3==2	&&	PortFlag.Port2Flg!=0)				//端口2IC卡号并且有卡号
//	{
//		memcpy(sCommData.data,ICData.Port2Data,ICCardDataSize);	//复制卡号
//		sCommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data
//	}
//	else if(sCommData.sAddr.Addr3==3	&&	PortFlag.Port3Flg!=0)				//端口3IC卡号并且有卡号
//	{
//		memcpy(sCommData.data,ICData.Port3Data,ICCardDataSize);	//复制卡号
//		sCommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data
//	}
//	else if(sCommData.sAddr.Addr3==4	&&	PortFlag.Port4Flg!=0)				//端口4IC卡号并且有卡号
//	{
//		memcpy(sCommData.data,ICData.Port4Data,ICCardDataSize);	//复制卡号
//		sCommData.sStart.nLength	=	4+ICCardDataSize;							//数据段长度：地址+状态码+Data
//	}
//	//==============================根据有无数据更新状态码
//	if(sCommData.sStart.nLength>4)						//CommData.data内有数据
//	{
//		sCommData.eErrorCode		=	eCommErr00;											//无错误
//	}
//	else																		//CommData.data内无数据
//	{
//		sCommData.eErrorCode		=	eCommErr35;											//无药箱数据
//	}
//	//==============================数据校验
//	sCommData.sStart.nBcc8	=	BCC8((u8*)&(sCommData.sAddr.Addr1),sCommData.sStart.nLength);
//}

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
//	//==============================根据相应端口设置相应时间
//	if(sCommData.data[0]	==	0)			//吸合时间为0表示使用默认值
//	{
//		LockOnTime[sCommData.sAddr.Addr3-1]	=	DefaultOnTime;		//默认锁吸合时间
//	}
//	else if(sCommData.data[0]	>	MaxOnTime/1000)					//吸合时间超出最大时间
//	{
//		LockOnTime[sCommData.sAddr.Addr3-1]	=	MaxOnTime;		//锁吸合最大时间
//	}
//	else
//	{
//		LockOnTime[sCommData.sAddr.Addr3-1]	=	((u32)sCommData.data[0])*1000;		//锁吸合时间
//	}
//	
//	//==============================封装应答数据
//	sCommData.sStart.eCmd	=	(eCommCmdDef)eCommAck147;			//开锁取药箱
//	sCommData.eErrorCode		=	eCommErr00;										//无错误
//	sCommData.sStart.nLength	=	4+1;												//数据段长度：地址+状态码+Data
//	sCommData.sStart.nBcc8	=	BCC8((u8*)&(sCommData.sAddr.Addr1),sCommData.sStart.nLength);
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

