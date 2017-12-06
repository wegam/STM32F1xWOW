#ifndef __DS2401_H
#define __DS2401_H 
#include "stdint.h"





uint8_t CRC8_Calculate(uint8_t *pBuf, uint8_t len);
void Dallas_Rst(void);
uint8_t Dallas_Check(void);
uint8_t Dallas_ReadBit(void);
uint8_t Dallas_ReadByte(void);
void Dallas_WriteByte(uint8_t dat);
uint8_t Dallas_Init(void);
uint8_t Dallas_GetID(uint8_t *pBuf);

    
#endif

