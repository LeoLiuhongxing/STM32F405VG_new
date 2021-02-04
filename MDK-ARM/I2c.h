/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         IIC driver
    Description:    I2c.h
    =====================================================================================
*/
#ifndef _I2C_H
#define _I2C_H
#include "types.h"

#define SDA_PIN 7
#define SCL_PIN 6

#define SDA_IN()  {GPIOB->MODER&=~(3<<(SDA_PIN*2));GPIOB->MODER|=0<<SDA_PIN*2;}	//input mode
#define SDA_OUT() {GPIOB->MODER&=~(3<<(SDA_PIN*2));GPIOB->MODER|=1<<SDA_PIN*2;} //output mode
//
#define IIC_SCL   PBout(SCL_PIN) //SCL
#define IIC_SDA   PBout(SDA_PIN) //SDA
#define READ_SDA  PBin(SDA_PIN)  //??SDA


void IIC_Init(void);                				 
void IIC_Start(void);				
void IIC_Stop(void);	  			
void IIC_Send_Byte(u8 txd);			
u8 IIC_Read_Byte(unsigned char ack);
u8 IIC_Wait_Ack(void); 				
void IIC_Ack(void);					
void IIC_NAck(void);				

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);	 
#endif
