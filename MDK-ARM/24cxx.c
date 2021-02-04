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

//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������
//IIC��ʼ��
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOB_CLK_ENABLE();   //ʹ��GPIOBʱ��
    
    //PH4,5��ʼ������
    GPIO_Initure.Pin=GPIO_PIN_6|GPIO_PIN_7;
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_FAST;     //����
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    
    SDA_1;
    SCL_1;;  
}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT();     //sda�����
	SDA_1;	  	  
	SCL_1;
	delay_2_us();delay_2_us();//delay_us(4);
 	SDA_0;//START:when CLK is high,DATA change form high to low 
	delay_2_us();delay_2_us();//delay_us(4);
	SCL_0;//ǯסI2C���ߣ�׼�����ͻ�������� 
	delay_2_us();delay_2_us();//delay_us(4);
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	SCL_0;
	SDA_0;//STOP:when CLK is high DATA change form low to high
	delay_2_us();delay_2_us();//delay_us(4);
	SCL_1;
    SDA_1;//����I2C���߽����ź�    
	delay_2_us();delay_2_us();//delay_us(4);
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
	u8 ucErrTime=0;
	SDA_IN();      //SDA����Ϊ����  
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
	SCL_0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
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
//������ACKӦ��		    
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
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(u8 txd)
{                        
    u8 t;   
	SDA_OUT(); 	    
    SCL_0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        if((txd&0x80)>>7==1)
			SDA_1;
		else
			SDA_0;
        txd<<=1; 	  
		delay_2_us();//delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		SCL_1;
		delay_2_us();//delay_us(2); 
		SCL_0;	
		delay_2_us();//delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA����Ϊ����
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
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
}

//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ  
//����ֵ  :����������
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{				  
	u8 temp=0;    
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	   //����д����
		IIC_Wait_Ack();
		IIC_Send_Byte(ReadAddr>>8);//���͸ߵ�ַ	 
	}else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //����������ַ0XA0,д���� 	 

	IIC_Wait_Ack(); 
    IIC_Send_Byte(ReadAddr%256);   //���͵͵�ַ
	IIC_Wait_Ack();	    
	IIC_Start();  	 	   
	IIC_Send_Byte(0XA1);           //�������ģʽ			   
	IIC_Wait_Ack();	 
    temp=IIC_Read_Byte(0);		   
    IIC_Stop();//����һ��ֹͣ����	
	return temp;
}
//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ    
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{				   	  	    																 
    u8 i = 0;
    IIC_Start();  
	if(EE_TYPE>AT24C16)
	{
		IIC_Send_Byte(0XA0);	    //����д����
		IIC_Wait_Ack();
		IIC_Send_Byte(WriteAddr>>8);//���͸ߵ�ַ
 	}else
	{
		IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //����������ַ0XA0,д���� 
	}	 
	IIC_Wait_Ack();	   
    IIC_Send_Byte(WriteAddr%256);   //���͵͵�ַ
	IIC_Wait_Ack(); 	 										  		   
	IIC_Send_Byte(DataToWrite);     //�����ֽ�							   
	IIC_Wait_Ack();  		    	   
    IIC_Stop();//����һ��ֹͣ���� 
	for(i =0; i<10;i++)
        delay_1_ms();//delay_ms(10);	 
}
//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ  
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{  	
	u8 t;
	for(t=0;t<Len;t++)
	{
		AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
	}												    
} 
//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ 
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
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
//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(255)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
#define LAST_ADDRESS 0xFFFF
u8 AT24CXX_Check(void)
{
    u8 temp;
    temp=AT24CXX_ReadOneByte(LAST_ADDRESS);//����ÿ�ο�����дAT24CXX			   
    if(temp==0X55)
        return 0;		   
    else//�ų���һ�γ�ʼ�������
    {
        AT24CXX_WriteOneByte(LAST_ADDRESS,0X55);
        temp=AT24CXX_ReadOneByte(LAST_ADDRESS);	  
        if(temp==0X55)
            return 0;
    }
    return 1;    
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);	
		NumToRead--;
	}
}  
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
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
