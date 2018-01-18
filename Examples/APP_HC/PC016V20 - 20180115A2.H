#ifndef __PC016V20_H
#define __PC016V20_H


#include "stm32f10x_exti.h"

#include "STM32_TOOLS.H"		//异或校验工具
#include "SWITCHID.H"
#include "STM32_CRC.H"
#include "STM32_USART.H"
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

#define	MinPortNum		1			//最小/起始端口号 1~4
#define	MaxPortNum		4			//最大端口号 1~4
#define	MaxSysTime		1000	//TimeSYS最大计时时间，单位ms，到达最大计时时间后，所有计时器清零

//==============端口---读卡器
#define	ICCardReadPort1					USART3		//J1
#define	ICCardReadPort2					USART1		//J2
#define	ICCardReadPort3					UART4			//J3
#define	ICCardReadPort4					UART5			//J4---不支持DMA传输
#define	ICCardReadBaudRate			19200
#define	CardBufferSize					64				//读卡器缓存大小
#define	CardDataSize						32				//接收到IC读卡器完整数据大小（协议+数据共22byte）备用10byte
//#define	ICCardDataSize					16				//IC卡数据大小（有效卡号字节长度）
#define	ICDataStartByte					2					//16字节数据中有效卡号起始位 0~15
#define	ICDataNum								3					//16字节数据中有效卡号字节数

#define	CardStartByte						0					//16字节数据中有效卡号起始位 0~15
#define	CardLength							4					//16字节数据中有效卡号字节数

//==============复用端口

//==============端口---总线485
#define	Bus485Port				USART2
#define	Bus485CtlPort			GPIOA
#define	Bus485CtlPin			GPIO_Pin_1
#define	Bus485BaudRate		19200											//波特率
#define	BusDataSize				256												//一条消息携带的最大数据个数
#define	Bus485BufferSize	300												//一条消息的最大缓存大小（协议数据+数据）
#define	CommProtocolLenth		11											//标准协议部分数据长度//sizeof(sCommHeadDef)+sizeof(sCommStartDef)+sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)
#define	CommBasicLenth			4												//数据长度nLength中地址和状态码的长度
#define	FixedAddrLenth			4												//固定数据长度nLength中地址和状态码的长度
#define	FixedDataLenth			3												//固定数据长度-----卡号数据

//==============端口---数码管485
#define	Seg485PortRmap		USART1		//端口重定义
#define	Seg485CtlPort			GPIOB
#define	Seg485CtlPin			GPIO_Pin_5
#define	Seg485BaudRate		19200
#define	Seg485BufferSize	22
//==============端口---锁
#define	LockPort(n)		LockPort##n
#define	LockPin(n)		LockPin##n
#define	LockOn(n)			GPIO_SetBits(LockPort(n),	LockPin(n))			//开锁
#define	LockOff(n)		GPIO_ResetBits(LockPort(n),	LockPin(n))		//释放锁

#define	LockPort1			GPIOC
#define	LockPin1			GPIO_Pin_3
#define	LockPort2			GPIOB
#define	LockPin2			GPIO_Pin_9
#define	LockPort3			GPIOB
#define	LockPin3			GPIO_Pin_3
#define	LockPort4			GPIOB
#define	LockPin4			GPIO_Pin_4


#define	DefaultOnTime	30000				//默认(最小)锁吸合时间30S
#define	MaxOnTime			120000			//锁吸合最大时间120S	2分钟
#define	LockOffTime		5000				//药箱取走后锁释放时间3S
#define	SegOnTime			60000				//插入药箱后数码管常亮时间60S


#define	Pt(n)	GPIO_SetBits(LockPort(n),	LockPin(n))

//u8 ICCARD_CMD_GetDataBlock1[] =
//{
//	0xE1,   /* F-Head 	*/			//为帧头，上位机下发时固定为0XE1，下位机应答时固定为0XD2；
//	0x24,  	/* CmdType 	*/			//命令类型
//	0x01,		/* Sector 	*/			//1字节扇区号
//	0x01,   /* Block 		*/			//1字节块号
//	0x60,   /* KeyType	*/			//1字节密钥模式 KEYA（0x60）/KEYB(0x61)
//	0xFF,   /* PassWord0 */			//6字节密码
//	0xFF,   /* PassWord1 */
//	0xFF,   /* PassWord2 */
//	0xFF,		/* PassWord3 */
//	0xFF,   /* PassWord4 */
//	0xFF,		/* PassWord5 */
//	0x1C,   /* Crc16-HIGN */
//	0x0D,		/* Crc16-LOW */
//	0x1E   	/* F-End*/						//为帧尾，上位机下发时固定为0X1E，下位机应答时固定为0X2D；
//};
typedef enum
{
	eGetID	=	0x92,		//上位机主动获取ID请求应答命令
	eSendID	=	0x93,		//下位机主动上传ID号命令
	eLock		=	0x93,		//锁控制命令上报
	eGetBox	=	0x93,		//取药命令上报
}BusCmdDef;
typedef enum
{
	eNoErr		=	0x00,		//无错误
	eNoData		=	0x23,		//无药箱数据
	eBoxTout	=	0x24,		//取药箱超时
	eBoxOff		=	0x25,		//药箱取走
	eBoxBack	=	0x26,		//药箱收回
	eReadErr	=	0x27,		//读卡器通讯异常
	eBoxTake	=	0x28,		//药箱被强制取走
}StatusCodeDef;
typedef enum
{
	eHead			=	0x7E,		//头标识
	eEnd			=	0x7F,		//结束标识
}BusFarmeDef;
//=========================读卡器
typedef	struct	_ICCRC16
{
	u8 CRC_H;
	u8 CRC_L;
}ICCRC16Def;

typedef	struct	_ICBuffer			//读卡器数据格式
{
	u8 Head;
	u8 CmdType;
	u8 Sector;
//	u8 Block;
	u8 data[16];
	ICCRC16Def CRC16;
	u8 End;
}ICBufferDef;

typedef	struct	_ICReadCmd		//读卡命令格式
{
	u8	Head;						//0xE1,   /* F-Head 	*/			//为帧头，上位机下发时固定为0XE1，下位机应答时固定为0XD2；
	u8	CmdType;				//0x24,  	/* CmdType 	*/			//命令类型
	u8	Sector;					//0x01,		/* Sector 	*/			//1字节扇区号
	u8	Block;					//0x01,   /* Block 		*/			//1字节块号
	u8	KeyType;				//0x60,   /* KeyType	*/			//1字节密钥模式 KEYA（0x60）/KEYB(0x61)
	u8	PassWord[6];		//6字节密码0xFF
	ICCRC16Def	CRC16;
	u8 End;
}ReadCmdDef;

//================485总线通讯结构体（病区柜）
typedef	struct	_sFarme
{
	u8	Head;			//b0识别符	0x7E
	u8	dAdd;			//b1目标地址（暂时理解为单元柜地址）下发为接收地址，上传为0x00
	u8	sAdd;			//b2源地址 下发为0x00，上传为本地址
	u8	SN;				//b3流水号 0x01~0xFF 每次使用后加1
	u8	Cmd;			//b4命令号：0x12-获取指定IC卡号，0x13取药
	u8	Ucode;		//b5用户码：不处理，原样返回
	u8	Length;		//b6地址+异常码固定长度为4，卡数据长度为3
	
	u8 	Addr1;		//b7柜地址(单元柜号)
	u8 	Addr2;		//b8层地址
	u8	Addr3;		//b9槽地址（端口号）	
	
	u8	EC;				//错误码
	
	u8	data[BusDataSize];		//DataBuffer	
	
	u8	Bcc8;			//异或校验	//计算范围为起始段到数据段(dAdd~data[n])到错误码
	u8	End;			//结束符
}sFarmeDef;
//typedef	struct	_sBusData
//{
//	u8	dAdd;			//目标地址（暂时理解为单元柜地址）上级请求下级的地址
//	u8	Ucode;		//用户码：不处理，原样返回
//	u8 	Addr1;		//柜地址(单元柜号)
//	u8 	Addr2;		//层地址
//	u8	Addr3;		//槽地址（端口号）	
//}sBusDataDef;
//=======================================================数码管通讯结构体：插入药箱常亮30S；取药箱请求：一定时间内闪烁；无药箱：小数点一直闪烁；读卡通讯故障：小数点常亮
typedef struct _SegCmd
{
	unsigned char DispEnNum	:1;		//bit0显示数值	：	0-不显示，		1-显示
	unsigned char DispEnDp	:1;		//bit1显示点		：	0-不显示，		1-显示
	unsigned char DispMdNum	:1;		//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
	unsigned char DispMdDp	:1;		//bit3点模式		：	0-静态显示，	1-0.5S闪烁
	unsigned char DispTime	:1;		//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
} SegCmdDef;
typedef struct _sSegFarme 	//数码管通讯数据格式
{
	unsigned char desAddr;				//目的地址---数码管拔码地址
	unsigned char srcAddr;				/*源地址*/
	unsigned char runningNumber;	/*流水号*/
	SegCmdDef			cmd;						/*命令*/
	unsigned char len;						/*长度*/
	unsigned char data[8];				/*数据,高位在前,data[0~3]为显示内容,data[4~7]为闪烁时间*/ 
} sSegFarmeDef;			//数码管数据结构体
//=======================================================读卡器构体
typedef	struct	_CardReader		//读卡器端口及缓存
{
	struct
	{
		unsigned char ReadFlg		:1;	//bit0：0-无操作,			1-有操作请求(需要主动读卡）
		unsigned char ReadData	:1;	//bit1：0-无操作,			1-主动读卡
		unsigned char ReadID		:1;	//bit2：0-无操作,			1-读到ID(需要增加相关数据及标志)		
		unsigned char ReadErr		:1;	//bit3：0-读卡器正常，	1-读卡器通讯异常
		unsigned char ReadNone	:1;	//bit4：0-无操作,			1-卡号未读到(需要删除相关数据及标志)
		unsigned char NoCard		:1;	//bit5：0-无操作,			1-无卡（无药箱）
	}ReaderSts;										//读卡器状态
	unsigned char Rev[CardBufferSize];		//读卡器数据接收---备份（卡数据+协议大小为22）
	unsigned char Rxd[CardBufferSize];		//读卡器数据接收---接收缓冲区（DMA）（卡数据+协议大小为22）
	USART_TypeDef*	UartPort;
}CardReaderDef;
//=======================================================锁结构体
typedef	struct	_LockFlag		//锁控制标志及吸合时间
{
	struct
	{
		unsigned char LockFlg		:1;	//bit0：0-无操作,1-有操作请求(控制锁的状态）
		unsigned char LockOn		:1;	//bit1：0-释放锁，1-开锁
		unsigned char LockSts		:1;	//bit2：0-已释放，1-已开锁
		unsigned char LockTout	:1;	//bit3：0-未超时，1-已超时(取药超时)
	}sLockSts;		//药箱状态
//	unsigned long	On					:1;		//0-不处理，1-开锁
//	unsigned long	Status			:1;		//0-锁未开，1-已开
	unsigned long	LockOnTime	;	//锁吸合时间，最大值0x3FFF FFFF
	GPIO_TypeDef*	GPIOx;
	u16	GPIO_Pin_n;
}LockFlagDef;
//=======================================================时间结构体
typedef	struct	_Time			//相关时间表
{
	unsigned long	TimeSYS;	//系统计时器
	unsigned long	TimeSEG;	//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	unsigned long	TimeBOX;	//药箱检测计时器	每个端口分配100mS时间检测----可用于检测读卡器通讯状态
	unsigned long	TimeBUS;	//外部总线时间
	unsigned long	TimeCard;	//读卡器时间
}TimeDef;

//=======================================================数码管结构体
typedef struct _sSeg 
{
	struct
	{
		unsigned char SegFlg			:1;	//bit0：0-无操作,		1-有操作请求(需要更新数码管状态）
		unsigned char SegTimeOut	:1;	//bit1：0-无操作,		1-取药超时，需要关闭数码管
	}sSegSts;			//数码管标志
	sSegFarmeDef 	SegFarme;					//数码管通讯协议
} sSegDef;			//数码管数据结构体
//=======================================================药箱结构体
typedef	struct	_sBox		//端口状态--：有卡/无卡
{
	struct
	{
		unsigned char BoxFlg		:1;	//bit0：0-无操作,			1-操作
		unsigned char BoxOn			:1;	//bit1：0-无药箱,			1-有药箱
		unsigned char ReadID		:1;	//bit2：0-无操作,			1-读到ID(需要增加相关数据及标志)
		unsigned char NoID			:1;	//bit3：0-无操作,			1-卡号未读到(需要删除相关数据及标志)
		unsigned char ReadErr		:1;	//bit4：0-读卡器正常，	1-读卡器通讯异常
	}BoxSts;		//药箱状态
	unsigned char 	CardData[CardLength];	//卡号数据CardData[0]为百位，CardData[1]为十位，CardData[2]为个位
	unsigned long		CardNumber;						//十进制卡号
}sBoxDef;
//=======================================================总线请求结构体
typedef	struct	_sBus		//端口状态--：有卡/无卡
{
	struct
	{
		unsigned char BusFlg		:1;	//bit0：0-无操作,	1-有待处理事件(上报数据或者状态）
		unsigned char GetID			:1;	//bit1：0-无请求，	1-有请求	，获取ID请求需要上报ID
		unsigned char GetBox		:1;	//bit2：0-无请求，	1-有请求	，取药(数码管数值同时闪烁)
		unsigned char GotBox		:1;	//bit3：0-无操作，	1-药箱已被取走
		unsigned char TakeBox		:1;	//bit4：0-无操作，	1-药箱被强制取
		unsigned char BoxBack		:1;	//bit5：0-无操作，1-药箱收回,收回后需要上报药箱号（有GotBox标志时设置此位）
		unsigned char TimeOut		:1;	//bit6：0-无操作，	1-取药超时
		unsigned char SendID		:1;	//bit7：0-无操作，	1-检测到药箱插入，主动上报ID，如果是取药，取消主动上报，使用状态上报
		
	}BusqSts;		//药箱状态
	unsigned char 	CardData[CardLength];	//卡号数据CardData[0]为百位，CardData[1]为十位，CardData[2]为个位
	unsigned char		Ucode;								//用户码
	sFarmeDef		sBusFarme;		//485总线结构体
}sBusDef;


//=======================================================端口/槽位所有信息结构体
typedef	struct	_Port
{
	sSegDef					Seg;				//数码管数据结构体
	LockFlagDef			Lock;				//锁信息
	sBoxDef					sBox;				//药箱相关信息
	sBusDef					sBus;				//总线请求结构体
	CardReaderDef		sReader;		//读卡器结构体

	unsigned char		PortNum;							//槽地址（端口号）
} PortDef;
//=======================================================运行步骤
typedef	struct	_Step
{
	unsigned char 	ReadCard		:1;			//bit0:	0-未执行读卡,需要读卡，		1-已读完卡
	unsigned char 	WriteSeg		:1;			//bit1:	0-未更新数码管,需要更新，	1-已更新(读卡过程中不更新数码管，免得占用串口）
	unsigned char 	BoxStatus		:1;			//bit3:	0-未更新药箱状态，需要更新，	1-已更新
} StepDef;
//=======================================================公共数据结构体
typedef struct _sPlu 
{
	StepDef	Step;		//执行分步（主要防止读卡器与数码管复用串口数据错乱）
	TimeDef	Time;		//各功能计时器
} sPluDef;			//数码管数据结构体
//=======================================================板内端口/板内数据
typedef	struct	_sBorad		//端口状态--：有卡/无卡
{
	PortDef	Port1;
	PortDef	Port2;
	PortDef	Port3;
	PortDef	Port4;
	
	sPluDef	sPlu;		//公共数据结构体

	u8	Nserial;		//流水号 1~0xFF
}sBoradDef;


//=======================================================全局变量
extern sBoradDef	sBorad;					//端口/槽位所有信息结构体


extern SWITCHID_CONF	SWITCHID;			//拔码开关

//==============读卡器命令
extern const u8 CardReaderCmd_SetArea[14];		//设置读卡器读卡区域命令
extern const u8 CardReaderCmd_ReadData[14];		//读卡命令


//==============485结体体
extern RS485_TypeDef	RS485_Bus;	//总线通讯485
extern RS485_TypeDef	RS485_Seg7;	//数码管显示485

//==============485缓冲区
extern sFarmeDef	sFarme;				//总线485数据结构体---待处理
extern sFarmeDef	sFarmeTxd;		//总线485数据结构体---发送
extern sFarmeDef	sFarmeRxd;		//总线485数据结构体---接收
extern sFarmeDef	sFarmeRev;		//总线485数据结构体---缓存
extern u8 RS485BusTxd[Bus485BufferSize];


//extern u8 Seg485Txd[Seg485BufferSize];
extern u8 Seg485Rxd[Seg485BufferSize];
//extern u8 Seg485Rev[Seg485BufferSize];

//==============读卡器缓冲区
extern ReadCmdDef	ReadCmd;			//读卡命令
extern u8	CardData[CardLength];		//卡有效数据缓存						
extern u8 ICCardReadRxd[CardBufferSize];
extern u8 ICCardReadRev[CardBufferSize];

//extern ICBufferDef ICCardReadRxd1;
//extern ICBufferDef ICCardReadRev1;

extern ICBufferDef ICCardReadRxd2;
extern ICBufferDef ICCardReadRev2;

extern ICBufferDef ICCardReadRxd3;
extern ICBufferDef ICCardReadRev3;



extern u8 ICCardReadCount1;
extern u8 ICCardReadCount2;
extern u8 ICCardReadCount3;
extern u8 ICCardReadCount4;

//==============其它变量

extern u8 SwitchData;						//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
//extern u32	SYSTIME;						//系统计时器
//extern TimeDef	SYSTime;				//系统运行相关计时器
extern u32	Port4TimeOut;				//端口4连续读数超时


//=================================================================================================================主程序
void PC016V20_Configuration(void);
void PC016V20_Server(void);


//=================================================================================================================RS485外部总线处理
//void MessageProcess(void);							//消息处理:外部RS485总线数据接收与发送管理
void MessageProcessRecevie(void);				//消息接收处理
void MessageProcessSend(void);					//消息发送处理

//=================================================================================================================协议程序
void RS485_Server(void);							//RS485收发处理

void BusFrameAck(sFarmeDef*	sFarme);			//外部总线应答
void MessageProcess(sFarmeDef*	sFarme,sBoradDef*	sBorad);				//外部总线消息处理

void MessageAnalysis(sFarmeDef*	sFarme);		//解析消息
void MessagePacket(sFarmeDef*	sFarme);			//打包消息

void BusFrameProcessGetID(sFarmeDef*	sFarme,sBoradDef*	sBorad);		//数据命令处理
void BusFrameProcessGetBox(sFarmeDef*	sFarme,sBoradDef*	sBorad);		//数据命令处理---取药

void BusFrameProcessSendBox(sFarmeDef*	sFarme,sBoradDef*	sBorad);	//发送药箱数据

void BusFrameProcessPacketAndSend(sFarmeDef*	sFarme);					//按照协议打包数据并发送
void BusFrameProcessLock(sFarmeDef*	sFarme,sBoradDef*	sBorad);	//锁命令处理

//=================================================================================================================药箱
void BoardBoxProcess(sBoradDef*	sBorad);				//药箱处理程序
void BoardGetBoxProcess(sBoradDef*	sBorad);		//取药时的时间管理,包括超时

//=================================================================================================================读卡器程序

void SendReadCardCmd(USART_TypeDef* USARTx);				//发送读卡命令使读卡器发回数据
	

u8 	 BoardCardReaderServer(void);					//读取IC卡数据
u8 	CardReaderPortn(void);									//端口4读卡---端口4用UART5，无法使用DMA送数据，只能用中断接收
u8 	CardReaderPort4(void);									//端口4读卡---端口4用UART5，无法使用DMA送数据，只能用中断接收
void CardReadTimeCount(void);			//取卡后倒计时，如果倒计时未完成再收回卡数据，读卡不稳定时的误动作
u8 	 BoardCardDataAnalysis(ICBufferDef* CardData);		//读取IC卡数据校验，返回0失败，作废，返回1正确
void BoardCardReaderReadAll(sBoradDef*	sBorad);			//发送4个端口读卡指令
void BoardSaveCardData(sBoradDef*	sBorad);		//根据状态保存卡号及设置相关状态
void BoardClrCardData(sBoradDef*	sBorad);		//删除卡号数据及设置相关状态

//=================================================================================================================数码管控制程序
void BoardSetSeg(sBoradDef*	sBorad);			//根据标志设置数码管状态（无箱，有箱，取箱，读卡通讯不良）
//=================================================================================================================锁程序
//void Lock_Server(sBoradDef*	sBorad);
//=================================================================================================================硬件驱动程序
void HALUsartRemapEnable(void);				//使能串口复用
void HALUsartRemapDisable(void);			//关闭串口复用
void HALSendSeg(u32* Buffer,u16 Length);					//向数码管发送数据

//=================================================================================================================配置程序
void Lock_Configuration(void);				//锁初始化
void Switch_Configuration(void);			//拔码开关初始化及读数
void RS485_Configuration(void);				//RS485配置
void CardReader_Configuration(void);	//读卡器配置
void Data_Initialize(void);						//参数初始化
void ResetData(void);									//重置相关变量
#endif



