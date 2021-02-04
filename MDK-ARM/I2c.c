/*
    =====================================================================================
    Processor:      STM32F405VGT
    Author:         Leo Liu
    Date:           12/2011
    Module:         types.h
    Description:    This header contains global types and definitions
    =====================================================================================
*/
#include "stm32f4xx_hal.h"
#include "types.h"
#include "I2c.h"
#include "delay.h"
//PB6 SCL
//PB7 SDA
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();   //GPIOB CLOCK enable
    
    //
    GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //???????
    GPIO_Initure.Pull=GPIO_PULLUP;          //????
    GPIO_Initure.Speed=GPIO_SPEED_FAST;     //????
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    
    IIC_SDA=1;
    IIC_SCL=1;  
}

//????IIC??'?z?
void IIC_Start(void)
{
	SDA_OUT();     //sda?????
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//??I2C???????????????????? 
}	


//????IIC???z?
void IIC_Stop(void)
{
	SDA_OUT();//sda?????
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//????I2C????????z?
	delay_us(4);							   	
}
//???????zw???
//???????1????????????
//        0???????????
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA?????????  
	IIC_SDA=1;delay_us(1);	   
	IIC_SCL=1;delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//??????0 	   
	return 0;  
} 
//????ACK???
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}
//??????ACK???		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}					 				     
//IIC????h?????
//?????????????
//1???????
//0???????			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//????????'???????
    for(t=0;t<8;t++)
    {              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //??TEA5767?????????????????
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
//??1??????ack=1???????ACK??ack=0??????nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA?????????
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(2);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//????nACK
    else
        IIC_Ack(); //????ACK   
    return receive;
}
