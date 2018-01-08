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

//==============端口---读卡器
#define	ICCardReadPort1					USART3		//J1
#define	ICCardReadPort2					USART1		//J2
#define	ICCardReadPort3					UART4			//J3
#define	ICCardReadPort4					UART5			//J4---不支持DMA传输
#define	ICCardReadBaudRate			19200
#define	ICCardReadBufferSize		22				//接收到IC读卡器完整数据大小（协议+数据）
#define	ICCardDataSize					16				//IC卡数据大小（有效卡号字节长度）

//==============端口---总线485
#define	Bus485Port				USART2
#define	Bus485CtlPort			GPIOA
#define	Bus485CtlPin			GPIO_Pin_1

#define	Bus485BaudRate		19200											//波特率
#define	BusDataSize				256												//一条消息携带的最大数据个数
#define	Bus485BufferSize	sizeof(sCommunicationDef)	//一条消息的最大缓存大小（协议数据+数据）

//==============端口---数码管485
#define	Seg485Port				USART1Rmap		//端口重定义
#define	Seg485BaudRate		19200
#define	Seg485BufferSize	32
//==============端口---锁
#define	LockPort1			GPIOC
#define	LockPin1			GPIO_Pin_3

#define	LockPort2			GPIOB
#define	LockPin2			GPIO_Pin_9

#define	LockPort3			GPIOB
#define	LockPin3			GPIO_Pin_3

#define	LockPort4			GPIOB
#define	LockPin4			GPIO_Pin_4

#define	DefaultOnTime	10000				//默认锁吸合时间10S
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

typedef	struct	_ICData	//IC卡数据
{
	u8	Port1Data[ICCardDataSize];
	u8	Port2Data[ICCardDataSize];
	u8	Port3Data[ICCardDataSize];
	u8	Port4Data[ICCardDataSize];
}ICDataDef;

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
	eCommErr35	=	0x23,		//无药箱数据
	eCommErr36	=	0x24,		//取药箱超时
	eCommErr37	=	0x25		//取药箱超时
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
	eCommCmd16	=	0x11,		//获取所有IC卡号
	eCommCmd17	=	0x12,		//获取指定IC卡号
	eCommCmd18	=	0x13		//开锁取药箱
}eCommCmdDef;
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
	eCommAck145	=	0x91,		//获取所有IC号
	eCommAck146	=	0x92,		//获取指定IC号
	eCommAck147	=	0x93		//开锁取药箱
}eCommAckDef;
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
	u8						nBcc8;			//8位数异或校验码：地址段+数据段 校验
	u8						nUserCode;	//用户码0x01~0xFF,上位机使用标识，不做处理，原数返回
	u8						nLength;		//地址段+数据段 的总字节数
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

//typedef	struct	_Communication
//{
//	CommHeadDef	Head1;			//头标识符	0xFA
//	CommHeadDef	Head2;			//头标识符	0xF5
//	u8 Ser;									//流水号0x01~0xFF
//	CommCmdDef Cmd;					//命令：
//	u8 CRC8;								//校验码(ADDR1~data异或校验）
//	u8 UserCode;						//用户码
//	u8 Length;							//用户码
//	u8 Addr1;								//单元柜地址
//	u8 Addr2;								//层地址
//	u8 Addr3;								//槽地址
//	CommErrorDef ErrorCode;	//异常码
//	u8 data[ICCardDataSize];						//Buffer
//}CommunicationDef;
//=========================运行
typedef	struct	_PortFlag		//端口状态--：有卡/无卡
{
	unsigned char	Port1Flg:2;	//0-无卡，1-有卡
	unsigned char	Port2Flg:2;
	unsigned char	Port3Flg:2;
	unsigned char	Port4Flg:2;
}PortFlagDef;

typedef	struct	_LockFlag		//锁控制标志0--不处理，1--开锁操作
{
	unsigned char	Lock1:2;	//0--不处理，1--开锁操作
	unsigned char	Lock2:2;
	unsigned char	Lock3:2;
	unsigned char	Lock4:2;
}LockFlagDef;


//=========================运行
extern SWITCHID_CONF	SWITCHID;			//拔码开关
extern LockFlagDef	LockFlag;				//锁控制标志0--不处理，1--开锁操作

//==============485结体体
extern RS485_TypeDef	RS485_Bus;	//总线通讯485
extern RS485_TypeDef	RS485_Seg7;	//数码管显示485

//==============485缓冲区
extern sCommunicationDef	CommData;			//总线485数据结构体
//extern CommunicationDef	CommData1;			//总线485数据结构体---端口1
//extern CommunicationDef	CommData2;			//总线485数据结构体---端口2
//extern CommunicationDef	CommData3;			//总线485数据结构体---端口3
//extern CommunicationDef	CommData4;			//总线485数据结构体---端口4
extern u8 RS485BusTxd[Bus485BufferSize];
extern u8 RS485BusRxd[Bus485BufferSize];
extern u8 RS485BusRev[Bus485BufferSize];

//==============读卡器缓冲区
extern PortFlagDef	PortFlag;	//端口状态--：有卡/无卡
extern ReadCmdDef	ReadCmd;			//读卡命令
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
extern u8 SwitchData;		//存储拔码开关最新地址，如果地址变化，再重新配置运行参数
extern u32	SYSTIME;

extern u32	LockOnTime[4];	//锁吸合时间，如果时间为0则释放锁





//========================主程序
void PC016V20_Configuration(void);
void PC016V20_Server(void);

//========================服务程序
void RS485_Server(void);							//RS485收发处理
u8 ICCardReader_Server(void);					//读取IC卡数据
void Lock_Server(void);


//========================配置程序
void RS485_Configuration(void);					//RS485配置
void ICCardReader_Configuration(void);	//读卡器配置
void SWITCHID_Configuration(void);			//拔码开关初始化及读数
void Lock_Configuration(void);			//锁初始化


//========================驱动程序
void ICCardReader_Read(u8 Num);							//读相应端口的卡
void ICCardDataPacket(void);		//数据打包


void LockOnTimeSet(void);			//设置锁吸合时间
void Lock_On(u8 Num);					//开锁
void Lock_Off(u8 Num);				//释放锁
void ResetData(void);					//重置相关变量
#endif



