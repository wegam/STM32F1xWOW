
#ifndef __CD4511_H
#define __CD4511_H
//#include <stdint.h>
#include "stm32f10x_gpio.h"


//BCD��ṹ��
typedef	struct	_CD4511_BCD
{
	unsigned char	A:1;
	unsigned char	B:1;
	unsigned char	C:1;
	unsigned char	D:1;
	unsigned char	N:4;		//��ʹ��
}CD4511BcdDef;


typedef struct	_CD4511_Pin
{
	//A/A0
	GPIO_TypeDef* 	CD4511_A0_PORT;				//GPIOX
	unsigned short 	CD4511_A0_Pin;				//GPIO_Pin_x
	
	//B/A1
	GPIO_TypeDef* 	CD4511_A1_PORT;				//GPIOX
	unsigned short 	CD4511_A1_Pin;				//GPIO_Pin_x
	
	//C/A2
	GPIO_TypeDef* 	CD4511_A2_PORT;				//GPIOX
	unsigned short 	CD4511_A2_Pin;				//GPIO_Pin_x
	
	//D/A3
	GPIO_TypeDef* 	CD4511_A3_PORT;				//GPIOX
	unsigned short 	CD4511_A3_Pin;				//GPIO_Pin_x
	
	//ENABLE-PIN
//	GPIO_TypeDef* 	CD4511_EN_PORT;				//GPIOX
//	unsigned short 	CD4511_EN_Pin;				//GPIO_Pin_x

}CD4511_Pindef;


extern CD4511BcdDef*	CD4511Bcd;		//BCD��ṹ��

void CD4511_PinConf(CD4511_Pindef *Pinfo);

void CD4511_Clear(CD4511_Pindef *Pinfo);		//��������bitA~bitG����͵�ƽ
	
void CD4511_WriteData(CD4511_Pindef *Pinfo,u8 num);	//BCDת��ΪSegment��� ֻ���0~9

void CD4511_DisplayOFF(CD4511_Pindef *Pinfo);			//�ر���ʾ---��NPN������
void CD4511_DisplayON(CD4511_Pindef *Pinfo);			//����ʾ---��NPN������










#endif


