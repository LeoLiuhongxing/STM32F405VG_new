/*
                    KaVo-Sybron Dental(Shanghai) Company Ltd
                    Copyright (C) 2020 - all rights reserved

    Project:        Nova
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         keys.h
    Description:    Header file for the keys module
    =====================================================================================
*/

#ifndef KEYS_H
#define KEYS_H

#define TIME_LONGKEY    200 // number multiplied by 10ms
#define NB_KEYS         14   // 14 Keys
#define DGL_CNT         2 // number of deglitch counts

typedef enum
{
    //K2              //K4        //K3        //K5
    ADULT_CHILD = 0, ANA_DIG,     KEY_UP,     KEY_DOWN,    
    //K6              //K7        //K8        //K9
    TOOTH_UP1,       TOOTH_UP2,   TOOTH_UP3,  TOOTH_UP4,
    //K10             //K11       //K12       //K13
    TOOTH_DOWN1,     TOOTH_DOWN2, TOOTH_DOWN3,TOOTH_DOWN4,
    EXPOSE_BUTTON,
    KEY_POWER,
    KEY_NO
} KEYS;

typedef enum
{
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_ON,
    KEY_PRESSED_LONG,
    KEY_CHANGED,
    KEY_OFF
} KEYS_STAT;


typedef struct
{
    uint8_t pin:            1;
    uint8_t changed:        1;
    uint8_t pressed:        1;
    uint8_t released:       1;
    uint8_t on:             1;
    uint8_t pressed_long:   1;
    uint8_t dglCnt;
    uint8_t longCnt;
    uint8_t smState;
} KEY;

#define KEY_ROW 3
#define KEY_COLUMN 4
typedef struct
{
    KEY key[KEY_NO];
} KEYS_STRUCT;

void Keys_GetPins(void);
void Keys_Statemachine(KEYS);
void Keys_Portinit(void);


BOOL Keys_HasAnyKeyChanged(void);
BOOL Keys_IsChanged(KEYS);
BOOL Keys_IsPressed(KEYS);
BOOL Keys_IsReleased(KEYS);
BOOL Keys_IsPressedLong(KEYS);
BOOL Keys_IsChangedClear(KEYS);
BOOL Keys_IsOn(KEYS);
BOOL Keys_IsPressedClear(KEYS);
BOOL Keys_IsReleasedClear(KEYS);
BOOL Keys_IsPressedLongClear(KEYS);
BOOL Keys_IsOnClear(KEYS);

extern void Keys_Scan(void);    // scans keys and set according flags. called every xx ms
extern void Keys_ClearAll(void); // clears all key flags
extern void Keys_ClearKey(KEYS);    // clears flags of secified key
extern void Keys_Init(void);  // port and key initiaization


#endif /* KEYS_H_ */
