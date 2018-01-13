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
#define	ICCardReadBufferSize		32				//接收到IC读卡器完整数据大小（协议+数据）
//#define	ICCardDataSize					16				//IC卡数据大小（有效卡号字节长度）
#define	ICDataStartByte					2					//16字节数据中有效卡号起始位 0~15
#define	ICDataNum								3					//16字节数据中有效卡号字节数

#define	CardStartByte						0					//16字节数据中有效卡号起始位 0~15
#define	CardLength							4					//16字节数据中有效卡号字节数

//==============端口---总线485
#define	Bus485Port				USART2
#define	Bus485CtlPort			GPIOA
#define	Bus485CtlPin			GPIO_Pin_1
#define	Bus485BaudRate		19200											//波特率
#define	BusDataSize				256												//一条消息携带的最大数据个数
#define	Bus485BufferSize	sizeof(sCommunicationDef)	//一条消息的最大缓存大小（协议数据+数据）
#define	CommProtocolLenth		11											//标准协议部分数据长度//sizeof(sCommHeadDef)+sizeof(sCommStartDef)+sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)
#define	CommBasicLenth			4													//数据长度nLength中地址和状态码的长度

//==============端口---数码管485
#define	Seg485PortRmap		USART1		//端口重定义
#define	Seg485CtlPort			GPIOB
#define	Seg485CtlPin			GPIO_Pin_5
#define	Seg485BaudRate		19200
#define	Seg485BufferSize	16
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


#define	DefaultOnTime	30000				//默认锁吸合时间30S
#define	MaxOnTime			120000			//锁吸合最大时间120S	2分钟

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


//=========================盒子
typedef	enum	_ePortNum		//端口编号
{
	ePort1	=	1,
	ePort2	=	2,
	ePort3	=	3,
	ePort4	=	4
}ePortNumDef;
//=========================通讯
typedef	enum	_BoxStatus
{
	BoxOn		=	0x01,			//无盒子或者取出
	BoxOff	=	0x02			//有盒子
}BoxStatusDef;
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


//=========================通讯
typedef	enum	_eCommHead
{
	eCommHead1	=	0xFA,
	eCommHead2	=	0xF5
}eCommHeadDef;
#define IS_eCommHead(Head) (	Head == eCommHead1 ||	Head == eCommHead2	)
typedef	enum	_eCommErrorCode
{
	eCommErr00	=	0x00,		//无错误
	eCommErr01	=	0x01,		//忙状态
	eCommErr02	=	0x02,		//数据非法
	eCommErr03	=	0x03,		//
	eCommErr04	=	0x04,		//
	eCommErr05	=	0x05,		//
	eCommErr06	=	0x06,		//
	eCommErr07	=	0x07,		//
	eCommErr08	=	0x08,		//
	eCommErr09	=	0x09,		//
	eCommErr10	=	0x0A,		//
	eCommErr11	=	0x0B,		//
	eCommErr12	=	0x0C,		//
	eCommErr13	=	0x0D,		//
	eCommErr14	=	0x0E,		//
	eCommErr15	=	0x0F,		//
	eCommErr16	=	0x11,		//抽屉开状态
	eCommErr17	=	0x12,		//抽屉关状态
	eCommErr18	=	0x13,		//称重数据不稳定
	eCommErr19	=	0x14,		//
	eCommErr20	=	0x15,		//
	eCommErr21	=	0x16,		//
	eCommErr22	=	0x17,		//
	eCommErr23	=	0x18,		//
	eCommErr24	=	0x19,		//
	eCommErr25	=	0x1A,		//
	eCommErr26	=	0x1B,		//
	eCommErr27	=	0x1C,		//
	eCommErr28	=	0x1D,		//
	eCommErr29	=	0x1E,		//
	eCommErr30	=	0x1F,		//
	eCommErr31	=	0x1F,		//
	eCommErr32	=	0x20,		//
	eCommErr33	=	0x21,		//单位重量错误
	eCommErr34	=	0x22,		//余量不足
	eCommErr35	=	0x23,		//无药箱数据/无卡
	eCommErr36	=	0x24,		//取药箱超时
	eCommErr37	=	0x25,		//药箱取走
	eCommErr38	=	0x26,		//药箱收回
	eCommErr39	=	0x27		//读卡器通讯异常
}eCommErrorCodeDef;
//================命令类型
typedef	enum	_eCommCmd
{
	eCommCmd00	=	0x00,		//未定义----
	eCommCmd01	=	0x01,		//写入数量
	eCommCmd02	=	0x02,		//强制弹出
	eCommCmd03	=	0x03,		//未定义----
	eCommCmd04	=	0x04,		//加药
	eCommCmd05	=	0x05,		//未定义----
	eCommCmd06	=	0x06,		//取药
	eCommCmd07	=	0x07,		//盘点
	eCommCmd08	=	0x08,		//写入提示
	eCommCmd09	=	0x09,		//写入注释
	eCommCmd10	=	0x0A,		//
	eCommCmd11	=	0x0B,		//
	eCommCmd12	=	0x0C,		//
	eCommCmd13	=	0x0D,		//
	eCommCmd14	=	0x0E,		//
	eCommCmd15	=	0x0F,		//
	eCommCmd16	=	0x11,		//
	eCommCmd17	=	0x12,		//获取指定IC卡号
	eCommCmd18	=	0x13		//开锁取药箱
}eCommCmdDef;
#define IS_eCommCmd(Cmd) (	Cmd == eCommCmd17 ||	Cmd == eCommCmd18	)
//================应答类型
typedef	enum	_eCommAck
{
	eCommAck00	=	0x00,		//
	eCommAck01	=	0x01,		//
	eCommAck02	=	0x02,		//
	eCommAck03	=	0x03,		//
	eCommAck04	=	0x04,		//
	eCommAck05	=	0x05,		//
	eCommAck06	=	0x06,		//
	eCommAck07	=	0x07,		//
	eCommAck08	=	0x08,		//
	eCommAck09	=	0x09,		//
	eCommAck10	=	0x0A,		//
	eCommAck11	=	0x0B,		//
	eCommAck12	=	0x0C,		//
	eCommAck13	=	0x0D,		//
	eCommAck129	=	0x81,		//写入数量
	eCommAck130	=	0x82,		//强制弹出
	eCommAck131	=	0x83,		//
	eCommAck132	=	0x84,		//加药
	eCommAck133	=	0x85,		//
	eCommAck134	=	0x86,		//取药
	eCommAck135	=	0x87,		//盘点
	eCommAck136	=	0x88,		//写入提示
	eCommAck137	=	0x89,		//写入注释
	eCommAck138	=	0x8A,		//
	eCommAck139	=	0x8B,		//
	eCommAck145	=	0x91,		//
	eCommAck146	=	0x92,		//获取指定IC卡号应答
	eCommAck147	=	0x93		//开锁取药箱
}eCommAckDef;
//================运行状态
typedef	enum	_eRunStep
{
	eIDLE,				//空闲/释放
	eCheckBox,		//检查药箱
	eUpdataSeg,		//更新数码管
	eGetBox,			//取药箱
	eLockSet
}eRunStepDef;		//运行状态
//================识别符结构体0xFA 0xF5
typedef	struct	_sCommHead
{
	eCommHeadDef	eHead1;
	eCommHeadDef	eHead2;
}sCommHeadDef;
//================起始段数据结构体
typedef	struct	_sCommStart
{	
	u8 						nSerial;		//流水号0x01~0xFF
	eCommCmdDef 	eCmd;				//命令号：
	u8						nBcc8;			//8位数异或校验码：地址段+异常码+数据段 校验
	u8						nUserCode;	//用户码0x01~0xFF,上位机使用标识，不做处理，原数返回
	u8						nLength;		//地址段+异常码+数据段 的总字节数
}sCommStartDef;
//================地址段结构体
typedef	struct	_sCommAddr
{	
	u8 	Addr1;		//柜地址(单元柜号)
	u8 	Addr2;		//层地址
	u8	Addr3;		//槽地址（端口号）
}sCommAddrDef;
//================总线通讯结构体（通讯协议）
typedef	struct	_sCommunication
{
	sCommHeadDef			sHead;								//识别符	0xFA 0xF5
	sCommStartDef			sStart;								//起始段
	sCommAddrDef			sAddr;								//地址段
	eCommErrorCodeDef eErrorCode;						//异常码(状态码）
	u8 								data[BusDataSize];		//DataBuffer
}sCommunicationDef;


//================485总线通讯结构体（病区柜）
typedef	struct	_sFarme
{
	u8	Head;								//识别符	0x7E
	u8	dAdd;								//目标地址（暂时理解为单元柜地址）
	u8	sAdd;								//源地址 下发为0x00
	u8	SN;				//流水号
	u8	Cmd;	//命令号：0x12-获取指定IC卡号，0x13取药
	u8	Ucode;		//用户码：不处理，原样返回
	u8	Length;		//数据长度
	
	u8 	Addr1;		//柜地址(单元柜号)
	u8 	Addr2;		//层地址
	u8	Addr3;		//槽地址（端口号）	
	
	u8	data[BusDataSize];		//DataBuffer
	
	u8	EC;				//错误码
	u8	Bcc8;			//异或校验	//计算范围为起始段到数据段(dAdd~data[n])
	u8	End;			//结束符
}sFarmeDef;


////=======================更新数码管结构体
typedef struct _SegFlg
{
	unsigned char Seg1	:2;		//显示数值	：	0-不处理，		1-显示，2-显示点，3-点闪烁
	unsigned char Seg2	:2;		//显示点		：	0-不显示，		1-显示
	unsigned char Seg3	:2;		//数值模式	：	0-静态显示，	1-0.5S闪烁
	unsigned char Seg4	:2;		//点模式		：	0-静态显示，	1-0.5S闪烁
} SegFlgDef;

//=======================命令结构体
typedef struct _SegCmd
{
	unsigned char DispEnNum	:1;		//bit0显示数值	：	0-不显示，		1-显示
	unsigned char DispEnDp	:1;		//bit1显示点		：	0-不显示，		1-显示
	unsigned char DispMdNum	:1;		//bit2数值模式	：	0-静态显示，	1-0.5S闪烁
	unsigned char DispMdDp	:1;		//bit3点模式		：	0-静态显示，	1-0.5S闪烁
	unsigned char DispTime	:1;		//bit4显示时间	：	0-长亮，			1-在显示时间内根据显示模式显示
} SegCmdDef;

typedef struct _sSeg 
{
	unsigned char desAddr;				/*目的地址*/
	unsigned char srcAddr;				/*源地址*/
	unsigned char runningNumber;	/*流水号*/
	SegCmdDef			cmd;						/*命令*/
	unsigned char len;						/*长度*/
	unsigned char data[8];				/*数据,高位在前,data[0~3]为显示内容,data[4~7]为闪烁时间,0表示不闪烁*/ //显示时间为0xFFFF FFFF表示为常亮
} sSegDef;			//数码管数据结构体


typedef	struct	_LockFlag		//锁控制标志及吸合时间
{
	unsigned long	On					:1;		//0-不处理，1-开锁
	unsigned long	Off					:1;		//0-不处理，1-释放锁
	unsigned long	LockOnTime	:30;	//锁吸合时间，最大值0x3FFF FFFF
}LockFlagDef;
typedef	struct	_Time		//锁控制标志0--不处理，1--开锁操作
{
	unsigned long	TimeSYS;	//系统计时器
	unsigned long	TimeSEG;	//数码管更新数据计时器----可用于检测数码管通讯状态 20ms外发一个数码管状态
	unsigned long	TimeBOX;	//药箱检测计时器	每个端口分配100mS时间检测----可用于检测读卡器通讯状态
	unsigned long	TimeBUS;	//外部总线时间
}TimeDef;
typedef	struct	_sBoxStatus		//端口状态--：有卡/无卡
{
	unsigned short	BoxFlg	:1;	//bit0：0-无卡,1-有卡
	unsigned short	BoxPro	:1;	//bit1：0-药箱无操作记录，1-药箱有操作记录（取药）;之前操作为取药，重新获取到卡表示药箱已收回
	unsigned short	BoxSta	:1;	//bit2：0-无操作，1-药箱收回,收回后需要上报药箱号
	unsigned short	BoxRead	:1;	//bit3：0-读卡器正常，1-读卡器通讯异常
	unsigned short	BoxSeg	:1;	//bit4：0-无操作，1-需要未将卡号发给数码管
	unsigned short	BoxCmd	:1;	//bit5：0-无操作，1-获取IC卡号操作
	unsigned short	BoxDel	:1;	//bit6：0-无待处理操作，	1-移除药箱请求(数码管数值闪烁)
	unsigned short	RecData	:1;	//bit7：0-无接收到卡数据，1-有接收到卡数据
	unsigned short	ClrData	:1;	//bit8：0-无操作，1-请求清除药箱在线状态
}sBoxStatusDef;

typedef	struct	_PortFlag	
{
	unsigned char	Port1Flg;	//bit0:0无卡,1有卡；bit1：0药箱无操作记录，1药箱有操作记录；bit1：0药箱取走，1药箱收回；bit3：1-有卡,2-药箱取走，3-读卡器通讯异常
	unsigned char	Port2Flg;
	unsigned char	Port3Flg;
	unsigned char	Port4Flg;
}PortFlagDef;

typedef	struct	_ICData	//IC卡数据
{
	u8	Port1Data[ICDataNum];
	u8	Port2Data[ICDataNum];
	u8	Port3Data[ICDataNum];
	u8	Port4Data[ICDataNum];
}ICDataDef;


//=========端口/槽位所有信息结构体
typedef	struct	_Port
{
	sBoxStatusDef		Status;								//箱子状态
	sSegDef					Seg;									//数码管数据结构体
	LockFlagDef			Lock;									//锁信息
	unsigned char 	CardData[CardLength];	//卡号数据CardData[0]为百位，CardData[1]为十位，CardData[2]为个位
	unsigned long		CardNumber;						//十进制
	unsigned short	TimeCount;						//计时器	
	unsigned char		Ucode;								//用户码
	unsigned char		Cmd;									//上位机下发的命令
} PortDef;
	
//==========板内端口
typedef	struct	_sBorad		//端口状态--：有卡/无卡
{
	PortDef	Port1;
	PortDef	Port2;
	PortDef	Port3;
	PortDef	Port4;
}sBoradDef;




extern sSegDef	sSeg;			//数码管数据结构体
extern SegFlgDef	SegFlg;	//更新数码管结构体	
//=========================运行
extern SWITCHID_CONF	SWITCHID;			//拔码开关
//extern LockFlagDef	LockFlag;				//锁控制标志0--不处理，1--开锁操作，当层需要检测药箱状态并在状态更新后上报结果

//==============485结体体
extern RS485_TypeDef	RS485_Bus;	//总线通讯485
extern RS485_TypeDef	RS485_Seg7;	//数码管显示485

//==============485缓冲区
extern sFarmeDef	sFarme;				//总线485数据结构体---待处理
extern sFarmeDef	sFarmeTxd;		//总线485数据结构体---发送
extern sFarmeDef	sFarmeRxd;		//总线485数据结构体---接收
extern sFarmeDef	sFarmeRev;		//总线485数据结构体---缓存
extern u8 RS485BusTxd[Bus485BufferSize];

extern sCommunicationDef	sCommData;				//总线485数据结构体---待处理
extern sCommunicationDef	sCommDataTxd;			//总线485数据结构体---发送
extern sCommunicationDef	sCommDataRxd;			//总线485数据结构体---接收
extern sCommunicationDef	sCommDataRev;			//总线485数据结构体---缓存

//extern u8 RS485BusTxd[Bus485BufferSize];
//extern u8 RS485BusRxd[Bus485BufferSize];
//extern u8 RS485BusRev[Bus485BufferSize];

extern u8 Seg485Txd[Seg485BufferSize];
extern u8 Seg485Rxd[Seg485BufferSize];
extern u8 Seg485Rev[Seg485BufferSize];

//==============读卡器缓冲区
//extern sGetIDflagDef	sGetID;					//获取ID请求标志
//extern PortFlagDef	PortFlag;	//端口状态--：有卡/无卡
extern sBoradDef	sBorad;					//端口/槽位所有信息结构体
//extern sBoxStatusDef	sBoxStatus;			//各药箱状态
extern sBoxStatusDef	sBoxStatus1;	//药箱1状态
extern sBoxStatusDef	sBoxStatus2;	//药箱2状态
extern sBoxStatusDef	sBoxStatus3;	//药箱3状态
extern sBoxStatusDef	sBoxStatus4;	//药箱4状态
extern ReadCmdDef	ReadCmd;			//读卡命令
extern u8	CardData[CardLength];		//卡有效数据缓存						
extern ICDataDef	ICData;				//4个端口IC数据
extern u8 ICCardReadRxd[ICCardReadBufferSize];
extern u8 ICCardReadRev[ICCardReadBufferSize];

extern ICBufferDef ICCardReadRxd1;
extern ICBufferDef ICCardReadRev1;

extern ICBufferDef ICCardReadRxd2;
extern ICBufferDef ICCardReadRev2;

extern ICBufferDef ICCardReadRxd3;
extern ICBufferDef ICCardReadRev3;

extern ICBufferDef ICCardReadRxd4;
extern ICBufferDef ICCardReadRev4;

extern u8 ICCardReadCount1;
extern u8 ICCardReadCount2;
extern u8 ICCardReadCount3;
extern u8 ICCardReadCount4;

//==============其它变量
extern eRunStepDef	eRunStep;		//运行状态
extern u8 SwitchData;						//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
extern u32	SYSTIME;						//系统计时器
extern TimeDef	SYSTime;				//系统运行相关计时器
extern u32	Port4TimeOut;				//端口4连续读数超时
//extern u32	LockOnTime[4];		//锁吸合时间，如果时间为0则释放锁





//========================主程序
void PC016V20_Configuration(void);
void PC016V20_Server(void);

//========================服务程序

void RS485_Server(void);							//RS485收发处理
u8 ICCardReader_Server(void);					//读取IC卡数据
void Lock_Server(sBoradDef*	sBorad);




//========================配置程序
void RS485_Configuration(void);					//RS485配置
void ICCardReader_Configuration(void);	//读卡器配置
void SWITCHID_Configuration(void);			//拔码开关初始化及读数
void Lock_Configuration(void);			//锁初始化


//========================驱动程序
void ICCardReader_Read(u8 Num,u32* Cmd);				//读相应端口的卡
void CardReader_Set(void);							//设置读头读卡扇区、块号、KEYA/KEYB、读卡密码0x44
void ICCardReader_ReadAll(void);			//发送4个端口读卡指令
void ICCardReader_CheckCard(void);
//void ICCardDataPacket(void);		//数据打包

void SegSendData(u8	Port,u32 Data,u32 Time);			//向数码管发显示数据


void LockOnTimeSet(void);			//设置锁吸合时间
void ResetData(void);					//重置相关变量

//=======================
void ProtocolFrameServer(void);		//协议服务
void ProtocolFrameGet(void);			//按照协议打包数据
void ProtocolFrameSetData(void);			//按照协议建立数据包
void ProtocolFrameAnalysis(sCommunicationDef* sCommData,sCommunicationDef* sCommDataTxd);	//根据协议分析数据
void ProtocolFrameSend(sCommunicationDef* sCommData);			//根据协议打包发送帧数据


void BusFrameAck(sFarmeDef*	sFarme);			//外部总线应答
void BusFrameProcess(sFarmeDef*	sFarme,sBoradDef*	sBorad);			//外部总线应答

void BusFrameProcessData(sFarmeDef*	sFarme,sBoradDef*	sBorad);	//数据命令处理
void BusFrameProcessLock(sFarmeDef*	sFarme,sBoradDef*	sBorad);	//锁命令处理

void BoardServer(sBoradDef*	sBorad);										//

void BoardGetCardData(sCommunicationDef* sCommData);		//获取IC卡数据或者状态
void BoardSaveCardData(sBoradDef*	sBorad);		//根据状态保存卡号及设置相关状态
void BoardClrCardData(sBoradDef*	sBorad);		//删除卡号数据及设置相关状态

void BoardSetLockTime(sCommunicationDef* sCommData);		//设置锁吸合时间

void BoardSetSeg(sBoradDef*	sBorad);			//根据标志设置数码管状态（无箱，有箱，取箱，读卡通讯不良）
#endif



