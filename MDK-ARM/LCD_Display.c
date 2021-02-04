#include "LCD_Display.h"
#include "delay.h"



u8 reverse(u8 c)
{
    u8 sta[16] ={
        0x00,0x08,0x04,0x0C,0x02,0x0A,0x06,0x0E,0x01,0x09,0x05,0x0D,0x03,0x0B,0x07,0x0F
    };

    u8 d = 0; 
    d |= (sta[c&0xF]) << 4;
    d |= sta[c>>4];
    return d;
}
void WriteData(unsigned int val)
{  
    u8  temp = 0;
    u16 temp2 = 0;
    temp = (uint8_t)val;
    temp2 = (u16)reverse(temp);
    
    LCD_CS_0;	//CS=0;
    LCD_RS_1;
    LCD_RD_1;
    GPIOE->ODR &= 0x00FF;  //ÖÃÁãµÍ°ËÎ»
    GPIOE->ODR |= temp2<<8;//½«Êý¾ÝÌî³äµÍ°ËÎ»Êä³ö¼Ä´æÆ÷				
    LCD_WR_0;	//WR=0;
    LCD_WR_1;	//WR=1;
    LCD_CS_1;	//CS=1;	
}

void WriteCommand(unsigned int reg)		
{	
    u8  temp = 0;
    u16 temp2 = 0;
    temp = (uint8_t)reg;
    temp2 = (u16)reverse(temp);
    LCD_CS_0;	//CS=0;	
    LCD_RS_0;
    LCD_RD_1;

    GPIOE->ODR &= 0x00FF;  //ÖÃÁãµÍ°ËÎ»
    GPIOE->ODR |= temp2<<8;//½«Êý¾ÝÌî³äµÍ°ËÎ»Êä³ö¼Ä´æÆ÷				
    LCD_WR_0;	//WR=0;
    LCD_WR_1;	//WR=1;
    LCD_RS_1;
    LCD_CS_1;	//CS=1;		
}

void LCD_Init(void)
{
    u8 i = 0;
    GPIO_InitTypeDef GPIO_InitStruct;
    /*Configure GPIO pin : PC4  LCD backlight*/
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : PC5  LCD TE*/
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /*Configure GPIO pin : PB10  LCD CS*/
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pin : PB10  LCD RESET*/
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

     /*Configure GPIO pin : PE7  LCD RS*/
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

     /*Configure GPIO pin : PB2  LCD WR*/
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

       /*Configure GPIO pin : PB1  LCD RD*/
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /*Configure GPIO pins : PE8 PE9 PE10 PE11 
                           PE12 PE13 PE14 PE15 */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11 
                          |GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    LCD_RESET_1;  //set the P4^1 as 1 for the reset
    delay_1_ms();//delay_ms(1);               // Delay 1ms
    LCD_RESET_0;   //set the P4^1 as 0 for the reset
    delay_1_ms();//delay_ms(1);            // Delay 10ms
    LCD_RESET_1;  //set the P4^1 as 1 for the reset
    delay_1_ms();//delay_ms(1);       // Delay 200ms

//************************Start initial sequence****************************//

    WriteCommand(0xCF);  //Power control B
    WriteData(0x00); 
    WriteData(0xC1);  
    WriteData(0x30); 

    WriteCommand(0xED);  //Power on sequence control
    WriteData(0x64); 
    WriteData(0x03);  
    WriteData(0x12);
    WriteData(0x81);

    WriteCommand(0xCB);  //Power Control
    WriteData(0x39); 
    WriteData(0x2C);  
    WriteData(0x00);
    WriteData(0x34); 
    WriteData(0x02);  

    WriteCommand(0xF7);  //Pump ratio control
    WriteData(0x20);

    WriteCommand(0xEA);  //Driver timing contron B
    WriteData(0x00);
    WriteData(0x00);

    WriteCommand(0x36);  //Memory Access
    WriteData(0xB8);
    //LLCD_WRITE_DATA(0xf8);    //ºáÆÁ

    WriteCommand(0xB6);  //Display Function Control 
    WriteData(0x0A);
    WriteData(0xA2);

    WriteCommand(0xC0);  //Power Control  GVDD
    WriteData(0x21);    

    WriteCommand(0xC1);  //Power Control
    WriteData(0x11);    //BT[2:0] VGH,VGL

    WriteCommand(0xC5);  //VCMH/L
    WriteData(0x35); 
    WriteData(0x32);  

    WriteCommand(0xC7);  //VCOM Offset
    WriteData(0xB2);

    WriteCommand(0xF2);  //3Gamma Function
    WriteData(0x00); //00-disable; 03-enable

    WriteCommand(0xF6);  //IF Control
    WriteData(0x01);
    WriteData(0x31);
    WriteData(0x00);

    WriteCommand(0x3A);  //Pixel format
    WriteData(0x65);

    WriteCommand(0xB1);  //Frame Rate Control
    WriteData(0x00);
    WriteData(0x19);

    WriteCommand(0xE8);  //Driver timing control A
    WriteData(0x85); 
    WriteData(0x10);  
    WriteData(0x7A);

    WriteCommand(0xB4);  //Display Inversion Control
    WriteData(0x00);

    WriteCommand(0x35);  //TE
    WriteData(0x00);
         
    WriteCommand(0xE0); //SET GAMMA    PGAMMACTL
    WriteData(0x00);//--VP0 [3:0]
    WriteData(0x21);//--VP1 [5:0]
    WriteData(0x1E);//--VP2 [5:0]
    WriteData(0x0B);//--VP4 [3:0]
    WriteData(0x0E);//--VP6 [4:0]
    WriteData(0x09);//--VP13 [3:0]
    WriteData(0x4A);//--VP20 [6:0]
    WriteData(0xC5);//--VP36 [3:0]  VP27 [3:0]
    WriteData(0x3A);//--VP43 [6:0]
    WriteData(0x09);//--VP50 [3:0]
    WriteData(0x0B);//--VP57 [4:0]
    WriteData(0x04);//--VP59 [3:0]
    WriteData(0x16);//--VP61 [5:0]
    WriteData(0x17);//--VP62 [5:0]
    WriteData(0x00);//--VP63 [3:0]
    // 

    WriteCommand(0xE1); //SET GAMMA   NGAMMACTL
    WriteData(0x0F);//--VN0 [3:0]
    WriteData(0x1E);//--VN1 [5:0]
    WriteData(0x21);//--VN2 [5:0]
    WriteData(0x04);//--VN4 [3:0]
    WriteData(0x11);//--VN6 [4:0]
    WriteData(0x07);//--VN13 [3:0]
    WriteData(0x35);//--VN20 [6:0]
    WriteData(0x76);//--VN36 [3:0]  VN27 [3:0]
    WriteData(0x45);//--VN43 [6:0]
    WriteData(0x06);//--VN50 [3:0]
    WriteData(0x14);//--VN57 [4:0]
    WriteData(0x0B);//--VN59 [3:0]
    WriteData(0x27);//--VN61 [5:0]
    WriteData(0x38);//--VN62 [5:0]
    WriteData(0x0F);//--VN63 [3:0]
    //

    WriteCommand(0x11);  //Exit Sleep
    //delay_ms(25);// 100
    for(i = 0;i<25;i++)
    {
        delay_1_ms();
    }
    WriteCommand(0x29);   //Display On 
    delay_1_ms();//delay_ms(1);//100         
    WriteCommand(0x2C);   // 
	    //LCD_BK_1;
}



void LCD_SetArea(unsigned int stx,unsigned int sty,unsigned int endx,unsigned int endy)
{
	WriteCommand(0x2A);  
	WriteData(stx>>8);    
	WriteData(stx&0xff);    	
	WriteData(endx>>8); 
	WriteData(endx&0xff);	

	WriteCommand(0x2B);  
	WriteData(sty>>8); 
	WriteData(sty&0xff);	
	WriteData(endy>>8); 
	WriteData(endy&0xff);	
}

void LcdWirteColorData(unsigned int color)
{
    u8  temp = 0;
    u16 temp2 = 0;
    u16 temp3 = 0;
    
    temp = (uint8_t)(color>>8);
    temp2 = (u16)reverse(temp);
    
    temp = (uint8_t)color;
    temp3 = (u16)reverse(temp);

    LCD_CS_0;	//CS=0;
    LCD_RS_1;
    LCD_RD_1;
    
    GPIOE->ODR &= 0x00FF;  //ÖÃÁãµÍ°ËÎ»
    GPIOE->ODR |= temp2<<8;//½«Êý¾ÝÌî³äµÍ°ËÎ»Êä³ö¼Ä´æÆ÷				
    LCD_WR_0;	//WR=0;
    LCD_WR_1;	//WR=1;
    GPIOE->ODR &= 0x00FF;  //ÖÃÁãµÍ°ËÎ»
    GPIOE->ODR |= temp3<<8;//½«Êý¾ÝÌî³äµÍ°ËÎ»Êä³ö¼Ä´æÆ÷				
    LCD_WR_0;	//WR=0;
    LCD_WR_1;	//WR=1;
    LCD_CS_1;	//CS=1;	
}

void LCD_Clear(unsigned int color)//ÇåÆÁº¯Êý
{  
    unsigned int i,j;

    LCD_SetArea(0,0,319,239);
    WriteCommand(0x2C);
    for(i=0;i<240;i++)
    {
		for(j=0;j<32;j++)
		{
			LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
            LcdWirteColorData(color);
		}
	}
}



void drawRec(uint16_t x,uint16_t L,uint16_t y,uint16_t W,uint16_t Colour)//
{
	uint32_t i;
	LCD_SetArea(x,y,L-1,W-1);
	WriteCommand(0x2C);
	for(i=0;i<(L-x)*(W-y);i++)//
	{
		LcdWirteColorData(Colour);
	}	
}


void pictureDisplay(uint16_t x,uint16_t L,uint16_t y,uint16_t W,const unsigned char *font)//ÏÔÊ¾Í¼Æ¬
{
	u32 i;
	uint16_t colour;
	LCD_SetArea(x,y,x+L-1,y+W-1);
    WriteCommand(0x2C);
	for(i=0;i<L*W;i++)
	{
		colour = *font++;
		colour = (colour<<8)+*font++;
        LcdWirteColorData(colour);
	}
}


void Display(uint16_t x,uint16_t y,uint8_t fontL,uint8_t fontW,const unsigned char *font, uint16_t foreColour,u8 num)
{
	
	unsigned char i,j,k,c;
	u16 FontWByte = 0;
	FontWByte = fontW/8;
	
	for(i=0;i<fontL;i++){
		for(j=0;j<FontWByte;j++){
			c=*(font+num*FontWByte*fontL+i*FontWByte+j);//
			for(k=0;k<8;k++){
				if(c&(0x80>>k)){
					LCD_SetArea(x+j*8+k,y+i,x+j*8+k+1,y+i+1);//¿ª1*1´°
					WriteCommand(0x2C);
					LcdWirteColorData(foreColour);
				}
			}
		}
	}	
}











