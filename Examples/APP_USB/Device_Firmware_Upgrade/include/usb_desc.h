/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_desc.h
* Author             : MCD Application Team
* Version            : V2.2.0
* Date               : 06/13/2008
* Description        : Descriptor Header for Device Firmware Upgrade (DFU)
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_DESC_H
#define __USB_DESC_H
#include "platform_config.h"
/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DFU_SIZ_DEVICE_DESC            18

#ifdef USE_STM3210B_EVAL
  #define DFU_SIZ_CONFIG_DESC          36
#else /* USE_STM3210E_EVAL */
  #define DFU_SIZ_CONFIG_DESC          45
#endif /* USE_STM3210B_EVAL */

#define DFU_SIZ_STRING_LANGID           4
#define DFU_SIZ_STRING_VENDOR           38
#define DFU_SIZ_STRING_PRODUCT          20
#define DFU_SIZ_STRING_SERIAL           26
#define DFU_SIZ_STRING_INTERFACE0       96    /* Flash Bank 0 */

#define DFU_SIZ_STRING_INTERFACE1       98     /* SPI Flash : M25P64*/
#define DFU_SIZ_STRING_INTERFACE2       106    /* NOR Flash : M26M128*/

extern  u8 DFU_DeviceDescriptor[DFU_SIZ_DEVICE_DESC];
extern  u8 DFU_ConfigDescriptor[DFU_SIZ_CONFIG_DESC];
extern  u8 DFU_StringLangId     [DFU_SIZ_STRING_LANGID];
extern  u8 DFU_StringVendor     [DFU_SIZ_STRING_VENDOR];
extern  u8 DFU_StringProduct    [DFU_SIZ_STRING_PRODUCT];
extern  u8 DFU_StringSerial     [DFU_SIZ_STRING_SERIAL];
extern  u8 DFU_StringInterface0 [DFU_SIZ_STRING_INTERFACE0];
extern  u8 DFU_StringInterface1 [DFU_SIZ_STRING_INTERFACE1];
extern  u8 DFU_StringInterface2_1 [DFU_SIZ_STRING_INTERFACE2];
extern  u8 DFU_StringInterface2_2 [DFU_SIZ_STRING_INTERFACE2];
extern  u8 DFU_StringInterface2_3 [DFU_SIZ_STRING_INTERFACE2];

#define bMaxPacketSize0             0x40     /* bMaxPacketSize0 = 64 bytes   */
#define wTransferSize               0x0400   /* wTransferSize   = 1024 bytes */
/* bMaxPacketSize0 <= wTransferSize <= 32kbytes */
#define wTransferSizeB0             0x00
#define wTransferSizeB1             0x04
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* External variables --------------------------------------------------------*/

#endif /* __USB_DESC_H */

/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/




