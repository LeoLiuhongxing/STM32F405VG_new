/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         data.c
    Description:    This file contains global data settings
    =====================================================================================
*/

#include "data.h"
#include "24cxx.h"
#include "pwm_timer.h"
#include "PWM.h"
#include "userinterface.h"
#include "stm32f4xx_hal.h"
#include "version.h"
#if X_RAY_TYPE
u16 const expose_table[27] = { 100, 110, 125, 140, 160, 180, 200, 220, 250, 280, 320, 360, 400, 450, 500,
560, 630, 710, 800, 900, 1000, 1100, 1250, 1400, 1600, 1800, 2000}; 
#else
u16 const expose_table[14] = { 100, 125, 160, 200, 250, 320, 400, 500, 630, 800, 1000, 1250, 1600, 2000}; 
#endif

#if 0
u16 const expose_time_default[6][8] = {400,400,320,400,400,400,320,400,//adult + analog
                                       200,200,160,200,200,200,160,200,//adult + plate
                                       160,160,125,160,160,160,125,160,//adult + sensor
                                       125,125,100,125,125,125,100,125,//child + sensor
                                       320,320,160,320,320,320,160,320,//child + analog
                                       160,160,100,160,160,160,100,160 //child + plate
                                       };
#else
u8 const expose_time_default[6][8] = { T400, T400, T320, T400, T400, T400, T320, T400,//adult + analog
                                       T200, T200, T160, T200, T200, T200, T160, T200,//adult + plate
                                       T160, T160, T125, T160, T160, T160, T125, T160,//adult + sensor
                                       T320, T320, T160, T320, T320, T320, T160, T320,//child + analog
                                       T160, T160, T100, T160, T160, T160, T100, T160, //child + plate
                                       T125, T125, T100, T125, T125, T125, T100, T125//child + sensor
                                       };

#endif
const _ExposurePara exposoure_para_default = {0,0,100,120,0,0x01};//曝光设置参数  DAP:1;
                                                                  //sleep time:120
#if 0                                                             //scan type: plate
u16 sys_expose_time[SCAN_TYPE_NO][8];     //曝光时间参数
u16 current_expose_time[SCAN_TYPE_NO][8]; //曝光时间参数
#else
u8 sys_expose_time[SCAN_TYPE_NO][TOOTH_POSITION_NO];     //曝光时间参数
u8 current_expose_time[SCAN_TYPE_NO][TOOTH_POSITION_NO]; //曝光时间参数
u8 demo_expose_time[SCAN_TYPE_NO][TOOTH_POSITION_NO];    //demo
#endif

_ExposurePara sys_exposoure_para;//曝光设置参数

_ExposurePara current_exposoure_para;//曝光设置参数
_ExposurePara demo_exposoure_para;//曝光设置参数




TOOTH_POSITION tooth_position = LEFT_UP_1;
TOOTH_POSITION demo_tooth_position = LEFT_UP_1;
//EVENT_TYPE nova_event[EVENT_NUMBER];
_EventData eventData;
extern IWDG_HandleTypeDef hiwdg;
//static u8 test_data[48] = {0};
void fagctory_default_store(void)
{
    //save default expose time
    u16 start_addr = EXPOSE_TIME_START_ADDRESS;
    u8 i = 0;
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
#if 0
    AT24CXX_Write(EXPOSE_TIME_START_ADDRESS, (u8*)expose_time_default, 48);    
#else
    for(i=0;i<6;i++)
    {
        AT24CXX_Write(EXPOSE_TIME_START_ADDRESS+8*i, (u8*)(&expose_time_default[i][0]), 8);
        __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    }

#if 0    
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    AT24CXX_Read(EXPOSE_TIME_START_ADDRESS, (u8*)test_data,48);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
#endif
#endif
    //__HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    //save default expose para
    start_addr = EXPOSE_PARA_START_ADDRESS;
    AT24CXX_WriteLenByte(start_addr,exposoure_para_default.ExposeCount,4);
    AT24CXX_WriteLenByte(start_addr+4,exposoure_para_default.DAPAccu,4);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    //AT24CXX_WriteOneByte(start_addr+8,exposoure_para_default.DAP);
    AT24CXX_WriteOneByte(start_addr+9,exposoure_para_default.SleepTime);
    AT24CXX_WriteOneByte(start_addr+10,exposoure_para_default.ScanType);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    AT24CXX_WriteOneByte(start_addr+11,exposoure_para_default.ScanMode);

    //save the default event record
    AT24CXX_WriteOneByte(EVENT_NUMBER_ADDRESS,0);
    eventData.EventCount = 0;
    AT24CXX_WriteOneByte(EVENT_RECORD_POINT,0);
    eventData.NextToUpdate = 0;
    for(u8 i=0; i<EVENT_NUMBER;i++)
    {
        AT24CXX_WriteOneByte(EVENT_RECORD_START_ADDRESS+i,0);
        eventData.data[i] = 0;
        __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    }
    
    //AT24CXX_WriteLenByte(VOLTAGE_PWM, PWM_VOLTAGE_VALUE,2);
    //AT24CXX_WriteLenByte(CURRENT_PWM, PWM_CURRENT_VALUE,2);
    
    //pwm_voltage_cali = PWM_VOLTAGE_VALUE;
    //pwm_current = PWM_CURRENT_VALUE;

}

void para_data_store(void)
{
    //save default expose time
    u16 start_addr = EXPOSE_TIME_START_ADDRESS;

    //AT24CXX_Write(EXPOSE_TIME_START_ADDRESS, (u8*)expose_time_default, 48);    
    //save default expose para
    start_addr = EXPOSE_PARA_START_ADDRESS;
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    if(sys_exposoure_para.ExposeCount != current_exposoure_para.ExposeCount)
        AT24CXX_WriteLenByte(start_addr,current_exposoure_para.ExposeCount,4);
    
    if(sys_exposoure_para.DAPAccu != current_exposoure_para.DAPAccu)
        AT24CXX_WriteLenByte(start_addr+4,current_exposoure_para.DAPAccu,4);

    //if(sys_exposoure_para.DAP != current_exposoure_para.DAP)
    //    AT24CXX_WriteOneByte(start_addr+8,current_exposoure_para.DAP);
    
    if(sys_exposoure_para.SleepTime != current_exposoure_para.SleepTime)
        AT24CXX_WriteOneByte(start_addr+9,current_exposoure_para.SleepTime);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    //AT24CXX_WriteOneByte(start_addr+9,exposoure_para_default.SleepTime);
    //AT24CXX_WriteOneByte(start_addr+10,exposoure_para_default.ScanType);
    if(sys_exposoure_para.ScanMode != current_exposoure_para.ScanMode)
        AT24CXX_WriteOneByte(start_addr+11,current_exposoure_para.ScanMode);

#if 0    
    //save the default event record
    AT24CXX_WriteOneByte(EVENT_NUMBER_ADDRESS,0);
    eventData.EventCount = 0;
    AT24CXX_WriteOneByte(EVENT_RECORD_POINT,0);
    eventData.NextToUpdate = 0;
    for(u8 i=0; i<EVENT_NUMBER;i++)
    {
        AT24CXX_WriteOneByte(EVENT_RECORD_START_ADDRESS+i,0);
        eventData.data[i] = 0;
    }
#endif    
}


void system_para_restore(void)
{
    u16 start_addr;
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    AT24CXX_Read(EXPOSE_TIME_START_ADDRESS, (u8*)sys_expose_time,48);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    for(u8 i=0;i<SCAN_TYPE_NO;i++)
    {
        for(u8 j=0;j<TOOTH_POSITION_NO;j++)
        {    
            current_expose_time[i][j] = sys_expose_time[i][j];
            demo_expose_time[i][j] = current_expose_time[i][j];
        }
    }
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    
    start_addr = EXPOSE_PARA_START_ADDRESS;
    sys_exposoure_para.ExposeCount = AT24CXX_ReadLenByte(start_addr,4);
    current_exposoure_para.ExposeCount = sys_exposoure_para.ExposeCount;
    
    sys_exposoure_para.DAPAccu = AT24CXX_ReadLenByte(start_addr+4,4);
    current_exposoure_para.DAPAccu = sys_exposoure_para.DAPAccu;
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    
    sys_exposoure_para.DAP = AT24CXX_ReadOneByte(start_addr+8);
    current_exposoure_para.DAP = sys_exposoure_para.DAP;
    sys_exposoure_para.SleepTime = AT24CXX_ReadOneByte(start_addr+9);
    current_exposoure_para.SleepTime = sys_exposoure_para.SleepTime;
    sys_exposoure_para.ScanType = AT24CXX_ReadOneByte(start_addr+10);
    current_exposoure_para.ScanType = sys_exposoure_para.ScanType;
    sys_exposoure_para.ScanMode = AT24CXX_ReadOneByte(start_addr+11);
    current_exposoure_para.ScanMode = sys_exposoure_para.ScanMode;
    demo_exposoure_para.ScanMode = current_exposoure_para.ScanMode;
    
    sleep_timer = current_exposoure_para.SleepTime*1000;
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    
    eventData.EventCount = AT24CXX_ReadOneByte(EVENT_NUMBER_ADDRESS);
    eventData.NextToUpdate = AT24CXX_ReadOneByte(EVENT_RECORD_POINT);
    for(u8 i=0;i<EVENT_NUMBER;i++)
    {
        eventData.data[i] = AT24CXX_ReadOneByte(EVENT_RECORD_START_ADDRESS+i);
        __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    }
}

void Demo_Para_Init(void)
{
    demo_exposoure_para.ExposeCount = current_exposoure_para.ExposeCount;
    
    demo_exposoure_para.DAP = current_exposoure_para.DAP;
    
    demo_exposoure_para.ExposeCount = current_exposoure_para.ExposeCount;
    
    demo_exposoure_para.ScanMode = current_exposoure_para.ScanMode;
    
    demo_exposoure_para.ScanType = current_exposoure_para.ScanType;
    
    demo_exposoure_para.SleepTime = current_exposoure_para.SleepTime;
}

void set_exposetime(u8 ScanType, TOOTH_POSITION tooth_position, u8 time)
{
    if(time > expose_table[T2000])
        return;
    if(ScanType >= SCAN_TYPE_NO)
        return;
    if(tooth_position >= TOOTH_POSITION_NO)
        return;
    u16 start_addr = EXPOSE_TIME_START_ADDRESS + ScanType*8 + tooth_position;
    AT24CXX_WriteOneByte(start_addr,time);
}

u8 get_exposetime(u8 ScanType, TOOTH_POSITION tooth_position)
{
    u8 temp = 0xFF;
    u16 start_addr = EXPOSE_TIME_START_ADDRESS + ScanType*8 + tooth_position;
    if(ScanType >= SCAN_TYPE_NO)
        return temp;
    if(tooth_position >= TOOTH_POSITION_NO)
        return temp;
    temp = AT24CXX_ReadOneByte(start_addr);
    return temp;
}

void set_sys_exposoure_para(u8 item, u32 pa)
{
    u8 temp = 0;
    switch(item)
    {
        case EXPOSE_COUNT:
            AT24CXX_WriteLenByte(EXPOSE_PARA_START_ADDRESS,pa,4);
            break;
        case DAP_ACCU:
            AT24CXX_WriteLenByte(EXPOSE_PARA_START_ADDRESS+4,pa,4);
            break;
        case DAP:
            temp = (u8)pa;
            AT24CXX_WriteOneByte(EXPOSE_PARA_START_ADDRESS+8,temp);
            break;
        case SLEEP_TIME:
            temp = (u8)pa;
            AT24CXX_WriteOneByte(EXPOSE_PARA_START_ADDRESS+9,temp);
            break;
        case SCAN_TYPE_PARA:
            temp = (u8)pa;
            AT24CXX_WriteOneByte(EXPOSE_PARA_START_ADDRESS+10,temp);
            break;
        case SCAN_MODE:
            temp = (u8)pa;
            AT24CXX_WriteOneByte(EXPOSE_PARA_START_ADDRESS+11,temp);
        default:
            break;
        
    }
}

void get_sys_exposoure_para(u8 item, u32* pa)
{
    switch(item)
    {
        case EXPOSE_COUNT:
            *pa = AT24CXX_ReadLenByte(EXPOSE_PARA_START_ADDRESS,4);
            break;
        case DAP_ACCU:
            *pa = AT24CXX_ReadLenByte(EXPOSE_PARA_START_ADDRESS+4,4);
            break;
        case DAP:
            *pa = AT24CXX_ReadOneByte(EXPOSE_PARA_START_ADDRESS+8);
            break;
        case SLEEP_TIME:
            *pa = AT24CXX_ReadOneByte(EXPOSE_PARA_START_ADDRESS+9);
            break;
        case SCAN_TYPE_PARA:
            *pa = AT24CXX_ReadOneByte(EXPOSE_PARA_START_ADDRESS+10);
        case SCAN_MODE:
            *pa = AT24CXX_ReadOneByte(EXPOSE_PARA_START_ADDRESS+11);
            break;
        default:
            break;
        
    }
}

u8 get_scan_type(u8 scanMode)
{
    scanMode &= 0x07;
    switch(scanMode)
    {
        case 0:
        case 4:
            return ADULT_ANA;
            //break;
        case 1:
        case 5:
            return CHILD_ANA;
            //break;
        case 2:
            return ADULT_PLATE;
            //break;
        case 3:
            return CHILD_PLATE;
            //break;
        case 6:
            return ADULT_SENSOR;
            //break;
        case 7:
            return CHILD_SENSOR;
            //break;
        default:
            break;
        
    }
}

void test_get_set_sys_para(void)
{   
    u32 temp_data;
    _ExposurePara temp = {0x12345678, 2000, 200, 180, 1,0x01};
    //_ExposurePara temp = {0, 0, 32, 32, 0};
    _ExposurePara read_temp = {0,0,0,0,0};
  
    set_sys_exposoure_para(EXPOSE_COUNT, temp.ExposeCount);
    set_sys_exposoure_para(DAP_ACCU, temp.DAPAccu);
    set_sys_exposoure_para(DAP, temp.DAP);
    set_sys_exposoure_para(SLEEP_TIME, temp.SleepTime);
    set_sys_exposoure_para(SCAN_TYPE_PARA, temp.ScanType);
    set_sys_exposoure_para(SCAN_MODE, temp.ScanMode);

    get_sys_exposoure_para(EXPOSE_COUNT,&read_temp.ExposeCount);
    get_sys_exposoure_para(DAP_ACCU,&read_temp.DAPAccu);
    get_sys_exposoure_para(DAP,&temp_data);
    read_temp.DAP = (u8)temp_data;
    get_sys_exposoure_para(SLEEP_TIME,&temp_data);  
    read_temp.SleepTime =  (u8)temp_data; 
    get_sys_exposoure_para(SCAN_TYPE_PARA,&temp_data);    
    read_temp.ScanType =  (u8)temp_data; 
    get_sys_exposoure_para(SCAN_MODE,&temp_data);    
    read_temp.ScanType =  (u8)temp_data; 
}

void test_para_store_restore(void)
{
    fagctory_default_store();
    system_para_restore();
}

void test_set_get_exposetime(void)
{ 
//    u16 start_addr = EXPOSE_TIME_START_ADDRESS;
    u8 temp_data = 0;

    for(u8 i=0;i<SCAN_TYPE_NO;i++)
    {
        for(u8 j=0;j<TOOTH_POSITION_NO;j++)
        {
            set_exposetime(i,j,temp_data);//get_exposetime(i,j);
            temp_data++;
        }
    }

    for(u8 i=0;i<SCAN_TYPE_NO;i++)
    {
        for(u8 j=0;j<TOOTH_POSITION_NO;j++)
        {
            temp_data = get_exposetime(i,j);//get_exposetime(i,j);
            if(temp_data != sys_expose_time[i][j])
                return;
        }
    }
}

void event_Data_Store(u8 data)
{
    u16 start_addr = EVENT_RECORD_START_ADDRESS + eventData.NextToUpdate;
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);

    AT24CXX_WriteOneByte(start_addr,data);
    eventData.data[eventData.NextToUpdate] = data;
    (eventData.NextToUpdate)++;
    if(eventData.NextToUpdate >= EVENT_NUMBER)
        eventData.NextToUpdate = 0;
    
    eventData.EventCount ++;
    if(eventData.EventCount >= EVENT_NUMBER)
        eventData.EventCount = EVENT_NUMBER;
    AT24CXX_WriteOneByte(EVENT_NUMBER_ADDRESS,eventData.EventCount);
    AT24CXX_WriteOneByte(EVENT_RECORD_POINT,eventData.NextToUpdate);
    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
    
}

u8 read_Event_Data(u8 offset)
{
    u16 start_addr = EVENT_RECORD_START_ADDRESS + offset;
    u8 temp = 0;
    if(offset >= EVENT_NUMBER)
        return 0;
    temp = AT24CXX_ReadOneByte(start_addr);
    return temp;
}
#if 0
static u8 test_event_data = 0;
void test_Event_Data_EEPROM_function(void)
{
    for(u8 i=0;i<EVENT_NUMBER;i++)
        event_Data_Store(i);
    
    for(u8 i=0;i<EVENT_NUMBER;i++)
        test_event_data = read_Event_Data(i);
}
#endif

void get_voltage_current(u16* voltage, u16* current)
{
    
    *voltage = AT24CXX_ReadLenByte(VOLTAGE_PWM,2);
    *current = AT24CXX_ReadLenByte(CURRENT_PWM,2);
    
}

void set_voltage_current(u16 voltage, u16 current)
{
    AT24CXX_WriteLenByte(VOLTAGE_PWM, voltage,2);
    AT24CXX_WriteLenByte(CURRENT_PWM, current,2);
}

void check_para_is_valid(void)
{
    u32 temp = 0;
    u8 temp_data[4] = {0,0,0,0}; 
    temp = AT24CXX_ReadLenByte(EERPOM_DATA_FLAG_ADDR,4);
    system_para_restore();
    for(u8 i=0;i<4;i++)
    {
        temp_data[3-i] = temp & 0xff;
        temp >>=8;
    }
    if( (temp_data[0] == EEPROM_DATA_VALID_FLAG0)
      &&(temp_data[1] == EEPROM_DATA_VALID_FLAG1)
      &&(temp_data[2] == EEPROM_DATA_VALID_FLAG2)
      &&(temp_data[3] == EEPROM_DATA_VALID_FLAG3)
      )
    {
        if(   (current_exposoure_para.DAP>=50)
            &&(current_exposoure_para.DAP<=150)
            &&(current_exposoure_para.ScanMode <= 7)
            &&(current_exposoure_para.ScanType <= 5)
            &&(current_exposoure_para.SleepTime >= 30)
            &&(current_exposoure_para.SleepTime <= 180)
          )
        {
            return ;//is ok
        }else
        {
            fagctory_default_store();
            system_para_restore();
        }
    }else
    {
        
        AT24CXX_WriteOneByte(EERPOM_DATA_FLAG_ADDR,EEPROM_DATA_VALID_FLAG0);
        AT24CXX_WriteOneByte(EERPOM_DATA_FLAG_ADDR+1,EEPROM_DATA_VALID_FLAG1);
        AT24CXX_WriteOneByte(EERPOM_DATA_FLAG_ADDR+2,EEPROM_DATA_VALID_FLAG2);
        AT24CXX_WriteOneByte(EERPOM_DATA_FLAG_ADDR+3,EEPROM_DATA_VALID_FLAG3);
        //para_data_store();
        fagctory_default_store();
        set_voltage_current(CALI_VOLTAGE,CALI_CURRENT);
        return;
    }
}
