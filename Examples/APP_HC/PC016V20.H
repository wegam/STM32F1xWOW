#ifndef __PC016V20_H
#define __PC016V20_H


#include "stm32f10x_exti.h"

#include "STM32_TOOLS.H"		//���У�鹤��
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


#include	"stdio.h"			//����printf
#include	"string.h"		//����printf
#include	"stdarg.h"		//���ڻ�ȡ��ȷ�������Ĳ���
#include	"stdlib.h"		//malloc��̬�����ڴ�ռ�

#define	MinPortNum		1			//��С/��ʼ�˿ں� 1~4
#define	MaxPortNum		4			//���˿ں� 1~4
#define	MaxSysTime		1000	//TimeSYS����ʱʱ�䣬��λms����������ʱʱ������м�ʱ������

//==============�˿�---������
#define	ICCardReadPort1					USART3		//J1
#define	ICCardReadPort2					USART1		//J2
#define	ICCardReadPort3					UART4			//J3
#define	ICCardReadPort4					UART5			//J4---��֧��DMA����
#define	ICCardReadBaudRate			19200
#define	ICCardReadBufferSize		22				//���յ�IC�������������ݴ�С��Э��+���ݣ�
//#define	ICCardDataSize					16				//IC�����ݴ�С����Ч�����ֽڳ��ȣ�
#define	ICDataStartByte					2					//16�ֽ���������Ч������ʼλ 0~15
#define	ICDataNum								3					//16�ֽ���������Ч�����ֽ���

#define	CardStartByte						0					//16�ֽ���������Ч������ʼλ 0~15
#define	CardLength							4					//16�ֽ���������Ч�����ֽ���

//==============���ö˿�

//==============�˿�---����485
#define	Bus485Port				USART2
#define	Bus485CtlPort			GPIOA
#define	Bus485CtlPin			GPIO_Pin_1
#define	Bus485BaudRate		19200											//������
#define	BusDataSize				256												//һ����ϢЯ����������ݸ���
#define	Bus485BufferSize	300												//һ����Ϣ����󻺴��С��Э������+���ݣ�
#define	CommProtocolLenth		11											//��׼Э�鲿�����ݳ���//sizeof(sCommHeadDef)+sizeof(sCommStartDef)+sizeof(sCommAddrDef)+sizeof(eCommErrorCodeDef)
#define	CommBasicLenth			4													//���ݳ���nLength�е�ַ��״̬��ĳ���

//==============�˿�---�����485
#define	Seg485PortRmap		USART1		//�˿��ض���
#define	Seg485CtlPort			GPIOB
#define	Seg485CtlPin			GPIO_Pin_5
#define	Seg485BaudRate		19200
#define	Seg485BufferSize	22
//==============�˿�---��
#define	LockPort(n)		LockPort##n
#define	LockPin(n)		LockPin##n
#define	LockOn(n)			GPIO_SetBits(LockPort(n),	LockPin(n))			//����
#define	LockOff(n)		GPIO_ResetBits(LockPort(n),	LockPin(n))		//�ͷ���

#define	LockPort1			GPIOC
#define	LockPin1			GPIO_Pin_3
#define	LockPort2			GPIOB
#define	LockPin2			GPIO_Pin_9
#define	LockPort3			GPIOB
#define	LockPin3			GPIO_Pin_3
#define	LockPort4			GPIOB
#define	LockPin4			GPIO_Pin_4


#define	DefaultOnTime	30000				//Ĭ��(��С)������ʱ��30S
#define	MaxOnTime			120000			//���������ʱ��120S	2����
#define	SegOnTime			30000				//����ҩ�������ܳ���ʱ��30S


#define	Pt(n)	GPIO_SetBits(LockPort(n),	LockPin(n))

//u8 ICCARD_CMD_GetDataBlock1[] =
//{
//	0xE1,   /* F-Head 	*/			//Ϊ֡ͷ����λ���·�ʱ�̶�Ϊ0XE1����λ��Ӧ��ʱ�̶�Ϊ0XD2��
//	0x24,  	/* CmdType 	*/			//��������
//	0x01,		/* Sector 	*/			//1�ֽ�������
//	0x01,   /* Block 		*/			//1�ֽڿ��
//	0x60,   /* KeyType	*/			//1�ֽ���Կģʽ KEYA��0x60��/KEYB(0x61)
//	0xFF,   /* PassWord0 */			//6�ֽ�����
//	0xFF,   /* PassWord1 */
//	0xFF,   /* PassWord2 */
//	0xFF,		/* PassWord3 */
//	0xFF,   /* PassWord4 */
//	0xFF,		/* PassWord5 */
//	0x1C,   /* Crc16-HIGN */
//	0x0D,		/* Crc16-LOW */
//	0x1E   	/* F-End*/						//Ϊ֡β����λ���·�ʱ�̶�Ϊ0X1E����λ��Ӧ��ʱ�̶�Ϊ0X2D��
//};
typedef enum
{
	eGetID	=	0x92,		//��λ��������ȡID����Ӧ������
	eSendID	=	0x93,		//��λ�������ϴ�ID������
	eLock		=	0x93,		//�����������ϱ�
}BusCmdDef;
typedef enum
{
	eNoErr		=	0x00,		//�޴���
	eNoData		=	0x23,		//��ҩ������
	eBoxTout	=	0x24,		//ȡҩ�䳬ʱ
	eBoxOff		=	0x25,		//ҩ��ȡ��
	eBoxBack	=	0x26,		//ҩ���ջ�
	eReadErr	=	0x27,		//������ͨѶ�쳣
	eBoxTake	=	0x28,		//ҩ�䱻ǿ��ȡ��
}StatusCodeDef;
typedef enum
{
	eHead			=	0x7E,		//ͷ��ʶ
	eEnd			=	0x7F,		//������ʶ
}BusFarmeDef;
//=========================������
typedef	struct	_ICCRC16
{
	u8 CRC_H;
	u8 CRC_L;
}ICCRC16Def;

typedef	struct	_ICBuffer			//���������ݸ�ʽ
{
	u8 Head;
	u8 CmdType;
	u8 Sector;
//	u8 Block;
	u8 data[16];
	ICCRC16Def CRC16;
	u8 End;
}ICBufferDef;

typedef	struct	_ICReadCmd		//���������ʽ
{
	u8	Head;						//0xE1,   /* F-Head 	*/			//Ϊ֡ͷ����λ���·�ʱ�̶�Ϊ0XE1����λ��Ӧ��ʱ�̶�Ϊ0XD2��
	u8	CmdType;				//0x24,  	/* CmdType 	*/			//��������
	u8	Sector;					//0x01,		/* Sector 	*/			//1�ֽ�������
	u8	Block;					//0x01,   /* Block 		*/			//1�ֽڿ��
	u8	KeyType;				//0x60,   /* KeyType	*/			//1�ֽ���Կģʽ KEYA��0x60��/KEYB(0x61)
	u8	PassWord[6];		//6�ֽ�����0xFF
	ICCRC16Def	CRC16;
	u8 End;
}ReadCmdDef;

//================485����ͨѶ�ṹ�壨������
typedef	struct	_sFarme
{
	u8	Head;			//b0ʶ���	0x7E
	u8	dAdd;			//b1Ŀ���ַ����ʱ����Ϊ��Ԫ���ַ���·�Ϊ���յ�ַ���ϴ�Ϊ0x00
	u8	sAdd;			//b2Դ��ַ �·�Ϊ0x00���ϴ�Ϊ����ַ
	u8	SN;				//b3��ˮ�� 0x01~0xFF ÿ��ʹ�ú��1
	u8	Cmd;			//b4����ţ�0x12-��ȡָ��IC���ţ�0x13ȡҩ
	u8	Ucode;		//b5�û��룺��������ԭ������
	u8	Length;		//b6��ַ+�쳣��̶�����Ϊ4�������ݳ���Ϊ3
	
	u8 	Addr1;		//b7���ַ(��Ԫ���)
	u8 	Addr2;		//b8���ַ
	u8	Addr3;		//b9�۵�ַ���˿ںţ�	
	
	u8	data[BusDataSize];		//DataBuffer
	
	u8	EC;				//������
	u8	Bcc8;			//���У��	//���㷶ΧΪ��ʼ�ε����ݶ�(dAdd~data[n])��������
	u8	End;			//������
}sFarmeDef;
//typedef	struct	_sBusData
//{
//	u8	dAdd;			//Ŀ���ַ����ʱ����Ϊ��Ԫ���ַ���ϼ������¼��ĵ�ַ
//	u8	Ucode;		//�û��룺��������ԭ������
//	u8 	Addr1;		//���ַ(��Ԫ���)
//	u8 	Addr2;		//���ַ
//	u8	Addr3;		//�۵�ַ���˿ںţ�	
//}sBusDataDef;
//=======================================================�����ͨѶ�ṹ�壺����ҩ�䳣��30S��ȡҩ������һ��ʱ������˸����ҩ�䣺С����һֱ��˸������ͨѶ���ϣ�С���㳣��
typedef struct _SegCmd
{
	unsigned char DispEnNum	:1;		//bit0��ʾ��ֵ	��	0-����ʾ��		1-��ʾ
	unsigned char DispEnDp	:1;		//bit1��ʾ��		��	0-����ʾ��		1-��ʾ
	unsigned char DispMdNum	:1;		//bit2��ֵģʽ	��	0-��̬��ʾ��	1-0.5S��˸
	unsigned char DispMdDp	:1;		//bit3��ģʽ		��	0-��̬��ʾ��	1-0.5S��˸
	unsigned char DispTime	:1;		//bit4��ʾʱ��	��	0-������			1-����ʾʱ���ڸ�����ʾģʽ��ʾ
} SegCmdDef;

typedef struct _sSeg 
{
	unsigned char desAddr;				//Ŀ�ĵ�ַ---����ܰ����ַ
	unsigned char srcAddr;				/*Դ��ַ*/
	unsigned char runningNumber;	/*��ˮ��*/
	SegCmdDef			cmd;						/*����*/
	unsigned char len;						/*����*/
	unsigned char data[8];				/*����,��λ��ǰ,data[0~3]Ϊ��ʾ����,data[4~7]Ϊ��˸ʱ��*/ 
} sSegDef;			//��������ݽṹ��
//=======================================================���ṹ��
typedef	struct	_LockFlag		//�����Ʊ�־������ʱ��
{
	unsigned long	On					:1;		//0-��������1-����
//	unsigned long	Off					:1;		//0-��������1-�ͷ���
	unsigned long	LockOnTime	:30;	//������ʱ�䣬���ֵ0x3FFF FFFF
	GPIO_TypeDef*	GPIOx;
	u16	GPIO_Pin_n;
}LockFlagDef;
//=======================================================ʱ��ṹ��
typedef	struct	_Time		//�����Ʊ�־0--��������1--��������
{
	unsigned long	TimeSYS;	//ϵͳ��ʱ��
	unsigned long	TimeSEG;	//����ܸ������ݼ�ʱ��----�����ڼ�������ͨѶ״̬ 20ms�ⷢһ�������״̬
	unsigned long	TimeBOX;	//ҩ�����ʱ��	ÿ���˿ڷ���100mSʱ����----�����ڼ�������ͨѶ״̬
	unsigned long	TimeBUS;	//�ⲿ����ʱ��
	unsigned long	TimeCard;	//������ʱ��
}TimeDef;
//=======================================================��λ״̬�ṹ��
typedef	struct	_sBoxStatus		//�˿�״̬--���п�/�޿�
{
	unsigned short	BoxFlg		:1;	//bit0��0-�޿�,1-�п�
	unsigned short	BoxBack		:1;	//bit1��0-�޲�����1-ҩ���ջ�,�ջغ���Ҫ�ϱ�ҩ��ţ���BoxOff��־ʱ���ô�λ��
	unsigned short	BoxRead		:1;	//bit2��0-������������1-������ͨѶ�쳣
	unsigned short	BoxSeg		:1;	//bit3��0-�޲�����1-��Ҫ���������
	unsigned short	BoxOff		:1;	//bit4��0-�޲�����1-ҩ�䱻ȡ�ߣ���BoxDel����ʱ���ô�λ��
	unsigned short	RecData		:1;	//bit5��0-�޽��յ������ݣ�1-�н��յ�������
	unsigned short	ClrData		:1;	//bit6��0-�޲�����1-ɾ����������(ҩ�䲻�ڣ���Ҫɾ�����ݼ����������
	unsigned short	GetID			:1;	//bit7��0-������		1-������	����ȡID������Ҫ�ϱ�ID
	unsigned short	GetBox		:1;	//bit8��0-������		1-������	��ȡҩ(�������ֵ��˸)
	unsigned short	SendID		:1;	//bit14��0-�޲�����	1-��⵽ҩ����룬�����ϱ�ID�������ȡҩ��ȡ�������ϱ���ʹ��״̬�ϱ�	
	unsigned short	SendSts		:1;	//bit10��0-�޲�����1-ȡҩ״̬��ҩ��仯����Ҫ�ϱ�ҩ��״̬������������������ϱ�״̬
	unsigned short	BoxTout		:1;	//bit11��0-�޲�����1-ȡҩ��ʱ
	unsigned short	BoxTake		:1;	//bit11��0-�޲�����1-ҩ�䱻ǿ��ȡ��
}sBoxStatusDef;

//=======================================================�˿�/��λ������Ϣ�ṹ��
typedef	struct	_Port
{
	sBoxStatusDef		Status;								//����״̬
	sSegDef					Seg;									//��������ݽṹ��
	LockFlagDef			Lock;									//����Ϣ
//	sBusDataDef			BusData;							//�����ϼ��·����������
	unsigned char 	CardData[CardLength];	//��������CardData[0]Ϊ��λ��CardData[1]Ϊʮλ��CardData[2]Ϊ��λ
	unsigned long		CardNumber;						//ʮ����
	unsigned short	TimeCount;						//��ʱ��	
	unsigned char		Ucode;								//�û���
	unsigned char		PortNum;							//�۵�ַ���˿ںţ�
} PortDef;
//=======================================================���в���
typedef	struct	_Step
{
	unsigned char 	ReadCard		:1;			//bit0:	0-δִ�ж���,��Ҫ������		1-�Ѷ��꿨
	unsigned char 	WriteSeg		:1;			//bit1:	0-δ���������,��Ҫ���£�	1-�Ѹ���(���������в���������ܣ����ռ�ô��ڣ�
	unsigned char 	BoxStatus		:1;			//bit3:	0-δ����ҩ��״̬����Ҫ���£�	1-�Ѹ���
} StepDef;	
//=======================================================���ڶ˿�/��������
typedef	struct	_sBorad		//�˿�״̬--���п�/�޿�
{
	PortDef	Port1;
	PortDef	Port2;
	PortDef	Port3;
	PortDef	Port4;
	StepDef	Step;
	TimeDef	Time;		//�����ܼ�ʱ��
	u8	Nserial;		//��ˮ�� 1~0xFF
}sBoradDef;


//=======================================================ȫ�ֱ���
extern sBoradDef	sBorad;					//�˿�/��λ������Ϣ�ṹ��
extern SWITCHID_CONF	SWITCHID;			//���뿪��

//==============����������
extern const u8 CardReaderCmd_SetArea[14];		//���ö�����������������
extern const u8 CardReaderCmd_ReadData[14];		//��������


//==============485������
extern RS485_TypeDef	RS485_Bus;	//����ͨѶ485
extern RS485_TypeDef	RS485_Seg7;	//�������ʾ485

//==============485������
extern sFarmeDef	sFarme;				//����485���ݽṹ��---������
extern sFarmeDef	sFarmeTxd;		//����485���ݽṹ��---����
extern sFarmeDef	sFarmeRxd;		//����485���ݽṹ��---����
extern sFarmeDef	sFarmeRev;		//����485���ݽṹ��---����
extern u8 RS485BusTxd[Bus485BufferSize];


extern u8 Seg485Txd[Seg485BufferSize];
extern u8 Seg485Rxd[Seg485BufferSize];
extern u8 Seg485Rev[Seg485BufferSize];

//==============������������
extern ReadCmdDef	ReadCmd;			//��������
extern u8	CardData[CardLength];		//����Ч���ݻ���						
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

//==============��������

extern u8 SwitchData;						//�洢���뿪�����µ�ַ�������ַ�仯���������������в���
//extern u32	SYSTIME;						//ϵͳ��ʱ��
//extern TimeDef	SYSTime;				//ϵͳ������ؼ�ʱ��
extern u32	Port4TimeOut;				//�˿�4����������ʱ


//=================================================================================================================������
void PC016V20_Configuration(void);
void PC016V20_Server(void);



//=================================================================================================================Э�����
void RS485_Server(void);							//RS485�շ�����

void BusFrameAck(sFarmeDef*	sFarme);			//�ⲿ����Ӧ��
void MessageProcess(sFarmeDef*	sFarme,sBoradDef*	sBorad);				//�ⲿ������Ϣ����

void BusFrameProcessGetID(sFarmeDef*	sFarme,sBoradDef*	sBorad);		//���������
void BusFrameProcessGetBox(sFarmeDef*	sFarme,sBoradDef*	sBorad);		//���������---ȡҩ

void BusFrameProcessSendBox(sFarmeDef*	sFarme,sBoradDef*	sBorad);	//����ҩ������

void BusFrameProcessPacketAndSend(sFarmeDef*	sFarme);					//����Э�������ݲ�����
void BusFrameProcessLock(sFarmeDef*	sFarme,sBoradDef*	sBorad);	//�������

//=================================================================================================================ȡҩ
void BoardGetBoxProcess(sBoradDef*	sBorad);		//ȡҩ������ȡҩ��ʱ

//=================================================================================================================����������
u8 	 BoardCardReaderServer(void);					//��ȡIC������
void BoardCardReaderReadAll(sBoradDef*	sBorad);			//����4���˿ڶ���ָ��
void BoardSaveCardData(sBoradDef*	sBorad);		//����״̬���濨�ż��������״̬
void BoardClrCardData(sBoradDef*	sBorad);		//ɾ���������ݼ��������״̬

//=================================================================================================================����ܿ��Ƴ���
void BoardSetSeg(sBoradDef*	sBorad);			//���ݱ�־���������״̬�����䣬���䣬ȡ�䣬����ͨѶ������
//=================================================================================================================������
void Lock_Server(sBoradDef*	sBorad);
//=================================================================================================================Ӳ����������
void HALUsartRemapEnable(void);				//ʹ�ܴ��ڸ���
void HALUsartRemapDisable(void);			//�رմ��ڸ���
void HALSendSeg(u32* Buffer);					//������ܷ�������

//=================================================================================================================���ó���
void Lock_Configuration(void);				//����ʼ��
void Switch_Configuration(void);			//���뿪�س�ʼ��������
void RS485_Configuration(void);				//RS485����
void CardReader_Configuration(void);	//����������
void Data_Initialize(void);						//������ʼ��
void ResetData(void);									//������ر���
#endif


