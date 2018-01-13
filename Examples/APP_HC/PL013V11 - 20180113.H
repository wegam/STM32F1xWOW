#ifndef __PL013V11_H
#define __PL013V11_H

#include "stm32f10x_type.h"


//==============�˿�---����485
#define	Bus485Port				USART2
#define	Bus485CtlPort			GPIOA
#define	Bus485CtlPin			GPIO_Pin_1
#define	Bus485BaudRate		19200											//������
#define	Bus485DataSize		64												//һ����ϢЯ����������ݸ���

#define	DPPort	GPIOB				//С������ƶ˿�
#define	DPPin		GPIO_Pin_4	//С�������΢�ܽ�

#define	PortA0	GPIOB
#define	PinA0		GPIO_Pin_0

#define	PortA1	GPIOB
#define	PinA1		GPIO_Pin_1

#define	PortA2	GPIOB
#define	PinA2		GPIO_Pin_2

#define	PortA3	GPIOB
#define	PinA3		GPIO_Pin_3

#define	PortEN1	GPIOB
#define	PinEN1	GPIO_Pin_15

#define	PortEN2	GPIOB
#define	PinEN2	GPIO_Pin_14

#define	PortEN3	GPIOB
#define	PinEN3	GPIO_Pin_5

//=======================����ṹ��
typedef struct _Cmd
{
	unsigned char DispEnNum	:1;		//��ʾ��ֵ	��	0-����ʾ��		1-��ʾ
	unsigned char DispEnDp	:1;		//��ʾ��		��	0-����ʾ��		1-��ʾ
	unsigned char DispMdNum	:1;		//��ֵģʽ	��	0-��̬��ʾ��	1-0.5S��˸
	unsigned char DispMdDp	:1;		//��ģʽ		��	0-��̬��ʾ��	1-0.5S��˸
	unsigned char DispTime	:1;		//bit4��ʾʱ��	��	0-������			1-����ʾʱ���ڸ�����ʾģʽ��ʾ
} CmdDef;
//=======================���սṹ��
typedef struct _sReceFarme
{
	unsigned char desAddr;				/*Ŀ�ĵ�ַ*/
	unsigned char srcAddr;				/*Դ��ַ*/
	unsigned char runningNumber;	/*��ˮ��*/
	CmdDef				cmd;						/*����*/
	unsigned char len;						/*����*/
	unsigned char data[8];				/*����,��λ��ǰ,data[0~3]Ϊ��ʾ����,data[4~7]Ϊ��˸ʱ��,0��ʾ����˸*/ //��ʾʱ��Ϊ0xFFFF FFFF��ʾΪ����
} sReceFarmeDef;				//���սṹ��	
//=======================Ӧ��ṹ��
typedef struct uart2_send 
{
	unsigned char desAddr;				/*Ŀ�ĵ�ַ*/
	unsigned char srcAddr;				/*Դ��ַ*/
	unsigned char runningNumber;	/*��ˮ��*/
	unsigned char cmd;						/*����*/
	unsigned char len;						/*����*/
	unsigned char data[1];				/*����*/
} UART2_SEND;


extern sReceFarmeDef	sReceFarmeRxd;				//���սṹ��---���ջ�����
extern sReceFarmeDef	sReceFarmeRev;				//���սṹ��---���ݻ�����
extern sReceFarmeDef	sReceFarmeSrv;				//���սṹ��---������������




void PL013V11_Configuration(void);
void PL013V11_Server(void);


void SWITCHID_Configuration(void);			//���뿪�س�ʼ��������
void RS485_Configuration(void);			//RS485����


void CD4511_Configuration(void);			//���������
void CD4511_DISPALY(u8 wei,u16 num);
void CD4511_DisplayClear(void);		//�����ʾ
void CD4511_DisplayDp(void);			//��λ��ʾС����

void RS485_Server(void);			//RS485��������

void DataServer(void);		//�������յ�����Ч����

void DisplayServer(void);		//��ʾ������򣬸�����ʾ����ˢ��

void SwitchIdServer(void);		//������ֵ���ޱ仯���������ID



#endif
