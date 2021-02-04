
#ifndef __24CXX_H
#define __24CXX_H

#include "main.h"
#include "types.h"
#include "stm32f4xx_hal_gpio.h"

#define SDA_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET)
#define SDA_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET)
#define READ_SDA  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) //GPIO_ReadInputDataBit(GPIOC, 5)//GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5)
#define SCL_1 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET)//GPIO_SetBits(GPIOC, GPIO_Pin_4)
#define SCL_0 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET)//GPIO_ResetBits(GPIOC, GPIO_Pin_4)

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
u8 IIC_Wait_Ack(void);
void IIC_Ack(void);
void IIC_NAck(void);
void IIC_Send_Byte(u8 txd);
u8 IIC_Read_Byte(unsigned char ack);
u8 AT24CXX_ReadOneByte(u16 ReadAddr);
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);
u8 AT24CXX_Check(void);
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);
void SDA_OUT(void);
void SDA_IN(void);
void test_time(void);
u8 test_read_write(void);
u8 test_read_write_bytes(void);
void EraseAll(void);

#endif /*__24CXX_H*/
