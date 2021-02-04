/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         24cxx.h
    Description:    This header contains global types and definitions
    =====================================================================================
*/

#include "24cxx.h"
#include "delay.h"


#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767  
#define AT24C512	65535  

#define EE_TYPE AT24C64 

//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
//IIC初始化
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();   //使能GPIOB时钟
    
    //PH4,5初始化设置
    GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FAST;     //快速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    
    SDA_1;
    SCL_1;;  
}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	SDA_1;	  	  
	SCL_1;
	delay_2_us();delay_2_us();//delay_us(4);
 	SDA_0;//START:when CLK is high,DATA change form high to low 
	delay_2_us();delay_2_us();//delay_us(4);
	SCL_0;//钳住I2C总线，准备发送或接收数据 
	delay_2_us();delay_2_us();//delay_us(4);
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	SCL_0;
	SDA_0;//STOP:when CLK is high DATA change form low to high
	delay_2_us();delay_2_us();//delay_us(4);
	SCL_1;
    SDA_1;//发送I2C总线结束信号    
	delay_2_us();delay_2_us();//delay_us(4);
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	SDA_1;delay_1_us();//delay_us(1);	   
	SCL_1;delay_1_us();//delay_us(1);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	SCL_0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	SCL_0;
	SDA_OUT();
	SDA_0;
	delay_2_us();//delay_us(2);
	SCL_1;
	delay_2_us();//delay_us(2);
	SCL_0;
	//delay_us(2);
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	SCL_0;
	SDA_OUT();
    SDA_1;
	delay_2_us();//delay_us(2);
	SCL_1;
	delay_2_us();//delay_us(2);
	SCL_0;
//	delay_us(2);
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    SCL_0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        if((txd&0x80)>>7==1)
			SDA_1;
		else
			SDA_0;
        txd<<=1; 	  
		delay_2_us();//delay_us(2);   //对TEA5767这三个延时都是必须的
		SCL_1;
		delay_2_us();//delay_us(2); 
		SCL_0;	
		delay_2_us();//delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
   	{
        SCL_0; 
        delay_2_us();//delay_us(2);
	    SCL_1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_1_us();//delay_us(1); 
     }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}

//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址  
//返回值  :读到的数据
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;    
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//发送高地址	 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据 	 

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //发送低地址
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //进入接收模式			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//产生一个停止条件	
	return temp;
}
//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址    
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    u8 i = 0;
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //发送写命令
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//发送高地址
 	}else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据 
	}	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr%256);   //发送低地址
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //发送字节							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();//产生一个停止条件 
	for(i =0; i<10;i++)
        delay_1_ms();//delay_ms(10);	 
}
//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址  
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
} 
//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址 
//返回值     :数据
//Len        :要读出数据的长度2,4
//u32 Leo_Liu=0;
//u32 Leo_add = 0;
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{  	
	u8 t[4] = {0};
	u32 temp=0;
#if 0
	for(t=0;t<Len;t++)
	{
        temp<<=8*t;
		temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1); 
	}
#else
    for(u8 i=0;i<Len;i++)
    {
        t[i] = AT24CXX_ReadOneByte(ReadAddr+i);
    }
    if(Len == 2)
    {
        temp = t[1];
        temp <<=8;
        temp += t[0];
    }else if(Len == 4)
    {
        temp = t[3];
        temp <<=8;
        temp += t[2];
        temp <<=8;
        temp += t[1];
        temp <<=8;
        temp += t[0];        
    }
    
#endif    
	return temp;												    
}
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
#define LAST_ADDRESS 0xFFFF
u8 AT24CXX_Check(void)
{
    u8 temp;
    temp=AT24CXX_ReadOneByte(LAST_ADDRESS);//避免每次开机都写AT24CXX			   
    if(temp==0X55)
        return 0;		   
    else//排除第一次初始化的情况
    {
        AT24CXX_WriteOneByte(LAST_ADDRESS,0X55);
        temp=AT24CXX_ReadOneByte(LAST_ADDRESS);	  
        if(temp==0X55)
            return 0;
    }
    return 1;    
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
	while(NumToWrite--)
	{
		AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
		WriteAddr++;
		pBuffer++;
	}
}

void EraseAll(void)
{
    u8 temp = 0xFF;
    u16 addr = 0;
    for(u8 i=0;i<8;i++)
    {    
        for(u16 j=0;j<1024;j++)
        {
            addr = i*1024+j;
            AT24CXX_WriteOneByte(addr,temp); 
        }
    }
    
}

void SDA_OUT(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
	
		GPIO_InitStructure.Pin = GPIO_PIN_7;
		GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;//GPIO_Mode_OUT;
		//GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
		GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_HIGH;
		GPIO_InitStructure.Pull = GPIO_PULLUP;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
    
}
void SDA_IN(void)
{
		GPIO_InitTypeDef  GPIO_InitStructure;
		GPIO_InitStructure.Pin = GPIO_PIN_7;
		GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
		GPIO_InitStructure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStructure);
}

#define NUM 1

void test_time(void)
{
    u8 i = 0;
    SDA_1;
    SCL_1;
    if(READ_SDA)
        i = 1;
    else
        i = 0;
    delay_1_ms();//delay_1_us();//delay_2_us();//delay_us(NUM);
    SDA_0;
    SCL_0;
    if(READ_SDA)
        i = 1;
    else
        i = 0;
    delay_1_ms();//delay_1_us();//delay_2_us();//delay_us(NUM);
}

u8 test_read_write(void)
{
    u8 write_data = 0;
    u8 read_data = 0;
    u8 ret = 0;
    u8 i = 0;
    for(i=1;i<255;i++)
    {
        AT24CXX_WriteOneByte(0x55, write_data);
        read_data = AT24CXX_ReadOneByte(0x55);
        if(write_data == read_data)
        {
            write_data++;
        }else
        {
            ret = 1;
        }
    }
    return ret;
}

#define TEST_ADDRESS 255
#define TEST_SIZE    255
u8 test_read_write_bytes(void)
{
    u8 write_data[TEST_SIZE];
    u8 read_data[TEST_SIZE];
    u8 i = 0;
    u8 ret = 0;
    
    for(i=0; i<TEST_SIZE;i++)
    {
        read_data[i] = 0;
        write_data[i] = i;
    }
    AT24CXX_Write(TEST_ADDRESS,write_data,TEST_SIZE);
    AT24CXX_Read(TEST_ADDRESS,read_data,TEST_SIZE);
    
    for(i=0;i<TEST_SIZE;i++)
    {
        if(write_data[i] != read_data[i] )
        {
            ret = i;
            return ret;
        }
    }
    return ret;
    
    
}
