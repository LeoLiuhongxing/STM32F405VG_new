/*
 =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         PWM.h
    Description:    
 =====================================================================================
*/
#ifndef __PWM_H
#define __PWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"    
    
/* Macro definitions ---------------------------------------------------------*/
#define PWM_MA_REF     (1)
#define PWM_KV_REF     (0)
#define PWM_VOLTAGE_VALUE     1185//21.4%      1050//1397  //(1919) 37.48%
#define PWM_CURRENT_VALUE     1529//27.6%           1200//1330//0x590  //(1424)  27.82%
#define PWM_NUM_OF_CHANNELS   2
#define PWM_PERIOD_VALUE      0x15A5 //£¨0x15A5  5541£©15.25 KHz
/** 
 * Initialises the PWM hardware.
 * @return ERR_NONE for success, error code for failure
 * @see none
 *************************************************************************/
void PWM_Init(void);

/** 
 * Set the PWM output (duty cycle) to required level.
 * @param channel to set
 * @param output duty cycle scaled from 0 to 1
 * @return none
 * @see none
 *************************************************************************/
void pwmSet(uint32_t channel, float output);

/** 
 * Get the set PWM output (duty cycle)
 * @param channel to get
 * @return duty cycle scaled from 0 to 1
 * @see none
 *************************************************************************/
float pwmGet(uint32_t channel);

/** 
 * Set the PWM output (duty cycle) target to required level.
 * Calling pwmUpdate frequently makes output reach the
 * target eventually.
 * @param channel to set
 * @param target duty cycle scaled from 0 to 1
 * @return none
 * @see none
 *************************************************************************/
void pwmTargetSet(uint32_t channel, float target);    
#ifdef __cplusplus
}
#endif

#endif
