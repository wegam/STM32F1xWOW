#ifndef __ICCARD_H
#define __ICCARD_H


//#include "stm32f10x_gpio.h"
//#include "stm32f10x_rcc.h"
//#include "STM32_GPIO.H"
#include "STM32_PWM.H"
//#include "STM32_GPIO.H"
//#include "STM32_USART.H"
//#include "STM32_DMA.H"

//================IC������
typedef enum	_ICCARD_Cmd
{
	GetOnlineDevice		=	0x20,		//�������߻���
	UpDataUID					=	0x23,		//ˢ���ϴ�UID�������� 0x23
	GetBlockData			=	0x24,		//�������飨���ݣ�
	SetBlockData			=	0x25,		//д������
	SetPassWord				=	0x26,		//�޸���������
	SetRelayOn				=	0x40,		//�̵�����
	SetRelayOff				=	0x41,		//�رռ̵���
	RemoteCtl					=	0x3E,		//Զ�̲���
	SetReaderID				=	0x42,		//���ö�ͷ���
	GetReaderID				=	0x43,		//��ȡ��ͷ���
	SetReaderKey			=	0x44		//���ö�ͷ����	
}ICCARD_CmdType;

//=================д������
#define ICCARD_MO_SetDataBlock(Sector,	/*����*/	\
																Block,	/*��*/			\
																ICCARD_Number	/*����*/	\
																)												\
0xE1,   /* F-Head 	:Ϊ֡ͷ����λ���·�ʱ�̶�Ϊ0XE1����λ��Ӧ��ʱ�̶�Ϊ0XD2*/	\
0x25,  	/* CmdType 	:��������*/	\
0x01,		/* Sector 	:1�ֽ�������*/	\
0x01,   /* Block 	:1�ֽڿ��	*/	\
0x60,   /* KeyType	:1�ֽ���Կģʽ KEYA��0x60��/KEYB(0x61)*/	\
0xFF,   /* PassWord0 :6�ֽ�����*/	\
0xFF,   /* PassWord1 */	\
0xFF,   /* PassWord2 */	\
0xFF,		/* PassWord3 */	\
0xFF,   /* PassWord4 */	\
0xFF,		/* PassWord5 */	\
0xFF,   /* Data0 	:16�ֽ�����*/	\
0xFF,   /* Data1 	*/	\
0xFF,   /* Data2 	*/	\
0xFF,   /* Data3 	*/	\
0xFF,   /* Data4 	*/	\
0xFF,   /* Data5 	*/	\
0xFF,   /* Data6 	*/	\
0xFF,   /* Data7 	*/	\
0xFF,   /* Data8 	*/	\
0xFF,   /* Data9 	*/	\
0xFF,   /* Data10 	*/	\
0xFF,   /* Data11 	*/	\
0xFF,   /* Data12 	*/	\
0xFF,   /* Data13 	*/	\
0xFF,   /* Data14 	*/	\
0xFF,   	/* Data15 	*/	\
0x1C,   /* Crc16-HIGN */	\
0x0D,		/* Crc16-LOW */	\
0x1E   	/* F-End	:Ϊ֡β����λ���·�ʱ�̶�Ϊ0X1E����λ��Ӧ��ʱ�̶�Ϊ0X2D*/


	
	

void ICCARD_Configuration(void);
void ICCARD_Server(void);

unsigned char* ICCARD_WriteData(u32	Number);

#endif