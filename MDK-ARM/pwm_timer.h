/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         delay.h
    Description:    This header contains global types and definitions
    =====================================================================================
*/
#ifndef __PWM_TIMER_H
#define __PWM_TIMER_H
#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "Timers.h"
#include "types.h"


extern u32 sleep_timer;
extern u32 preheat_timer;
extern u32 exposuring_timer;
extern u32 colling_timer;
extern u16 breath_led_timer;
extern u32 sleep2off_timer;

#define PREHEAT_DURATION        804
#define EXPOSURING_DURATION     110
#define COLLING_DURATION        202
#define BREATH_INTERVAL         1500
#define SLEEP2OFF_DURATION      180000//

typedef StaticTimer_t osStaticTimerDef_t;

#if 0
extern osTimerId_t myTimer01Handle;
extern osStaticTimerDef_t myTimer01ControlBlock;
extern const osTimerAttr_t myTimer01_attributes;
#endif

extern osTimerId_t myTimer02Handle;
extern osStaticTimerDef_t myTimer02ControlBlock;
extern const osTimerAttr_t myTimer02_attributes;
//void Callback01(void *argument);
void Callback02(void *argument);
#endif

