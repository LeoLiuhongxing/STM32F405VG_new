/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         data.h
    Description:    This file contains global data settings
    =====================================================================================
*/
#ifndef __DATA_H
#define __DATA_H 	
#include "types.h"
#include "version.h"

#ifdef __cplusplus
 extern "C" {
#endif

#define EVENT_NUMBER (20)

/*EEPROM address: 0-1023*/ 
#define EXPOSE_TIME_START_ADDRESS (0)//2048

/*EEPROM address：1024-2047*/
#define EXPOSE_PARA_START_ADDRESS (1024)
     
/*EEPROM address：2048-3072*/
/*EEPROM 读写100万次，如果每天谢100次，可以使用30年*/
#define EVENT_RECORD_START_ADDRESS (2050)
#define EVENT_NUMBER_ADDRESS       (2048)
#define EVENT_RECORD_POINT         (2049)
     
//voltage 
#define VOLTAGE_PWM                 (3072) 
#define CURRENT_PWM                 (3074) 
     

#define CHILD_MODE   (0x01)
#define DIGITAL_MODE (0x02)
#define SENSOR_MODE  (0x04)     

#define MAX_SLEEP_DURATION  180 //s
#define MIN_SLEEP_DUARTION  30  //s
#define SLEEP_DURATION_STEP 5   //s

#define MAX_DAP             150 //s
#define MIN_DAP             50  //s
#define DAP_STEP 5          1//s

//
#define EEPROM_DATA_VALID_FLAG0 0x55
#define EEPROM_DATA_VALID_FLAG1 0xAA
#define EEPROM_DATA_VALID_FLAG2 0xAA
#define EEPROM_DATA_VALID_FLAG3 0x55
//
#define EERPOM_DATA_FLAG_ADDR   4096//4k

#define CALI_VOLTAGE            1000
#define CALI_CURRENT            1000


//event data 恢复出厂设置时，全部设置为默认值，
typedef struct{
    u8     EventCount;        //default value: 0
    u8     NextToUpdate;      //offset: default value is 0
    u8 	   data[EVENT_NUMBER];//default value: oxFF
}_EventData;

typedef struct{
    u32     ExposeCount;   //
    u32     DAPAccu;
    u8 	    DAP;            //Min: 50; Max: 150
    u8      SleepTime;      //Min: 30 Max:180; step:5
    u8      ScanType;       //0: ADULT_ANA    1: ADULT_PLATE  2: ADULT_SENSOR 
                            //3. CHILD_ANA    4: CHILD_PLATE  5: CHILD_SENSOR
    u8      ScanMode;       //bit 0: 0 adult,  1 child;
                            //bit 1: 0 analog, 1 digital
                            //bit 2: 0 plate,  1 sensor
}_ExposurePara;

typedef enum{
    EXPOSE_COUNT = 0,
    DAP_ACCU,
    DAP,
    SLEEP_TIME,
    SCAN_TYPE_PARA,
    SCAN_MODE,
    EXPOSOURE_PARA_NO
}EXPOSOURE_PARA;

typedef enum{
    PLATE,
    SENSOR
}DIGITALMODE;

typedef enum{
    POWER_ON=1,
    POWER_OFF,
    SLEEP_MODE,
    EXPOSUREDURATION,
    ERRORINFO,
    BATTERY_TOO_LOW,
    EVENT_NO
}EVENT_TYPE;

typedef enum{
    ADULT_ANA = 0,
    ADULT_PLATE,
    ADULT_SENSOR,
    CHILD_ANA,
    CHILD_PLATE,
    CHILD_SENSOR,
    SCAN_TYPE_NO
}SCAN_TYPE;

typedef enum
{
    LEFT_UP_1 = 0,
    LEFT_UP_2,
    LEFT_UP_3,
    LEFT_UP_4,
    LEFT_DOWN_1,
    LEFT_DOWN_2,
    LEFT_DOWN_3,
    LEFT_DOWN_4,
    TOOTH_POSITION_NO
}TOOTH_POSITION;

#if X_RAY_TYPE
typedef enum
{
    T100, T110, T125, T140,
    T160, T180, T200, T220,
    T250, T280, T320, T360,
    T400, T450, T500, T560,
    T630, T710, T800, T900,
    T1000,T1100,T1250,T1400,
    T1600,T1800,T2000,EXPO_TIMER_NO,
}EXPOSE_TIME_LIST;
#else
typedef enum
{
    T100, T125, T160, T200, 
    T250, T320, T400, T500, 
    T630, T800, T1000,T1250,
    T1600,T2000,EXPO_TIMER_NO,
}EXPOSE_TIME_LIST;
#endif
#if 0
extern u16 sys_expose_time[6][8];    //曝光时间参数
extern u16 current_expose_time[6][8];//曝光时间参数
#else
extern u8 sys_expose_time[6][8];     //曝光时间参数
extern u8 current_expose_time[6][8]; //曝光时间参数
extern u8 demo_expose_time[6][8];    //demo
#endif
#if X_RAY_TYPE
extern u16 const expose_table[27];
#else
extern u16 const expose_table[14];
#endif
extern _ExposurePara sys_exposoure_para;
extern _ExposurePara current_exposoure_para;//曝光设置参数
extern _ExposurePara demo_exposoure_para;   //曝光设置参数
extern TOOTH_POSITION demo_tooth_position; 
extern TOOTH_POSITION tooth_position;
extern EVENT_TYPE nova_event[EVENT_NUMBER];
extern const _ExposurePara exposoure_para_default;
extern _EventData eventData;

void test_para_store_restore(void);
void fagctory_default_store(void);
void system_para_restore(void);
void set_exposetime(u8 ScanType, TOOTH_POSITION tooth_position, u8 time);
u8 get_exposetime(u8 ScanType, TOOTH_POSITION tooth_position);
void get_sys_exposoure_para(u8 item, u32* pa);
void set_sys_exposoure_para(u8 item, u32 pa);
void test_set_get_exposetime(void);
void test_get_set_sys_para(void);
u8 get_scan_type(u8 scanMode);
void event_Data_Store(u8 data);
u8 read_Event_Data(u8 offset);
void para_data_store(void);
void get_voltage_current(u16* voltage, u16* current);
void set_voltage_current(u16 voltage, u16 current);
void check_para_is_valid(void);
#if 0
void test_Event_Data_EEPROM_function(void);
#endif
#ifdef __cplusplus
}
#endif

#endif

