/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         delay.c
    Description:    used to delay
    =====================================================================================
*/
#include "delay.h"
#include "types.h"
#include "stm32f4xx_hal.h"


#define DELAY_COUNT (80) 
#define DELAY_COUNT_MS (990)

void delay_2_us(void)
{
    u32 i = 0;
    do{
        i++;
    }while(i<DELAY_COUNT);
        
}

void delay_1_us(void)
{
    u32 i = 0;
    do{
        i++;
    }while(i<(DELAY_COUNT/2));
        
}

void delay_1_ms(void)
{
    u32 i = 0;
    do{
        delay_1_us();
        i++;
    }while(i < DELAY_COUNT_MS);
}
