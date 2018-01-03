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





#ifdef PC006V21			//�ּ�����ư�

#include "PC006V21.H"

#include	"stdio.h"			//����printf
#include	"string.h"		//����printf
#include	"stdarg.h"		//���ڻ�ȡ��ȷ�������Ĳ���
#include	"stdlib.h"		//malloc��̬�����ڴ�ռ�


#include "STM32_TIM.H"
#include "SWITCHID.H"
#include "STM32_WDG.H"
#include "STM32_EXTI.H"
#include "STM32_GPIO.H"
#include "STM32_PWM.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_CAN.H"
#include "STM32F10x_BitBand.H"


#define	TestModel		//����ģʽ������ģʽ�£�1~4,4~1��ת
//SW6 ON��ʾ����ת��صİ壬�������źŲɼ���/��ת������ư�
//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������


//===============��ת������ư�����ת��ش������źŲɼ���ר��ͨѶ�˲�����
//CAN�˲�����ʹ�� 0x0A
//��������CAN-IDΪ0x80
//��ת������ư�CAN-IDΪ0xC0
#define	MotorBD_Group		0x0A
#define	MotorBD_ID			0xC0
#define	SensorBD_Group	MotorBD_Group
#define	SensorBD_ID			0x80



//===============��ת������ư����ӿ�---���յ����������
//CAN�˲�����0x09
//CAN-IDΪ0xCC			���е�ָ����������0xFA 0x55 nWindows
#define	Cmd_Group		0x09
#define	Cmd_ID			0xCC

//===============��������ͨѶ�˿�
//CAN�˲����� 0x08
//CAN-IDΪ�����ַ
#define	DATA_Group	0x08
#define	DATA_ID			0xFA

#define	CAN1_BaudRate	500000		//CAN������	500K


//=================PA7������ư�CCW���������
#define	MOTOR_Plus_PORT		GPIOA
#define	MOTOR_Plus_Pin		GPIO_Pin_7
//=================PA6������ư�CW���������
#define	MOTOR_DIR_PORT		GPIOA
#define	MOTOR_DIR_Pin			GPIO_Pin_6

#define	MOTOR_PlusHigh		MOTOR_Plus_PORT->BSRR 	= MOTOR_Plus_Pin		//Plus	=	1;	//�ߵ�ƽ
#define	MOTOR_PlusLow			MOTOR_Plus_PORT->BRR 		= MOTOR_Plus_Pin		//Plus	=	0;	//�͵�ƽ
#define	MOTOR_RunRight		MOTOR_DIR_PORT->BSRR 		= MOTOR_DIR_Pin			//EN	=	1;	//˳ʱ��
#define	MOTOR_RunLeft			MOTOR_DIR_PORT->BRR			= MOTOR_DIR_Pin			//EN	=	0;	//��ʱ��

//=================PA6��������CCW���������
#define	TXLED_PORT	GPIOA
#define	TXLED_Pin		GPIO_Pin_7
//=================PA7��������CW���������
#define	RXLED_PORT	GPIOA
#define	RXLED_Pin		GPIO_Pin_6

#define	MOTOR_PWM_Frequency		500			//����Ƶ��--��λHz
#define	PWM_UpdataCount			2			//Ƶ�ʱ仯ռ�����������Ӽ���ʱPWM_Updata����������һ�����Ƶ��
#define	PWM_RunUpCount			500		//����/������Ҫ�Ĳ���
#define	MOTOR_TIMx				TIM1		//��ת�����ʹ�õĶ�ʱ��

#define	SteepPerWindow			2*2030				//һ�������õ�������������
#define	MaxWindow		5					//��󴰿���



//#define	MOTOR_RunRight			MOTOR_DIR_PORT->BRR 	= MOTOR_DIR_Pin		//EN	=	1;	//˳ʱ��
//#define	MOTOR_RunLeft			MOTOR_DIR_PORT->BSRR		= MOTOR_DIR_Pin		//EN	=	0;	//��ʱ��

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
#define	Sensor1_Status	(Sensor1_Port->IDR & Sensor1_Pin)	//������
#define	Sensor2_Status	(Sensor2_Port->IDR & Sensor2_Pin)	//ԭ��
#define	Sensor3_Status	(Sensor3_Port->IDR & Sensor3_Pin)	//�м䴫����
#define	Sensor4_Status	(Sensor4_Port->IDR & Sensor4_Pin)	//������

SWITCHID_CONF SWITCHID;					//���뿪�ؽṹ��
PWM_TimDef		PWM_Tim;					//PWM��������ṹ��

u8 SwitchData	=	0;	//�洢���뿪�����µ�ַ�������ַ�仯���������������в���
u8 SensorBD		=	0;	//���������־��0-�Ǵ������壬1-��������
u8 MotorBD		=	0;	//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
u32	MotorTime	=	0;	//��ת�����������ʱ��

//u8	CANID	=	0;
u16 SYSTime=0;
//u16 data=0;
//u8 Flag=0;
u8 Sensor[4]	=	{0};	//4����������Ӧֵ�洢��
u8 SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�

//=================================��ת�����ر���
u8 PowerFag	=	0;
u8 PlusFlg						=	0;		//������������־��һ�������ؼ�һ������	
u8 RunToWindow				=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
u8 StatusOfWindow			=	0;		//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
u8 CMDOfWindow				=	0;		//���е�ָ����������
u16	RunFrequency			=	0;		//��ǰ����Ƶ��
u32 SteepCount				=	0;		//���в�������
u32 SteepNeedToRun		=	0;		//��Ҫ���е��ܲ���
#ifdef	TestModel
u8	CCWFlag		=	0;
u8	StartTest		=	0;
u32	TestTime	=	0;
#endif

CanRxMsg RxMessage;			//CAN���� 
//CanTxMsg TxMessage;			//CAN����



//========================���ó���
void SWITCHID_Configuration(void);			//���뿪�س�ʼ��������
void MotorBoard_Configuration(void);					//��ת������ư�����
void SensorBoard_Configuration(void);		//��ת���ƴ�����������


//========================��������
void Motor_RunSet(int Num);			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
	
void Reset_Data(void);					//��λ���е�ȫ�ֱ���ֵ
u8 SetToWindows(u8* nWindows);	//���е�ָ������	

void Test_modle(void);					//����ģʽ������ģʽ�£�1~4,4~1��ת	---�궨��TestModel

//========================�������
void SensorBoard_Server(void);		//��������������
void MotorBoard_Server(void);		//������ư�������
u8 Motor_Server(void);				//�����������״̬
void CAN_Server(void);				//CAN�շ����ݹ���
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
	
	SYS_Configuration();				//ϵͳ���� STM32_SYS.H	
	SysTick_DeleymS(500);				//SysTick��ʱnmS
	SysTick_Configuration(1000);		//ϵͳ���ʱ������72MHz,��λΪuS----��ʱɨ��PC006V21_Server
	
	SWITCHID_Configuration();			//���뿪�س�ʼ��������


	if(SensorBD)
	{
		SensorBoard_Configuration();	//��ת���ƴ�����������
	}
	else if(MotorBD)
	{		
		MotorBoard_Configuration();					//��ת������ư�����
	}
	else
	{
		SysTick_Configuration(1000);		//ϵͳ���ʱ������72MHz,��λΪuS----��ʱɨ��PC006V21_Server
	}
	
	PWM_OUT(TIM2,PWM_OUTChannel1,5,900);			//SYS-LED 1HZ 50%
	IWDG_Configuration(1000);	//�������Ź�����	Tout-��ʱ��λʱ�䣬��λms
	
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
//	if(PowerFag	==0)
//	{
//		SYSTime++;
//		if(SYSTime>=5000)
//		{
//			SYSTime	=0;
//			PowerFag=1;
//			SWITCHID_Configuration();									//���뿪�س�ʼ��������

//			if(SensorBD)
//			{
//				Sensor_Configuration();					//����������
//				
//				LED_Configuration();						//��Ϊ��������ʱ������LEDָʾ��
//				CAN_Configuration();						//CAN1����---��־λ��ѯ��ʽ�������ж�--����500K
//				SysTick_Configuration(100);			//ϵͳ���ʱ������72MHz,��λΪuS----��ʱɨ��PC006V21_Server
//			}
//			else if(MotorBD)
//			{
//				
//				Motor_Configuration();					//���������������
//				CAN_Configuration();						//CAN1����---��־λ��ѯ��ʽ�������ж�--����500K
//				SysTick_Configuration(1000);		//ϵͳ���ʱ������72MHz,��λΪuS----��ʱɨ��PC006V21_Server
//			}
//			
//			PWM_OUT(TIM2,PWM_OUTChannel1,1,500);			//SYS-LED 1HZ 50%
//			IWDG_Configuration(1000);	//�������Ź�����	Tout-��ʱ��λʱ�䣬��λms
//	
//		}
//		return;
//	}
	if(SensorBD)
	{
		SensorBoard_Server();		//��������������
	}	
	else if(MotorBD)
	{
		MotorBoard_Server();		//������ư�������
	}
	Switch_Server();			//�������ַ���ޱ�����������������������в���
}
//==============================================================================






/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void SensorBoard_Server(void)		//��������������
{
	//	u8 SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
	u8 status	=	0;			//CAN��ȡ����0��ʾ��Ч
	//1��============================�������Ź�ι��
	IWDG_Feed();				//�������Ź�ι��
	//2��============================��ѯCAN��������
	status	=	CAN_RX_DATA(&RxMessage);									//���CAN������������
	if(status!=0&&RxMessage.StdId==SensorBD_ID)					//CAN���յ������ݲ���IDƥ����ȷ
	{
		//�������źŲɼ�����
		if(RxMessage.Data[0]==0xAF	&&	RxMessage.Data[1]==0x01)
		{				
			SensorON	=	1;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
			SYSTime=0;
		}
		else if(RxMessage.Data[0]==0xAF	&&	RxMessage.Data[1]==0x00)
		{				
			SensorON	=	0;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
			
			//-------------�ر�LEDָʾ��
			TXLED_PORT->BRR=TXLED_Pin;		//PA6��������CCW���������---LED_OFF
			RXLED_PORT->BRR=RXLED_Pin;		//PA7��������CW���������---LED_OFF
		}
		memset(RxMessage.Data,0x00,2);
	}
	//******************************************************************************
	//								���²��ָ�����ת���ư������ѡ���Ƿ�ִ��
	//******************************************************************************
	if(SensorON	==0)			//��������δ�յ���������
	{
		return;
	}
	//3��============================��ʱ��0.1ms
	SYSTime++;
	if(SYSTime>=100000)
	{		
		SYSTime=0;
	}
	//4��============================��ȡ��������
	if((Sensor1_Status==0)	&&	(Sensor[0]<0xFF))
	{
		Sensor[0]++;
	}
	else
	{
		Sensor[0]	=	0;
	}
	if((Sensor2_Status==0)	&&	(Sensor[1]<0xFF))
	{
		Sensor[1]++;
	}
	else
	{
		Sensor[1]	=	0;
	}
	if((Sensor3_Status==0)	&&	(Sensor[2]<0xFF))
	{
		Sensor[2]++;
	}
	else
	{
		Sensor[2]	=	0;
	}
	if((Sensor4_Status==0)	&&	(Sensor[3]<0xFF))
	{
		Sensor[3]++;
	}
	else
	{
		Sensor[3]	=	0;
	}
	//5��============================�ϱ����������ݣ�0.5ms�Ϸ�һ������
	if(SYSTime%5==0)									//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
	{
		CAN_StdTX_DATA(SensorBD_ID,4,Sensor);			//CANʹ�ñ�׼֡��������---���ʹ��������� 0.5ms���ͼ��
	}
	//6��============================���������ڹ���ʱLED��˸
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
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void MotorBoard_Server(void)		//������ư�������
{
	//	u8 SensorON		=	0	;		//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�
	//�Ⱥ�˳��Ҫ�󣺵������ʱ��Ҫ���մ�����������ݣ�������Ҫ�Ȳ�ѯCAN����
	u8 status	=	0;			//CAN��ȡ����0��ʾ��Ч
	
	//1��============================�������Ź�ι��
	IWDG_Feed();				//�������Ź�ι��

	//2��============================��ѯCAN��������
	CAN_Server();					//CAN�շ����ݹ���
	
	//3��============================��ѯ������Ƴ��򣺵�����Ʋ��ö�ʱ���ж�
	Motor_Server();				//��ѯ�������״̬---��������С�ֹͣ����ǰ״̬��ѯ
	
	//4��============================����ģʽ
	Test_modle();					//����ģʽ
	
	//5��============================�������
	RxMessage.StdId	=	0x00;
	memset(RxMessage.Data,0X00,8);
	
	//6��============================��ʱ��1ms
	SYSTime++;
	if(SYSTime>=4000)
	{		
		SYSTime=0;
	}
}
//==============================================================================







/*******************************************************************************
*������			:	MotorBoard_Configuration
*��������		:	��ת������ư�����
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void MotorBoard_Configuration(void)		//��ת������ư�����
{
	//============================GPIO����
	//������ư壺	PA6-���������PA7-����������
	//�������壺		PA6��PA7��������ָʾ�ƣ��ɼ��ź�ʱ��˸��
	//1��============================���������������
	
	TIM_ConfigurationFreq(MOTOR_TIMx,MOTOR_PWM_Frequency);		//��ʱ��Ƶ�����÷�ʽ����СƵ��0.01Hz,���100KHz //���ڷ�ת��Ҫ˫��Ƶ��

	//======���������PA7������ư�CCW���������
	GPIO_Configuration_OPP50	(MOTOR_Plus_PORT,		MOTOR_Plus_Pin);			//CCW	//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
	
	//======������������PA6������ư�CW���������
	GPIO_Configuration_OPP50	(MOTOR_DIR_PORT,		MOTOR_DIR_Pin);			//CW	//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
	
	//3��============================CAN���ã�500K
	CAN_Configuration_NR(CAN1_BaudRate);			//CAN1����---��־λ��ѯ��ʽ�������ж�--����500K
	//4��============================CAN�����˲�������(ʹ�������˲����ֱ��봫������ͨѶ�ͽ����ⲿ����ָ��ͨѶ��
	//-----�������źŶ�ȡ�˲���������ת��������ͨѶ����MotorBD_Group��MotorBD_ID
	CAN_FilterInitConfiguration_StdData(MotorBD_Group,MotorBD_ID,	(MotorBD_ID^SensorBD_ID)^0XFFFF);			//CAN�˲�������---��׼����֡ģʽ---BIT7������
	//-----��ת�����˲�����Cmd_Group��Cmd_ID
	CAN_FilterInitConfiguration_StdData(Cmd_Group,		Cmd_ID,			0xFFFF);			//CAN�˲�������---��׼����֡ģʽ---������ư�����ⲿ��������ӿ�

	//4��============================ϵͳ���ʱ�����ã�����ʱ����ɨ��
	SysTick_Configuration(1000);		//ϵͳ���ʱ������72MHz,��λΪuS----��ʱɨ��PC006V21_Server
}
/*******************************************************************************
*������			:	SensorBoard_Configuration
*��������		:	��ת���ƴ�����������
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void SensorBoard_Configuration(void)		//��ת���ƴ�����������
{
	//============================PA6-CCW,PA7-CW����
	//������ư壺	PA6-���������PA7-����������
	//�������壺		PA6��PA7��������ָʾ�ƣ��ɼ��ź�ʱ��˸��
	
	//1��============================�źŲ�����˸LED����
	GPIO_Configuration_OPP2	(TXLED_PORT,TXLED_Pin);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
	GPIO_Configuration_OPP2	(RXLED_PORT,RXLED_Pin);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�2MHz----V20170605
	
	//2��============================�������ӿ�����:����Ϊ��������ģʽ(����Ч��
	//-----Sensor1-PB4	ǰ������
	//-----Sensor2-PB5	�󴫸���
	//-----Sensor3-PB6	ҩƷ������
	//-----Sensor4-PB7	���ô�����
	GPIO_Configuration_IPU(Sensor1_Port,	Sensor1_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
	GPIO_Configuration_IPU(Sensor2_Port,	Sensor2_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
	GPIO_Configuration_IPU(Sensor3_Port,	Sensor3_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
	GPIO_Configuration_IPU(Sensor4_Port,	Sensor4_Pin);			//��GPIO��Ӧ�ܽ�����Ϊ��������ģʽ----V20170605
		

	//3��============================CAN���ã�500K
	//--------���յ�����ư�����������������źŻ��߹ر�
	CAN_Configuration_NR(CAN1_BaudRate);			//CAN1����---��־λ��ѯ��ʽ�������ж�--����500K
	//4��============================CAN�����˲�������
	//-----�������źŶ�ȡ�˲���������ת������ư�ͨѶ����SensorBD_Group��SensorBD_ID
	CAN_FilterInitConfiguration_StdData(SensorBD_Group,SensorBD_ID,(MotorBD_ID^SensorBD_ID)^0XFFFF);			//CAN�˲�������---��׼����֡ģʽ---BIT7������
	//5��============================ϵͳ���ʱ�����ã�����ʱ����ɨ��
	SysTick_Configuration(100);			//ϵͳ���ʱ������72MHz,��λΪuS----��ʱɨ��PC006V21_Server(0.1msɨ�贫����)
}
//==============================================================================




/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void Test_modle(void)		//����ģʽ
{
#ifdef	TestModel
	if(StatusOfWindow	==	0)
	{
		CCWFlag	=	0;	//���з����־
		return;
	}
	TestTime++;
	if(TestTime>=1000)
	{
		TestTime	=	0;
	}

	if(RunToWindow!=0)
	{
		return;
	}
	else
	{
		CMDOfWindow	=	(TestTime%MaxWindow)+1;
	}
	
	
	
	
//	if(StatusOfWindow	==	0)
//	{
//		CCWFlag	=	0;	//���з����־
//		return;
//	}
//	
//	if(RunToWindow!=0)
//	{
//		TestTime	=	0;
//	}
//	else
//	{
//		TestTime++;
//	}
////	if(RunToWindow!=0)
////	{
////		return;
////	}
//	if(TestTime	>=	500)
//	{
//		TestTime	=	0;
//		if(CCWFlag	==	0)		//��ת
//		{
//			CMDOfWindow	=	StatusOfWindow+1;
//			if(CMDOfWindow	>=	MaxWindow)
//			{
//				CCWFlag	=	1;
//			}
//		}
//		else		//��ת
//		{
//			CMDOfWindow	=	StatusOfWindow-1;
//			if(CMDOfWindow	<=	1)
//			{
//				CCWFlag	=	0;
//			}
//		}
////		SetToWindows(&CMDOfWindow);		//���е�ָ������
//	}
#endif
}
//==============================================================================





/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void SWITCHID_Configuration(void)			//���뿪�س�ʼ��������
{
	//SW6 ON��ʾ����ת��صİ�
	//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������
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
	
	SwitchData	=	(SWITCHID.nSWITCHID)&0x3F;
	
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
*ע��				:	
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
		PWM_OUT_SetFre(&PWM_Tim,10);
		MOTOR_RunRight;													//˳ʱ��
		PWM_OUT_SetCount(&PWM_Tim,Num);		//����������������
	}
	else
	{
//		Motor_Configuration();			//���������������
		PWM_Tim.PWM_BasicData.PWM_Frequency=MOTOR_PWM_Frequency;						//Ƶ������--����
		PWM_OUT_SetFre(&PWM_Tim,10);
		Num	=	0	-	Num;
		MOTOR_RunLeft;													//��ʱ��
		PWM_OUT_SetCount(&PWM_Tim,Num);		//����������������
	}	
}
/*******************************************************************************
*������			:	function
*��������		:	��������˵��
*����				: 
*����ֵ			:	0-�޶�ʱ���жϣ�1-�ж�ʱ���ж�
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
u8 Motor_Server(void)				//��ѯ�������״̬
{
	//SW6 ON��ʾ����ת��صİ�
	//SW5 ON��ʾ��ת������ư壬OFF��ʾ��������
	//CAN�˲�����ʹ�� 0x0A
	//��������CAN-IDΪ0x80
	//��ת������ư�CAN-IDΪ0xC0
	//	u8 MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
	//	u8 SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������	
	//	u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	//	u8 StatusOfWindow	=	0;	//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
	
	//==============�������˵��
	//���й��̷֣��������٣��������У�����ֹͣ
	//����λ�ã�
	//���ԭ��Ϊ�м䴫�������ϵ�ʱ������ʱ��������ת������λ�ٻص�ԭ�㣨����ڻؼ���λ�������ҵ�ԭ�㣬��ֹͣ��ԭ�㣬��ʼ����ɣ�
	
	u8 status	=	0;
	
	
	if((MOTOR_TIMx->SR & TIM_IT_Update)==TIM_IT_Update	&&	RunToWindow!=0)			//��ʾ��ʱ�ж�---���м���δ��
	{
		if(PlusFlg	!=0)			//������������־��һ�������ؼ�һ������
		{
			PlusFlg	=	0;				//������������־��һ�������ؼ�һ������
			SteepCount	+=	1;							//���в�������
			if(RxMessage.StdId==SensorBD_ID&&RxMessage.Data[1]!=0&&RunToWindow==1)
			{
				StatusOfWindow	=	RunToWindow;	//�µĵ��ֹͣ����λ
				RunToWindow			=	0;						//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
				
				MOTOR_PlusLow;		//MOTOR_Plus����رգ��͵�ƽ��
				TIM_Cmd(MOTOR_TIMx, DISABLE); 	//�رն�ʱ��
			}
			else if(SteepCount>=SteepNeedToRun)	//���в�������
			{
				StatusOfWindow	=	RunToWindow;	//�µĵ��ֹͣ����λ
				RunToWindow			=	0;						//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
				
				MOTOR_PlusLow;		//MOTOR_Plus����رգ��͵�ƽ��
				TIM_Cmd(MOTOR_TIMx, DISABLE); 	//�رն�ʱ��
			}
			else
			{
				if(StatusOfWindow	==	0)
				{
					MOTOR_PlusLow;		//MOTOR_Plus����͵�ƽ
				}
				else
				{
					if(SteepCount<PWM_RunUpCount)
					{
						RunFrequency	+=	PWM_UpdataCount;		//��ǰ����Ƶ��
						TIM_SetFreq(MOTOR_TIMx,RunFrequency);		//�趨Ƶ��
					}
					else if(SteepCount+PWM_RunUpCount>=SteepNeedToRun)
					{
						RunFrequency	-=	PWM_UpdataCount;		//��ǰ����Ƶ��
						TIM_SetFreq(MOTOR_TIMx,RunFrequency);		//�趨Ƶ��
					}
					MOTOR_PlusLow;		//MOTOR_Plus����͵�ƽ
				}
			}
		}
		else
		{
			PlusFlg	=	1;				//������������־��һ�������ؼ�һ������
			MOTOR_PlusHigh;		//MOTOR_Plus����ߵ�ƽ
		}
		TIM_ClearITPendingBit(MOTOR_TIMx, TIM_IT_Update);			//����жϱ�־
	}
	
	
	
	if(StatusOfWindow==0	&&	RunToWindow==0)				//����ϵ��ʼ��
	{
		MOTOR_RunLeft;						//��ʱ��
		PlusFlg						=	0;		//������������־��һ�������ؼ�һ������	
		CMDOfWindow				=	1;		//���е�ָ����������
		RunToWindow				=	1;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
		
		SteepCount				=	0;		//���в�������
		SteepNeedToRun		=	MaxWindow*SteepPerWindow+PWM_RunUpCount;				//��Ҫ���е��ܲ���
		RunFrequency			=	2*MOTOR_PWM_Frequency;
		TIM_SetFreq(MOTOR_TIMx,RunFrequency);		//�趨Ƶ��MOTOR_PWM_Frequency
		TIM_Cmd(MOTOR_TIMx, ENABLE); 						//������ʱ��
	}
	else if(StatusOfWindow!=0	&&	RunToWindow==0	&&	CMDOfWindow!=0)		//����ڴ�������״̬�����µĴ�������
	{
		if(CMDOfWindow	==	StatusOfWindow)
		{			
			RunToWindow				=	0;							//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
			CMDOfWindow				=	0;							//���е�ָ����������
			
			TIM_Cmd(MOTOR_TIMx, DISABLE); 			//�رն�ʱ��
		}
		else if(CMDOfWindow	>	StatusOfWindow)
		{
			MOTOR_RunRight;											//˳ʱ��
			PlusFlg						=	0;							//������������־��һ�������ؼ�һ������				
			RunToWindow				=	CMDOfWindow;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
			CMDOfWindow				=	0;		//���е�ָ����������			
			
			SteepCount				=	0;							//���в�������
			if(RunToWindow	==	1)
			{
				SteepNeedToRun		=	(RunToWindow-StatusOfWindow)*SteepPerWindow+PWM_RunUpCount;				//��Ҫ���е��ܲ���
			}
			else
			{
				SteepNeedToRun		=	(RunToWindow-StatusOfWindow)*SteepPerWindow;				//��Ҫ���е��ܲ���
			}
			
			RunFrequency			=	MOTOR_PWM_Frequency;		//��ʼƵ��
			
			TIM_SetFreq(MOTOR_TIMx,RunFrequency);		//�趨Ƶ��
			
			TIM_Cmd(MOTOR_TIMx, ENABLE); 				//������ʱ��
		}
		else if(CMDOfWindow	<	StatusOfWindow)
		{
			MOTOR_RunLeft;											//��ʱ��
			PlusFlg						=	0;							//������������־��һ�������ؼ�һ������				
			RunToWindow				=	CMDOfWindow;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
			CMDOfWindow				=	0;							//���е�ָ����������			
			
			SteepCount				=	0;							//���в�������
			SteepNeedToRun		=	(StatusOfWindow-RunToWindow)*SteepPerWindow;				//��Ҫ���е��ܲ���	
			
			RunFrequency			=	MOTOR_PWM_Frequency;		//��ʼƵ��
			
			TIM_SetFreq(MOTOR_TIMx,RunFrequency);		//�趨Ƶ��
			
			TIM_Cmd(MOTOR_TIMx, ENABLE); 				//������ʱ��			
		}
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
*ע��				:	
*******************************************************************************/
u8 SetToWindows(u8* nWindows)		//���е�ָ������
{
	//	u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	//	u8 StatusOfWindow	=	0;	//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
	//	u8 CMDOfWindow	=	0;		//���е�ָ����������
	//	#define	Steeps		4000	//һ�������õ�������������
	//	#define	MaxWindow	8							//��󴰿���
	
	u8 newWindows	=	*nWindows;
	if(RunToWindow	!=0)				//��������
	{
		return 0;
	}
	if(StatusOfWindow	==	0)					//δ��ʼ��---��Ҫ��ԭ��
	{
		RunToWindow	=	1;											//��Ҫ���е��Ĵ�����
		
		Motor_RunSet(0-(MaxWindow*SteepPerWindow));			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
		
		PWM_OUT_SetFre(&PWM_Tim,MOTOR_PWM_Frequency);
		MOTOR_RunRight;													//˳ʱ��
		PWM_OUT_SetCount(&PWM_Tim,MaxWindow*SteepPerWindow);		//����������������

		return 0;
	}
	if(newWindows	==0	&&	StatusOfWindow!=0)									//��Чָ��
	{
		Motor_RunSet(0);								//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
		return 0;
	}	
	if(newWindows	>	MaxWindow)					//������󴰿�����
	{
		*nWindows	=	0;			//��ָ�����
		return 0;	
	}
	if(newWindows	==	StatusOfWindow)		//��ǰ����---����Ҫ��ת
	{
		*nWindows	=	0;			//��ָ�����
		return 0;
	}
	
	if(newWindows	>	StatusOfWindow)	//˳ʱ��ת��
	{
		RunToWindow	=	newWindows;		//��Ҫ���е��Ĵ�����
//		newWindows	=	newWindows-StatusOfWindow;
		Motor_RunSet(((newWindows-StatusOfWindow)*SteepPerWindow));			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
//		Motor_RunSet(0-((StatusOfWindow-newWindows)*Steeps));				//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
	}
	else		//��ת
	{
		RunToWindow	=	newWindows;
		Motor_RunSet(0-((StatusOfWindow-newWindows)*SteepPerWindow));				//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
//		Motor_RunSet(((newWindows-StatusOfWindow)*Steeps));			//ʹҩ����ת---NumΪ��ֵʱ��ʱ˳ʱ��ת(���������ӽ�)Num����������ֵʱΪ��ʱ��תNum��������0Ϊֹͣ
	}
	*nWindows	=	0;			//��ָ�����
	return 1;
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
	
	if(SensorBD)															//��������---��ת�����ʼ����ʱ��ʼ�ɼ��ź�
	{
		status	=	CAN_RX_DATA(&RxMessage);									//���CAN������������
		if(status	==	0)		//δ���յ�����
		{
			return;
		}
		if(RxMessage.StdId	==	MotorBD_ID)							//��ת������ư巢������
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
		}
	}
	else if(MotorBD)				//��ת������ư�
	{
		//	u8 RunToWindow	=	0;		//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
		status	=	CAN_RX_DATA(&RxMessage);									//���CAN������������
		if(status	!=	0)		//���յ�����
		{
			if(RxMessage.StdId	==	Cmd_ID)							//��ת������ư���յ���������
			{
				if(RxMessage.Data[0]==0xFA	&&	RxMessage.Data[1]==0x55)	//���е�ָ����������
				{
					CMDOfWindow	=	RxMessage.Data[2];
				}	
			}
		}
		
		if(RunToWindow!=0	&&	SensorON!=1)			//�������ת---��Ҫ�������巵�ش������ź�
		{
			Sensor[0]	=	0xAF;
			Sensor[1]	=	0x01;
			SensorON	=	1;
			CAN_StdTX_DATA(MotorBD_ID,2,Sensor);				//CANʹ�ñ�׼֡��������
			memset(Sensor,0x00,2);
		}
		else if(RunToWindow==0	&&	SensorON!=0)				//���ֹͣ��ת---��������ֹͣ�ɼ��ź�
		{
			Sensor[0]	=	0xAF;
			Sensor[1]	=	0x00;
			SensorON	=	0;
			CAN_StdTX_DATA(MotorBD_ID,2,Sensor);				//CANʹ�ñ�׼֡��������
			memset(Sensor,0x00,2);
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
*ע��				:	
*******************************************************************************/
void Switch_Server(void)			//�������ַ���ޱ�����������������������в���
{
	
	SWITCHID_Read(&SWITCHID);		//
	
	if((SWITCHID.nSWITCHID&0x3F)	!=	SwitchData)
	{
		Reset_Data();		//��λ���е�ȫ�ֱ���ֵ
		SwitchData	=	SWITCHID.nSWITCHID&0x3F;
		PC006V21_Configuration();
	}
	else if((SWITCHID.nSWITCHID&0x3F)	==	0x00)
	{
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
*ע��				:	
*******************************************************************************/
void Reset_Data(void)		//��λ���е�ȫ�ֱ���ֵ
{
	SwitchData	=	0;	//�洢���뿪�����µ�ַ�������ַ�仯���������������в���
	SensorBD	=	0;		//���������־��0-�Ǵ������壬1-��������
	MotorBD	=	0;		//��ת������ư��־��0-�ǵ�����ư壬1-������ư�
	MotorTime	=	0;	//��ת�����������ʱ��

	SYSTime=0;

	memset(Sensor,0x00,4);	//4����������Ӧֵ�洢��
	SensorON		=	0	;				//��ת������ư巢�������������ɼ�����BUFFER[0]=0XAF BUFFER[1]=0�أ�=1����0--���ɼ��źţ�1-�ɼ��ź�0.5ms�����ϱ�/������ư��������Ƿ�����������־

	RunToWindow	=	0;				//��ת����Ӧ���� 0-�ޣ�1-1�ţ�2-2�ţ�3-3�ţ�4-4��
	StatusOfWindow	=	0;		//��ʱֹͣλ�ã�0-δ��ʼ����1-ԭ�㣬2-2�Ŵ��ڣ�3-3�Ŵ��ڣ�4-4�Ŵ���
	CMDOfWindow	=	0;				//���е�ָ����������
}



#endif