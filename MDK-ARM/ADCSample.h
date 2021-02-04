/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         ADCSample.h
    Description:    This header contains global types and definitions
    =====================================================================================
*/
#ifndef __ADC_SAMPLE_H
#define __ADC_SAMPLE_H
#include "main.h"
#include "types.h"

#define TUBE_CURRENT        ADC_CHANNEL_12
#define TUBE_VOLTAGE        ADC_CHANNEL_11
#define ADAPTER_VOLTAGE     ADC_CHANNEL_6
#define BATTERY_VOLTAGE     ADC_CHANNEL_7

extern ADC_HandleTypeDef hadc1;
u16 Get_Adc(u32 ch);
u16 Get_Adc_Average(u32 ch,u8 times);

#endif
