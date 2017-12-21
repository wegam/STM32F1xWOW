/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : PC001V21.c
* Author             : WOW
* Version            : V2.0.1
* Date               : 06/26/2017
* Description        : PC001V21����ư�.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


//SW6 ON��ʾ����ת��صİ�
//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������

//CAN�˲�����ʹ�� 0x0A
//��������CAN-IDΪ0x80
//��ת������ư�CAN-IDΪ0xC0

//��ת������ư����ӿ�
//CAN�˲�����0x09
//CAN-IDΪ0xCC
#define	Cmd_Group	0x09
#define	Cmd_ID		0xCC

//��������ͨѶ�˿�
//CAN�˲����� 0x08
//CAN-IDΪ�����ַ
#define	DATA_Group	0x08
#define	DATA_ID			0xFA

#ifdef PC006V21			//�ּ�����ư�

#include "PC006V21.H"

#include	"stdio.h"			//����printf
#include	"string.h"		//����printf
#include	"stdarg.h"		//���ڻ�ȡ��ȷ�������Ĳ���
#include	"stdlib.h"		//malloc��̬�����ڴ�ռ�



#include "SWITCHID.H"
#include "STM32_WDG.H"
#include "STM32_EXTI.H"
#include "STM32_GPIO.H"
#include "STM32_PWM.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_CAN.H"
#include "STM32F10x_BitBand.H"

//=================PA6������ư�CCW���������
#define	MOTOR_PWM_PORT	GPIOA
#define	MOTOR_PWM_Pin		GPIO_Pin_7
//=================PA7������ư�CW���������
#define	MOTOR_DIR_PORT	GPIOA
#define	MOTOR_DIR_Pin		GPIO_Pin_6

//=================PA6��������CCW���������
#define	TXLED_PORT	GPIOA
#define	TXLED_Pin		GPIO_Pin_7
//=================PA7��������CW���������
#define	RXLED_PORT	GPIOA
#define	RXLED_Pin		GPIO_Pin_6

#define	MOTOR_PWM_Frequency	500	//����Ƶ��--��λHz
#define	PWM_UpdataCount	10					//Ƶ�ʱ仯ռ�������������Ǽ���PWM_Updata���������һ�����Ƶ��
#define	PWM_RunUpCount	1000				//����/������Ҫ�Ĳ���
#define	MOTOR_TIMx	TIM1				//��ת�����ʹ�õĶ�ʱ��

#define	Steeps		4000					//һ�������õ�������������
#define	MaxWindow	4							//��󴰿���

#define	MOTOR_RunRight			MOTOR_DIR_PORT->BSRR 	= MOTOR_DIR_Pin		//EN	=	1;	//˳ʱ��
#define	MOTOR_RunLeft				MOTOR_DIR_PORT->BRR		= MOTOR_DIR_Pin		//EN	=	0;	//��ʱ��

//=================������
#define	Sensor1_Port	GPIOB		//ǰ������
#define	Sensor1_Pin		GPIO_Pin_4

#define	Sensor2_Port	GPIOB		//�󴫸���
#define	Sensor2_Pin		GPIO_Pin_5

#define	Sensor3_Port	GPIOB		//ҩƷ������
#define	Sensor3_Pin		GPIO_Pin_6

#define	Sensor4_Port	GPIOB		//���ô�����
#define	Sensor4_Pin		GPIO_Pin_7

//======������״̬������ȡ״̬Ϊ�߱�ʾ���źţ���-���ź�
#define	Sensor1_Status	(Sensor1_Port->IDR & Sensor1_Pin)
#define	Sensor2_Status	(Sensor2_Port->IDR & Sensor2_Pin)
#define	Sensor3_Status	(Sensor3_Port->IDR & Sensor3_Pin)
#define	Sensor4_Status	(Sensor4_Port->IDR & Sensor4_Pin)

SWITCHID_CONF SWITCHID;					//���뿪�ؽṹ��
PWM_TimDef		PWM_Tim;					//PWM��������ṹ��

u8 SwitchData	=	0;	//�洢���뿪�����µ�ַ�������ַ�仯���������������в���
u8 SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������
u8 MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
u32	MotorTime	=	0;	//��ת�����������ʱ��

u8	CANID	=	0;
u16 SYSTime=0;
//u16 data=0;
//u8 Flag=0;
u8 Sensor[4]	=	{0};	//4����������Ӧֵ�洢��
u8 SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�

u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
u8 StatusOfWindow	=	0;	//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
u8 CMDOfWindow	=	0;		//���е�ָ����������



CanRxMsg RxMessage;			//CAN���� 
CanTxMsg TxMessage;			//CAN����




void SWITCHID_Configuration(void);			//���뿪�س�ʼ��������
void Motor_Configuration(void);					//���������������
void Sensor_Configuration(void);				//����������
void CAN_Configuration(void);						//CAN����
void LED_Configuration(void);						//��Ϊ��������ʱ������LEDָʾ��


void Motor_RunSet(int Num);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
void Sensor_Read(void);					//���������ź�	
void Reset_Data(void);					//��λ���е�ȫ�ֱ���ֵ
u8 SetToWindows(u8 nWindows);		//���е�ָ������	

void SensorBD_Server(void);		//��������������
void MotorBD_Server(void);		//������ư�������
u8 Motor_Server(void);				//�����������״̬
void CAN_Server(void);			//CAN�շ����ݹ���
void Switch_Server(void);			//�������ַ���ޱ�����������������������в���
/*******************************************************************************
*������		:	function
*��������	:	��������˵��
*����			: 
*���			:	��
*����ֵ		:	��
*����			:
*******************************************************************************/
void PC006V21_Configuration(void)
{
	SYS_Configuration();											//ϵͳ���� STM32_SYS.H	
	SysTick_DeleymS(100);											//SysTick��ʱnmS
	PWM_OUT(TIM2,PWM_OUTChannel1,1,500);			//SYS-LED 1HZ 50%
	SWITCHID_Configuration();									//���뿪�س�ʼ��������
	
	if(SensorBD)
	{
		Sensor_Configuration();				//����������
		SysTick_Configuration(100);		//ϵͳ���ʱ������72MHz,��λΪuS
		LED_Configuration();					//��Ϊ��������ʱ������LEDָʾ��
	}
	else if(MotorBD)
	{
		SysTick_Configuration(1000);							//ϵͳ���ʱ������72MHz,��λΪuS
		Motor_Configuration();										//���������������
	}
	
	CAN_Configuration();							//CAN1����---��־λ��ѯ��ʽ�������ж�--500K

//	IWDG_Configuration(1000);	//�������Ź�����	Tout-��ʱ��λʱ�䣬��λms
	
}

/*******************************************************************************
*������		:	function
*��������	:	��������˵��
*����			: 
*���			:	��
*����ֵ		:	��
*����			:
*******************************************************************************/
void PC006V21_Server(void)
{
	if(SensorBD)
	{
		SensorBD_Server();		//��������������
	}	
	else if(MotorBD)
	{
		MotorBD_Server();		//������ư�������
	}
	Switch_Server();			//�������ַ���ޱ�����������������������в���
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void SensorBD_Server(void)		//��������������
{
	Sensor_Read();		//���������ź�
	if(SensorON	&&	SYSTime%10==0)			//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
	{
		CAN_StdTX_DATA(CANID,4,Sensor);			//CANʹ�ñ�׼֡��������
	}
	
	SYSTime++;
	if(SYSTime>=100000)
	{		
		SYSTime=0;
	}
	//===============�����������ϴ�����������ʱLED��˸
	if(SensorON)
	{
		if(SYSTime%1000==0)
		{
			TXLED_PORT->BSRR=TXLED_Pin;		//PA6��������CCW���������---LED_ON
			RXLED_PORT->BSRR=RXLED_Pin;		//PA7��������CW���������---LED_ON
		}
		else	if(SYSTime%1000==200)
		{
			TXLED_PORT->BRR=TXLED_Pin;		//PA6��������CCW���������---LED_OFF
			RXLED_PORT->BRR=RXLED_Pin;		//PA7��������CW���������---LED_OFF
		}
	}
	else
	{
		TXLED_PORT->BRR=TXLED_Pin;		//PA6��������CCW���������---LED_OFF
		RXLED_PORT->BRR=RXLED_Pin;		//PA7��������CW���������---LED_OFF
	}		
	IWDG_Feed();				//�������Ź�ι��
	CAN_Server();				//CAN�շ����ݹ���
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void MotorBD_Server(void)		//������ư�������
{
	if(Motor_Server())					//�����������״̬---Ϊ��ʱ���ж�
	{
		return;
	}
	if(SYSTime==0	)
	{
//			memset(Sensor,0x08,4);
		Sensor[0]	=	0xAF;
		Sensor[1]	=	0x01;
		SensorON	=	1;
		CAN_StdTX_DATA(CANID,2,Sensor);				//CANʹ�ñ�׼֡��������
		memset(Sensor,0x00,2);
	}
	else if(SYSTime==2000)
	{
//			Sensor[0]	=	0xAF;
//			Sensor[1]	=	0x00;
//			SensorON	=	0;
//			CAN_StdTX_DATA(CANID,2,Sensor);				//CANʹ�ñ�׼֡��������
//			memset(Sensor,0x00,2);
	}	
	SYSTime++;
	if(SYSTime>=4000)
	{		
		SYSTime=0;
	}
	
	IWDG_Feed();				//�������Ź�ι��
	CAN_Server();				//CAN�շ����ݹ���
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void LED_Configuration(void)	//��Ϊ��������ʱ������LEDָʾ��
{
//	//=================PA6��������CCW���������
//#define	TXLED_PORT	GPIOA
//#define	TXLED_Pin		GPIO_Pin_7
////=================PA7��������CW���������
//#define	RXLED_PORT	GPIOA
//#define	RXLED_Pin		GPIO_Pin_6
	GPIO_Configuration_OPP2	(TXLED_PORT,TXLED_Pin);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
	GPIO_Configuration_OPP2	(RXLED_PORT,RXLED_Pin);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
}

/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void CAN_Configuration(void)		//CAN����
{
	//SW6 ON��ʾ����ת��صİ�
	//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������
	//CAN�˲�����ʹ�� 0x0A
	//��������CAN-IDΪ0x80
	//��ת������ư�CAN-IDΪ0xC0
	
	CAN_Configuration_NR(500000);							//CAN1����---��־λ��ѯ��ʽ�������ж�--500K
	if((SWITCHID.nSWITCHID	&	0x20)	==	0x20)
	{
		if((SWITCHID.nSWITCHID	&	0x30)	==	0x30)	//������ư�
		{
			CANID	=	0xC0;
		}
		else
		{
			CANID	=	0x80;			
		}
		//======��ת������ư��봫������ͨѶ�˿��˲���-----��ת������ư�����ת������ͨѶ�˲�����
		CAN_FilterInitConfiguration_StdData(0X0A,CANID,0xFFBF);			//CAN�˲�������---��׼����֡ģʽ---BIT7������
		//======��ת������ư����ⲿͨѶͨѶ�˿��˲���
		if(MotorBD)	//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
		{
			CAN_FilterInitConfiguration_StdData(Cmd_Group,	Cmd_ID,	0xFFFF);			//CAN�˲�������---��׼����֡ģʽ---������ư������ƽӿ�
			CAN_FilterInitConfiguration_StdData(DATA_Group,	DATA_ID,0xFFFF);			//CAN�˲�������---��׼����֡ģʽ---������ư�������ݽӿ�			
		}
	}
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void SWITCHID_Configuration(void)			//���뿪�س�ʼ��������
{
	//SW6 ON��ʾ����ת��صİ�
	//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������
	//CAN�˲�����ʹ�� 0x0A
	//��������CAN-IDΪ0x80
	//��ת������ư�CAN-IDΪ0xC0
	//	u8 MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
	//	u8 SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������
	
	SWITCHID.NumOfSW	=	6;
	//SW1--PA4
	SWITCHID.SW1_PORT	=	GPIOA;
	SWITCHID.SW1_Pin	=	GPIO_Pin_4;
	//SW2--PA5
	SWITCHID.SW2_PORT	=	GPIOA;
	SWITCHID.SW2_Pin	=	GPIO_Pin_5;
	//SW3--PB12
	SWITCHID.SW3_PORT	=	GPIOB;
	SWITCHID.SW3_Pin	=	GPIO_Pin_12;
	//SW4--PB13
	SWITCHID.SW4_PORT	=	GPIOB;
	SWITCHID.SW4_Pin	=	GPIO_Pin_13;
	//SW5--PB14
	SWITCHID.SW5_PORT	=	GPIOB;
	SWITCHID.SW5_Pin	=	GPIO_Pin_14;
	//SW6--PB15
	SWITCHID.SW6_PORT	=	GPIOB;
	SWITCHID.SW6_Pin	=	GPIO_Pin_15;
	
	SWITCHID_Conf(&SWITCHID);		//
	SWITCHID_Read(&SWITCHID);		//
	
	if((SWITCHID.nSWITCHID	&	0x20)	==	0x20)
	{
		if((SWITCHID.nSWITCHID	&	0x30)	==	0x30)	//������ư�
		{
			MotorBD		=	1;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
		}
		else
		{
			SensorBD	=	1;		//���������־��0-�Ǵ������壬1-��������		
		}
	}
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void Motor_Configuration(void)			//���������������
{
	//======PWM���
	PWM_Tim.PWM_BasicData.GPIOx	=	MOTOR_PWM_PORT;
	PWM_Tim.PWM_BasicData.GPIO_Pin_n	=	MOTOR_PWM_Pin;
	PWM_Tim.PWM_BasicData.PWM_Frequency	=	MOTOR_PWM_Frequency;	//Ƶ�� ��СƵ��0.2Hz
	PWM_Tim.PWM_BasicData.PWM_Updata	=	PWM_UpdataCount;		//Ƶ�ʱ仯ռ�������������Ǽ���PWM_Updata���������һ�����Ƶ��
	PWM_Tim.PWM_BasicData.PWM_RunUp	=	PWM_RunUpCount;				//����/������Ҫ�Ĳ���
	
	PWM_Tim.PWM_BasicData.TIMx	=	MOTOR_TIMx;
	PWM_OUT_TIMConf(&PWM_Tim);									//PWM�������;
//	PWM_OUT_SetFre(&PWM_Tim);										//����ʱ��
	PWM_OUT_SetCount(&PWM_Tim,0);									//����������������

	//======����������
	GPIO_Configuration_OPP50	(MOTOR_DIR_PORT,		MOTOR_DIR_Pin);			//CW	//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void Sensor_Configuration(void)		//����������
{
//#define	Sensor1_Port	GPIOB		//ǰ������
//#define	Sensor1_Pin		GPIO_Pin_4

//#define	Sensor2_Port	GPIOB		//�󴫸���
//#define	Sensor2_Pin		GPIO_Pin_5

//#define	Sensor3_Port	GPIOB		//ҩƷ������
//#define	Sensor3_Pin		GPIO_Pin_6

//#define	Sensor4_Port	GPIOB		//���ô�����
//#define	Sensor4_Pin		GPIO_Pin_7
	
	//========����������Ϊ��������ģʽ
	GPIO_Configuration_IPU(Sensor1_Port,	Sensor1_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
	GPIO_Configuration_IPU(Sensor2_Port,	Sensor2_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
	GPIO_Configuration_IPU(Sensor3_Port,	Sensor3_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
	GPIO_Configuration_IPU(Sensor4_Port,	Sensor4_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void Motor_RunSet(int Num)			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
{
	if(Num	==	0)		//ǿ��ֹͣ
	{
//		MOTOR_DISABLE;								//ֹͣ���
		PWM_OUT_SetCount(&PWM_Tim,0);		//�����������
	}
	else if(Num>0)
	{
//		Motor_Configuration();			//���������������
		PWM_Tim.PWM_BasicData.PWM_Frequency=MOTOR_PWM_Frequency;						//Ƶ������--����
		PWM_OUT_SetFre(&PWM_Tim);
		MOTOR_RunRight;													//˳ʱ��
		PWM_OUT_SetCount(&PWM_Tim,Num*Steeps);		//����������������
	}
	else
	{
//		Motor_Configuration();			//���������������
		PWM_Tim.PWM_BasicData.PWM_Frequency=MOTOR_PWM_Frequency;						//Ƶ������--����
		PWM_OUT_SetFre(&PWM_Tim);
		Num	=	0	-	Num;
		MOTOR_RunLeft;													//˳ʱ��
		PWM_OUT_SetCount(&PWM_Tim,Num*Steeps);		//����������������
	}	
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
u8 Motor_Server(void)				//�����������״̬
{
	//SW6 ON��ʾ����ת��صİ�
	//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������
	//CAN�˲�����ʹ�� 0x0A
	//��������CAN-IDΪ0x80
	//��ת������ư�CAN-IDΪ0xC0
	//	u8 MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
	//	u8 SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������	
	//	u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	
	u8 Status	=	0;
	CAN_Server();						//CAN�շ����ݹ���
	Status	=	PWM_OUT_TIMServer(&PWM_Tim);		//��ȡ��ʱ������״̬,����0��ʾ���жϷ���
	
		if(Status	==1)			//��ʾ��ʱ�ж�
		{
//			MotorTime	=	0;		//���������ʱ������
			//=======================����������е�λ
			if(StatusOfWindow	==	0)	//��ԭ��
			{
				if(RxMessage.Data[1]!=0)
				{
					StatusOfWindow	=	1;
					RunToWindow	=	0;
					Motor_RunSet(0);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
					return 2;
				}
			}
			if(RunToWindow==1	&&	StatusOfWindow!=1)
			{
				if(RxMessage.Data[1]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
					return 2;
				}
			}
			else if(RunToWindow==2	&&	StatusOfWindow!=2)
			{
				if(RxMessage.Data[3]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
					return 2;
				}
			}
			else if(RunToWindow==3	&&	StatusOfWindow!=3)
			{
				if(RxMessage.Data[2]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
					return 2;
				}
			}
			else if(RunToWindow==4	&&	StatusOfWindow!=3)
			{
				if(RxMessage.Data[1]!=0)
				{
					StatusOfWindow	=	RunToWindow;
					RunToWindow	=	0;
					Motor_RunSet(0);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
					return 2;
				}
			}
			
			return 1;															//��ʾ��ʱ���ж�
		}
		else if(Status	==2)	//��ʾ�������====������в������
		{
			MotorTime	=	0;		//���������ʱ������
			return 1;																//��ʾ��ʱ���ж�
		}
			
	MotorTime++;
	if(MotorTime>=1000)
	{
		
		MotorTime=0;
	}
	return 0;
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
u8 SetToWindows(u8 nWindows)		//���е�ָ������
{
	//	u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	//	u8 StatusOfWindow	=	0;	//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
	//	u8 CMDOfWindow	=	0;		//���е�ָ����������
	//	#define	Steeps		4000	//һ�������õ�������������
	//	#define	MaxWindow	8							//��󴰿���
	if(RunToWindow	!=0	)				//��������
	{
		return 0;
	}	
	if(nWindows	==0)									//��Чָ��
	{
		Motor_RunSet(0);								//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
		return 0;
	}	
	if(nWindows	>	MaxWindow)					//������󴰿�����
	{
		return 0;	
	}
	if(nWindows	==	StatusOfWindow)		//��ǰ����---����Ҫ��ת
	{
		return 0;
	}
	
	if(nWindows>StatusOfWindow)	//˳ʱ��ת��
	{
		RunToWindow	=	nWindows;		//��Ҫ���е��Ĵ�����
		Motor_RunSet(0-(nWindows-StatusOfWindow)*Steeps);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
	}
	else		//��ת
	{
		RunToWindow	=	nWindows;
		Motor_RunSet((nWindows-StatusOfWindow)*Steeps);				//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
	}
	return 1;
}


/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void Sensor_Read(void)		//���������ź�
{	
	//	u8 SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
	if(Sensor1_Status	==	0)
	{
		if(Sensor[0]<0xFF)
		Sensor[0]++;
	}
	else
	{
		Sensor[0]	=	0;
	}
	if(Sensor2_Status	==	0)
	{
		if(Sensor[1]<0xFF)
		Sensor[1]++;
	}
	else
	{
		Sensor[1]	=	0;
	}
	if(Sensor3_Status	==	0)
	{
		if(Sensor[2]<0xFF)
		Sensor[2]++;
	}
	else
	{
		Sensor[2]	=	0;
	}
	if(Sensor4_Status	==	0)
	{
		if(Sensor[3]<0xFF)
		Sensor[3]++;
	}
	else
	{
		Sensor[3]	=	0;
	}
	
//	if(Sensor[0]>SensorON	||	Sensor[1]>SensorON	||	Sensor[2]>SensorON	||	Sensor[3]>SensorON	)
//	{
//		memset(Sensor,0x00,4);
//		CAN_StdTX_DATA(CANID,4,Sensor);			//CANʹ�ñ�׼֡��������
//	}
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void CAN_Server(void)			//CAN�շ����ݹ���
{
	//SW6 ON��ʾ����ת��صİ�
	//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������
	//CAN�˲�����ʹ�� 0x0A
	//��������CAN-IDΪ0x80
	//��ת������ư�CAN-IDΪ0xC0
	//	u8 MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
	//	u8 SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������
	
	//	u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	//	u8 SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
	u8 status	=	0;		//CAN��ȡ����0��ʾ��Ч
	
	
	status	=	CAN_RX_DATA(&RxMessage);									//���CAN������������
	if(status	==0)
		return;
	if(SensorBD)															//��������
	{
//		Sensor[0]=0x01;
//		Sensor[1]=0x02;
//		Sensor[2]=0x03;
//		Sensor[3]=0x04;
//		CAN_StdTX_DATA(CANID,4,Sensor);				//CANʹ�ñ�׼֡��������
//		status	=	CAN_RX_DATA(&RxMessage);									//���CAN������������
//	if(status	==0)
//		return;
//		memset(RxMessage.Data,0x00,4);
		if(RxMessage.StdId	==	0xC0)							//��ת������ư巢������
		{
			//�������źŲɼ�����
			if(RxMessage.Data[0]==0xAF	&&	RxMessage.Data[1]==0x01)
			{				
				SensorON	=	1;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
			}
			else if(RxMessage.Data[0]==0xAF	&&	RxMessage.Data[1]==0x00)
			{				
				SensorON	=	0;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
			}
			memset(RxMessage.Data,0x00,2);
//			memset(Sensor,0x05,4);
//			CAN_StdTX_DATA(CANID,4,Sensor);				//CANʹ�ñ�׼֡��������
//			if(RxMessage.Data[0]	==	0x0A)					//��ת������ư��ѯ������״̬
//			{	
//				Sensor_Read();		//���������ź�
//				CAN_StdTX_DATA(CANID,4,Sensor);				//CANʹ�ñ�׼֡��������
//				memset(Sensor,0x00,4);
//			}
		}
	}
	else if(MotorBD)				//��ת������ư�
	{
//		status	=	CAN_RX_DATA(&RxMessage);									//���CAN������������
//		if(status	==0)
//			return;
		if(RxMessage.StdId	==	0x80)							//��ת��������������
		{
//			memset(Sensor,0x88,4);
//			memset(RxMessage.Data,0x00,8);
//			CAN_StdTX_DATA(CANID,4,Sensor);				//CANʹ�ñ�׼֡��������
//			if(RxMessage.Data[0]	==	0x0A)					//��ת������ư��ѯ������״̬
//			{				
//				CAN_StdTX_DATA(CANID,4,Sensor);				//CANʹ�ñ�׼֡��������
////				memset(Sensor,0x00,4);
//			}
		}
	}
	else
	{
//		memset(RxMessage.Data,0x00,4);
	}
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void Switch_Server(void)			//�������ַ���ޱ�����������������������в���
{
	
	SWITCHID_Read(&SWITCHID);		//
	
	if(SWITCHID.nSWITCHID	!=	SwitchData)
	{
		Reset_Data();		//��λ���е�ȫ�ֱ���ֵ
		SwitchData	=	SWITCHID.nSWITCHID;
		PC006V21_Configuration();
	}
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	wegam@sina.com
*******************************************************************************/
void Reset_Data(void)		//��λ���е�ȫ�ֱ���ֵ
{
//	SwitchData	=	0;	//�洢���뿪�����µ�ַ�������ַ�仯���������������в���
	SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������
	MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
	MotorTime	=	0;	//��ת�����������ʱ��

//	CANID	=	0;
	SYSTime=0;

	memset(Sensor,0x00,4);	//4����������Ӧֵ�洢��
	SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�

	RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	StatusOfWindow	=	0;	//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
	CMDOfWindow	=	0;		//F
}



#endif