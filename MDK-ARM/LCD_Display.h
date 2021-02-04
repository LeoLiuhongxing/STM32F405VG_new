#ifndef __LCD_DISPLAY_H
#define __LCD_DISPLAY_H

#include "main.h"
#include "types.h"
#include "stm32f4xx_hal_gpio.h"




#define     RED          0XF800	  //红色
#define     GREEN        0X07E0	  //绿色
#define     BLUE         0X001F	  //蓝色
#define     WHITE        0XFFFF	  //白色

#define    BLACK         0x0000

#define  LCD_CS_1  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET) //GPIO_SetBits(GPIOD, GPIO_Pin_2)
#define  LCD_CS_0  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)

#define  LCD_RS_1  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_SET) 
#define  LCD_RS_0  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_RESET)

#define  LCD_WR_1  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET) 
#define  LCD_WR_0  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_RESET) 

#define  LCD_RD_1  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET) 
#define  LCD_RD_0  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET) 

#define  LCD_RESET_1  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET) 
#define  LCD_RESET_0  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET) 

#define  LCD_BK_1    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET) 
#define  LCD_BK_0    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET) 


void LCD_Init(void);

void LCD_SetArea(unsigned int stx,unsigned int sty,unsigned int endx,unsigned int endy);
void drawRec(uint16_t x,uint16_t y,uint16_t L,uint16_t W,uint16_t Colour);

void LcdWirteColorData(unsigned int color);

void LCD_Clear(unsigned int color);

void Display(uint16_t x,uint16_t y,uint8_t fontL,uint8_t fontW,const unsigned char *font, uint16_t foreColour,u8 num);

void pictureDisplay(uint16_t x,uint16_t L,uint16_t y,uint16_t W,const unsigned char *font);
void MainWindowDisplayFunc(void);

#endif /*__LCD_DISPLAY_H*/
