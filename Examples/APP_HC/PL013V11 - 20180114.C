#ifdef PL013V11

#include "PL013V11.H"
#include "CD4511.H"



#include "SWITCHID.H"
#include "STM32F10x_BitBand.H"
#include "STM32_GPIO.H"
#include "STM32_SYS.H"
#include "STM32_SYSTICK.H"
#include "STM32_WDG.H"
#include "STM32_PWM.H"



#include "STM32_USART.H"
#include "STM32_TOOLS.H"		//����У�鹤��


#include "string.h"				//�����ڴ��������ͷ�ļ�



//************485ͨѶ����


SWITCHID_CONF	SWITCHID;			//���뿪��
RS485_TypeDef	RS485_Bus;				//����ͨѶ485
sReceFarmeDef	sReceFarmeRxd;				//���սṹ��---���ջ�����
sReceFarmeDef	sReceFarmeRev;				//���սṹ��---���ݻ�����
sReceFarmeDef	sReceFarmeSrv;				//���սṹ��---������������

CmdDef				DspCmd;					//��ʾ����

//volatile	CD4511_Pindef CD4511_Pin1;		//��һλ
//volatile	CD4511_Pindef CD4511_Pin2;		//�ڶ�λ
//volatile	CD4511_Pindef CD4511_Pin3;		//����λ




u8 Bus485Rev[Bus485DataSize]={0};
u8 Bus485Rxd[Bus485DataSize]={0};
u8 Bus485Txd[Bus485DataSize]={0};


u8 SwitchID=0;				//���뿪�ص�ַ //��Ԫ��ID----���ҵ�������Ϊ���ߣ����ұ�Ϊ���λ�����λΪ1��ʾ����ģʽ������ģʽ�´�0��9ѭ����ʾ

vu8	LongDsp		=	0;			//0--����ʾʱ������ʾ��1-����,������ʾ����ʱ
vu32	DSPTIME		=	0;			//��ʾ��ʱ��--��λmS
vu32	DSPCount	=	0;			//��ʾ��ʱ--��λmS
vu32	SYSTIME		=	0;			//ѭ����ʱ����
vu32	DSPNum		=	0;			//��ʾ����







/*******************************************************************************
* ������		:	
* ��������	:	 
* ����		:	
* ���		:
* ���� 		:
*******************************************************************************/
void PL013V11_Configuration(void)
{
	
	SYS_Configuration();					//ϵͳ����---��ϵͳʱ�� STM32_SYS.H
	
	GPIO_DeInitAll();							//�����е�GPIO�ر�----V20170605
	
	DeleyuS(1);										//��ʱ1S		
	
	SWITCHID_Configuration();			//���뿪�س�ʼ��������
	
	CD4511_Configuration();				//���������		
	
	RS485_Configuration();				//RS485����
	
	PWM_OUT(TIM2,PWM_OUTChannel1,1,900);	//PWM�趨-20161127�汾
	
	IWDG_Configuration(1000);							//�������Ź�����---������λms
	
	SysTick_Configuration(200);					//ϵͳ���ʱ������72MHz,��λΪuS
	
	CD4511_DisplayClear();		//�����ʾ
}
/*******************************************************************************
* ������		:	
* ��������	:	 
* ����		:	
* ���		:
* ���� 		:
*******************************************************************************/
void PL013V11_Server(void)
{
	//ѭ������1mS
//	u8 status=0;	
	
	IWDG_Feed();											//�������Ź�ι��	
	SYSTIME++;	
	if(SYSTIME>=10000)	//2��
	{
		SYSTIME=0;	
		SwitchIdServer();		//������ֵ���ޱ仯���������ID
	}
	
	RS485_Server();										//RS485��������
	
	DataServer();			//�������յ�����Ч����
	if(SYSTIME%5	==	0)	//1ms
	DisplayServer();	//��ʾ������򣬸�����ʾ����ˢ��
	
	
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
void SWITCHID_Configuration(void)			//���뿪�س�ʼ��������
{
	SWITCHID.NumOfSW	=	6;
	//SW1--PA4
	SWITCHID.SW1_PORT	=	GPIOB;
	SWITCHID.SW1_Pin	=	GPIO_Pin_8;
	//SW2--PA5
	SWITCHID.SW2_PORT	=	GPIOB;
	SWITCHID.SW2_Pin	=	GPIO_Pin_9;
	//SW3--PB12
	SWITCHID.SW3_PORT	=	GPIOB;
	SWITCHID.SW3_Pin	=	GPIO_Pin_10;
	//SW4--PB13
	SWITCHID.SW4_PORT	=	GPIOB;
	SWITCHID.SW4_Pin	=	GPIO_Pin_11;
	//SW5--PB14
	SWITCHID.SW5_PORT	=	GPIOB;
	SWITCHID.SW5_Pin	=	GPIO_Pin_12;
	//SW6--PB15
	SWITCHID.SW6_PORT	=	GPIOB;
	SWITCHID.SW6_Pin	=	GPIO_Pin_13;

	
	SWITCHID_Conf(&SWITCHID);		//
	SWITCHID_Read(&SWITCHID);		//
	
	SwitchID	=	(SWITCHID.nSWITCHID)&0x3F;
}
/*******************************************************************************
* ������			:	RS485_Configuration
* ��������		:	��������˵�� 
* ����			: void
* ����ֵ			: void
* �޸�ʱ��		: ��
* �޸�����		: ��
* ����			: 
*******************************************************************************/
void RS485_Configuration(void)			//RS485����
{
	//============================����ͨѶ485
	RS485_Bus.USARTx	=	Bus485Port;	
	RS485_Bus.RS485_CTL_PORT	=	Bus485CtlPort;
	RS485_Bus.RS485_CTL_Pin		=	Bus485CtlPin;
	
	RS485_DMA_ConfigurationNR	(&RS485_Bus,Bus485BaudRate,(u32*)&sReceFarmeRxd,Bus485DataSize);	//USART_DMA����--��ѯ��ʽ�������ж�,������Ĭ��Ϊ����״̬
}
/*******************************************************************************
* ������			:	RS485_Configuration
* ��������		:	��������˵�� 
* ����			: void
* ����ֵ			: void
* �޸�ʱ��		: ��
* �޸�����		: ��
* ����			: 
*******************************************************************************/
void CD4511_Configuration(void)			//���������
{
	//С������ƽ�����
	GPIO_Configuration_OPP50	(DPPort,	DPPin);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	GPIO_ResetBits(DPPort,	DPPin);
	
	GPIO_Configuration_OPP50	(PortA0,PinA0);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	GPIO_Configuration_OPP50	(PortA1,PinA1);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	GPIO_Configuration_OPP50	(PortA2,PinA2);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	GPIO_Configuration_OPP50	(PortA3,PinA3);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	
	GPIO_Configuration_OPP50	(PortEN1,PinEN1);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	GPIO_Configuration_OPP50	(PortEN2,PinEN2);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605
	GPIO_Configuration_OPP50	(PortEN3,PinEN3);			//��GPIO��Ӧ�ܽ�����ΪPP(����)���ģʽ������ٶ�50MHz----V20170605	
	
}

/*******************************************************************************
*������		:	function
*��������	:	��������˵��
*����			: 
*���			:	��
*����ֵ		:	��
*����			:
*******************************************************************************/
void CD4511_DISPALY(u8 wei,u16 num)
{
	u32 Num	=	0;
	GPIO_ResetBits(PortEN1,PinEN1);
	GPIO_ResetBits(PortEN2,PinEN2);
	GPIO_ResetBits(PortEN3,PinEN3);
	
	GPIO_ResetBits(DPPort,DPPin);
	
	Num	=	0;
	while(Num++<1000);
	
	
	GPIO_ResetBits(PortA0,PinA0);
//	GPIO_ResetBits(PortA1,PinA1);
	GPIO_ResetBits(PortA2,PinA2);
//	GPIO_ResetBits(PortA3,PinA3);
	
	GPIO_SetBits(PortA1,PinA1);
	GPIO_SetBits(PortA3,PinA3);
	
	Num	=	0;
	while(Num++<1000);
	
	if(wei==0)		//С����
	{
		if(num==0)		//С����
		{
			GPIO_SetBits(PortA1,PinA1);
			GPIO_SetBits(PortA3,PinA3);
			GPIO_SetBits(DPPort,DPPin);
			GPIO_SetBits(PortEN1,PinEN1);
		}
		else			//����ʾ
		{
			GPIO_ResetBits(PortEN1,PinEN1);
			GPIO_ResetBits(PortEN2,PinEN2);
			GPIO_ResetBits(PortEN3,PinEN3);
			GPIO_SetBits(PortA1,PinA1);
			GPIO_SetBits(PortA3,PinA3);
			GPIO_ResetBits(DPPort,DPPin);			
		}
		return;
	}
	else if(wei==1)
	{
		num	=	num%10;
	}
	else if(wei==2)
	{
		num	=	(num%100)/10;
	}
	else if(wei==3)
	{
		num	=	(num%1000)/100;
	}
	
//	//A0
	if((num&0x01)==0x01)
	{
		GPIO_SetBits(PortA0,PinA0);
	}
	else
	{
		GPIO_ResetBits(PortA0,PinA0);
	}
	//A1
	if((num&0x02)==0x02)
	{
		GPIO_SetBits(PortA1,PinA1);
	}
	else
	{
		GPIO_ResetBits(PortA1,PinA1);
	}
	//A2
	if((num&0x04)==0x04)
	{
		GPIO_SetBits(PortA2,PinA2);
	}
	else
	{
		GPIO_ResetBits(PortA2,PinA2);
	}
	//A3
	if((num&0x08)==0x08)
	{
		GPIO_SetBits(PortA3,PinA3);
	}
	else
	{
		GPIO_ResetBits(PortA3,PinA3);
	}

	
	
	Num	=	0;
	while(Num++<1000);
		
	if(wei==3)		//��λ
	{
		GPIO_SetBits(PortEN3,PinEN3);
	}
	else if(wei==2)	//ʮλ
	{
		GPIO_SetBits(PortEN2,PinEN2);
	}
	else if(wei==1)	//��λ
	{
		GPIO_SetBits(PortEN1,PinEN1);
	}
	else			//����ʾ
	{
		GPIO_ResetBits(PortEN1,PinEN1);
		GPIO_ResetBits(PortEN2,PinEN2);
		GPIO_ResetBits(PortEN3,PinEN3);
	}


}
/*******************************************************************************
*������		:	function
*��������	:	��������˵��
*����			: 
*���			:	��
*����ֵ		:	��
*����			:
*******************************************************************************/
void CD4511_DisplayClear(void)		//�����ʾ
{
	CD4511_DISPALY(0,1);
}
/*******************************************************************************
*������		:	function
*��������	:	��������˵��
*����			: 
*���			:	��
*����ֵ		:	��
*����			:
*******************************************************************************/
void CD4511_DisplayDp(void)		//��λ��ʾС����
{
	CD4511_DISPALY(0,0);				//��λ��ʾС����
}
/*******************************************************************************
*������		:	function
*��������	:	��������˵��
*����			: 
*���			:	��
*����ֵ		:	��
*����			:
*******************************************************************************/
void RS485_Server(void)
{
	u8	Num	=	0;
	Num	=	RS485_ReadBufferIDLE(&RS485_Bus,(u32*)&sReceFarmeRev,(u32*)&sReceFarmeRxd);
	if(Num	!=	0)	//������
	{
		if(sReceFarmeRev.desAddr	==	SwitchID)
		{
			memcpy((u8*)&sReceFarmeSrv,(u8*)&sReceFarmeRev,sizeof(sReceFarmeDef));
//			DataServer();		//�������յ�����Ч����
		}
//		else
//		{
//			memset((u8*)&sReceFarmeRev,0x00,Bus485DataSize);	//������ݻ�����
//		}
	}
	
}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void DataServer(void)		//�������յ�����Ч����
{
	if(SwitchID==0	||	(SwitchID&0x20)==0x20)		//δ����
	{
		return;
	}
	if(sReceFarmeSrv.desAddr	==	SwitchID)			//��ַ�����Ѽ��룬��ʾ���µ���Ч����
	{
		
		u32	Num	=	0;
		DSPCount	=	0;
		
		sReceFarmeSrv.desAddr	=0;		//ɾ����ַ��־
		
		DspCmd	=	sReceFarmeSrv.cmd;			//��ʾ����
		
		//==============��ȡ��ʾ��ֵ
		Num	|=	sReceFarmeSrv.data[0];
		Num<<=8;
		Num	|=	sReceFarmeSrv.data[1];
		Num<<=8;
		Num	|=	sReceFarmeSrv.data[2];
		Num<<=8;
		Num	|=	sReceFarmeSrv.data[3];
		DSPNum	=		Num;		//

		//==============��ȡ��ʾʱ��
		if(sReceFarmeSrv.cmd.DispTime	==0)
		{
			LongDsp	=	1;			//0--����ʾʱ������ʾ��1-����,������ʾ����ʱ
		}
		else
		{
			LongDsp	=	0;			//0--����ʾʱ������ʾ��1-����,������ʾ����ʱ
			Num	=	0;
			Num	|=	sReceFarmeSrv.data[4];
			Num<<=8;
			Num	|=	sReceFarmeSrv.data[5];
			Num<<=8;
			Num	|=	sReceFarmeSrv.data[6];
			Num<<=8;
			Num	|=	sReceFarmeSrv.data[7];
			DSPTIME	=		Num;		//
		}
		//==============����ȡ�꣬�������
//		memset((u8*)&sReceFarmeSrv,0x00,Bus485DataSize);	//������ݻ�����
	}
}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void DisplayServer(void)		//��ʾ������򣬸�����ʾ����ˢ��
{
//	DSPNum	=	1;
//	CD4511_DISPALY((u8)SYSTIME%3+1,(u16)DSPNum);		//ˢ������
//	return;
//	CD4511_DisplayOFF(&CD4511_Pin1);			//�ر���ʾ---��NPN������
//	CD4511_DisplayOFF(&CD4511_Pin2);			//�ر���ʾ---��NPN������
//	CD4511_DisplayOFF(&CD4511_Pin3);			//�ر���ʾ---��NPN������
	
	if(SwitchID==0	||	(SwitchID&0x20)==0x20)		//δ�������Ϊ����ģʽsw6Ϊ1��ʾ����ģʽ
	{
		DSPCount++;
		if(DSPCount>=900)
		{
			DSPCount	=	0;			
		}
		if(DSPCount%300	==	0)
		{
			u8	Num	=	0;
			DSPNum	=	DSPNum%10;
			if(DSPNum	==	9)
			{
				DSPNum	=	0;
			}
			else
			{
				DSPNum++;
				Num	=	DSPNum;
			}
			DSPNum	=	Num*100+Num*10+Num;
		}
		CD4511_DISPALY((u8)DSPCount%3+1,DSPNum);		//ˢ������
		GPIO_ResetBits(DPPort,DPPin);
		return;
	}
	else if(LongDsp	==	1)				//0--����ʾʱ������ʾ��1-����,������ʾ����ʱ����ʱС���㲻��ʾ
	{
		DSPCount++;
		if(DSPCount>=1000)
		{
			DSPCount	=	0;
		}
		if(DspCmd.DispEnNum)		//��ʾ��ֵ	��	0-����ʾ��		1-��ʾ����ʱС���㲻��ʾ
		{
//			CD4511_DISPALY((u8)DSPCount%3+1,DSPNum);		//ˢ������
			if(DspCmd.DispMdNum)	//��ֵģʽ	��	0-��̬��ʾ��	1-0.5S��˸
			{
				if((DSPCount%1000)>500)
				{
					CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//ˢ������
				}
				else
				{
					CD4511_DISPALY(0,0);						//����ʾ
				}
			}
			else
			{
				CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//ˢ������
			}
		}
		else if(DspCmd.DispEnDp)		//��ʾ��		��	0-����ʾ��		1-��ʾ
		{
			if(DspCmd.DispMdDp)			//��ģʽ		��	0-��̬��ʾ��	1-0.5S��˸
			{
				if((DSPCount%1000)<500)
				{
					CD4511_DISPALY(0,1);		//��ʾС����
				}
				else
				{
					CD4511_DISPALY(0,0);		//�ر�С����
				}
			}
			else
			{
				CD4511_DisplayDp();		//��λ��ʾС����
			}
		}
	}
	else
	{
		if(DSPTIME	==0)
		{
			return;
		}
		if(DSPCount>=DSPTIME)		//��ʾʱ�������꣬��Ҫ�ر���ʾ
		{
			DSPTIME	=	0;
			DSPCount=	0;
			CD4511_DisplayClear();		//�����ʾ
		}
		if((DspCmd.DispEnNum)&&DSPTIME)		//��ʾ��ֵ	��	0-����ʾ��		1-��ʾ����ʱС���㲻��ʾ
		{
			DSPCount	++;
			if(DspCmd.DispMdNum)	//��ֵģʽ	��	0-��̬��ʾ��	1-0.5S��˸
			{
				if((DSPCount%1000)>500)
				{
					CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//ˢ������
				}
				else
				{
					CD4511_DISPALY(0,0);						//����ʾ
				}
			}
			else		//����˸
			{
				CD4511_DISPALY((u8)DSPCount%3+1,(u16)DSPNum);		//ˢ������
			}
		}
		else if(DspCmd.DispEnDp==1)		//��ʾ��		��	0-����ʾ��		1-��ʾ
		{
			if(DspCmd.DispMdDp	&&	DSPCount%500==0)				//��ģʽ		��	0-��̬��ʾ��	1-0.5S��˸
			{
				GPIO_Toggle	(DPPort,	DPPin);		//��GPIO��Ӧ�ܽ������ת----V20170605
			}
		}
	}	
}
/*******************************************************************************
*������			:	function
*��������		:	function
*����				: 
*����ֵ			:	��
*�޸�ʱ��		:	��
*�޸�˵��		:	��
*ע��				:	
*******************************************************************************/
void SwitchIdServer(void)		//������ֵ���ޱ仯
{
	SWITCHID_Read(&SWITCHID);		//
	if(SwitchID	!=	(SWITCHID.nSWITCHID)&0x3F)
	{
		SwitchID	=	(SWITCHID.nSWITCHID)&0x3F;
		CD4511_DisplayClear();		//�����ʾ
	}
}

#endif