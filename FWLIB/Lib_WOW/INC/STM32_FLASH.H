/******************************** User_library *********************************
* �ļ��� 	: STM32_SDCard.H
* ����   	: wegam@sina.com
* �汾   	: V
* ����   	: 2016/01/01
* ˵��   	: 
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/


#ifndef __STM32_FLASH_H
#define __STM32_FLASH_H

#include "stm32f10x_type.h"
//#include "stm32f10x_gpio.h"
//#include "stm32f10x_dac.h"
//#include "stm32f10x_rcc.h"
//#include "stm32f10x_dma.h"
//#include "stm32f10x_tim.h"
//���洢�������� 
//С������Ʒ���洢�����Ϊ4K��64λ��ÿ���洢�黮��Ϊ32��1K�ֽڵ�ҳ�� 
//��������Ʒ���洢�����Ϊ16K��64λ��ÿ���洢�黮��Ϊ128��1K�ֽڵ�ҳ�� 
//��������Ʒ���洢�����Ϊ64K��64λ��ÿ���洢�黮��Ϊ256��2K�ֽڵ�ҳ�� 
//�����Ͳ�Ʒ���洢�����Ϊ32K��64λ��ÿ���洢�黮��Ϊ128��2K�ֽڵ�ҳ��
//�û������Լ�����Ҫ����
#define STM32_FLASH_SIZE 128 	 		//��ѡSTM32��FLASH������С(��λΪK)---�洢���С128K
#define STM32_FLASH_WREN 1        //ʹ��FLASHд��(0��������;1��ʹ��)

#if STM32_FLASH_SIZE<256					//С����
#define STM_SECTOR_SIZE 1024			//�ֽڵ�λ-----������С/ҳ��С
#else 
#define STM_SECTOR_SIZE	2048			//�ֽڵ�λ-----������С/ҳ��С
#endif



//FLASH��ʼ��ַ
#define STM32_FLASH_BASE	0x08000000 	//STM32 FLASH����ʼ��ַ





u16 STM32_FLASH_ReadHalfWord(u32 Addr);		//��ȡָ����ַ�İ���(16λ����)
void STM32_FLASH_Write_NoCheck(u32 StartAddr,u16 *pBuffer,u16 NumToWrite); 	//������д���������
void STM32_FLASH_Write(u32 StartAddr,u16 *pBuffer,u16 NumToWrite);		//��ָ����ַд��ָ�����ȵ�����
void STM32_FLASH_Read(u32 StartAddr,u16 *pBuffer,u16 NumToRead);			//��ָ����ַ��ʼ����ָ�����ȵ�����
void STM32_FLASH_Erase(u32 WriteAddr,u16 NumToWrite);			//����FLASH

#endif



