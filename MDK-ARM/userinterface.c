/*
    =====================================================================================
                 Leo

 
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         userinterface.c
    Description:    This header contains global types and definitions
    =====================================================================================
*/
#include "types.h"
#include "keys.h"
#include "font.h"
#include "LCD_Display.h"
#include "powerON.h"
#include "delay.h"
#include "cmsis_os2.h"
#include "data.h"
#include "delay.h"
#include "PWM.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "userinterface.h"
#include "pwm_timer.h"
#include "ADCSample.h"
#include "PID.h"
#include "uart2.h"
#include "version.h"
#include "stm32f4xx_hal.h"

struct
{
    uint8_t up_act:     1;
    uint8_t down_act:   1;
} keyActive;    // used for autorepeat
WINDOWS WIN;// = MainWindowDisplay;

SYSTEM_STATE system_state;// = IDLE_STATE;
EXPOSURE_PROCESS exposuring_state = EXPROSURE_IDLE_STATE; 
EXPOSURE_PROCESS demo_exposuring_state = EXPROSURE_IDLE_STATE; 

#define EXP_ENABLE   HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 , GPIO_PIN_SET)
#define EXP_DISABLE  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0 , GPIO_PIN_RESET)
#define PARA_SETTING_NO_ALLOWED 0
#define PARA_SETTING_ALLOWED    1
#define REPEAT_COUNT            20
static u8 repeat_key_time = 0;
static u8 currentFocus = 0;
static u8 adult_child_long_pressed = 0;
static u8 system_reset_focus = 0;
static u8 system_para_focus = 0;
static u8 event_record_page = 0;
static u8 record_page_total = 5;
static u8 battery_charge_state = BATTERY_NO_CHARGE;
static u8 para_setting_state = PARA_SETTING_NO_ALLOWED;

u8 exposure_over_wait = 0;//0:no wait  1: wait_flag
static u8 demo_exposure_over_wait = 0;//

//static u8 key_up_down_pressed = 0;//0x01 up key is pressed
                                  //0x02 down key is pressed
                                  //0x04 up & down key is pressed
                                  
static u8 power_led_state = 0;                                  
#define UP_KEY_IS_PRESSED         0x01
#define DOWN_KEY_IS_PRESSED       0x02


BATTERY_STATE currentBatteryState = GRID_DISPLAY_NO;

static u8 key_event = 0;
u16 power_key_press_duration = 0;
u8  power_key_is_pressing = KEY_RELEASED;
u16 setting_timeout = 0;
u16 battery2low_timeout = 0;
u16 pwm_voltage_cali = 0;
u16 pwm_voltage = 0;
u16 pwm_current = 0;
u32 exposure_interval_timeout = 0;


static u8 get_battery_charge_state(void);
static u8 calibration_focus = 0;
extern IWDG_HandleTypeDef hiwdg;
static void GUI_StoreKey(u8 event)
{
    key_event = event;
    return;
}

static void GUI_RestoreKey(u8* event)
{
    *event = key_event;
    return;
}
/*
    =====================================================================================
    Name:           FeedInKeys
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key:        the key, to which the event belongs
                    keystat:    status (event) of key
    Return value:   -
    Description:    Feeds the key events into the GUI.
    =====================================================================================
*/
static void FeedInKeys(KEYS key, KEYS_STAT keystat)
{
    switch (key)
    {
        case ADULT_CHILD:                      
            if (keystat == KEY_PRESSED)
            {
                //if(Keys_IsOn(TOOTH_UP1))
                //{
                //    GUI_StoreKey(CALIBRATION_SETTING);
                //}else
                //{                
                    GUI_StoreKey(GUI_KEY_ADULT_CHILD);
                //}
            }
            else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_ADULT_CHILD_LONG);
            }else if(keystat == KEY_RELEASED)
            {
                if(Keys_IsOn(TOOTH_UP1) && Keys_IsOn(TOOTH_DOWN4))
                {
                    GUI_StoreKey(CALIBRATION_SETTING);
                }else
                {
                    GUI_StoreKey(GUI_KEY_ADULT_CHILD_RELEASED);
                }
            }
            break;
        case ANA_DIG:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_ANA_DIG);
            }
            else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_ANA_DIG_LONG);
            }else if (keystat == KEY_RELEASED)
            {
                if(Keys_IsOn(TOOTH_UP1) && Keys_IsOn(TOOTH_DOWN4))
                {
                    GUI_StoreKey(CALIBRATION_DAP_SETTING);
                }else
                {
                    GUI_StoreKey(GUI_KEY_ANA_DIG_RELEASED);
                }
                
            }

            break;

        case KEY_DOWN:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_DOWN);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                keyActive.down_act = 1;
                repeat_key_time = 0;
            }
            else if (keystat == KEY_RELEASED)
            {
                keyActive.down_act = 0;
            }
            break;

        case KEY_UP:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_UP);
                //Buzzer_Key();
            }else if (keystat == KEY_PRESSED_LONG)
            {
                    keyActive.up_act = 1;
                    repeat_key_time = 0;
                //GUI_StoreKey(GUI_KEY_UP_LONG);
            }
            else if (keystat == KEY_RELEASED)
            {
                //keyActive.down_act = 0;
                keyActive.up_act = 0;
            }
            break;

        case TOOTH_UP1:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP1);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP1_LONG);
            }
            break;

        case TOOTH_UP2:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP2);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP2_LONG);
            }
            break;
  
        case TOOTH_UP3:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP3);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP3_LONG);
            }
            break;
        case TOOTH_UP4:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP4);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_UP4_LONG);
            }
            break;
        case TOOTH_DOWN1:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN1);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN1_LONG);
            }
            break;
        case TOOTH_DOWN2:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN2);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN2_LONG);
            }
            break;
        case TOOTH_DOWN3:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN3);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN3_LONG);
            }
            break;
        case TOOTH_DOWN4:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN4);
            }else if (keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_TOOTH_DOWN4_LONG);
            }
            break;
        case EXPOSE_BUTTON:
            if (keystat == KEY_PRESSED)
            {
                GUI_StoreKey(GUI_KEY_EXPOSE_BUTTON);
            }else if(keystat == KEY_RELEASED)
            {
                GUI_StoreKey(GUI_KEY_EXPOSE_BUTTON_RELEASED);
            }
            break;
        case KEY_POWER:
            if (keystat == KEY_PRESSED)
            {
                power_key_is_pressing = KEY_PRESSED;
                taskENTER_CRITICAL();
                power_key_press_duration = 0;
                taskEXIT_CRITICAL();
                GUI_StoreKey(GUI_KEY_POWER);
            }else if(keystat == KEY_RELEASED)
            {
                GUI_StoreKey(GUI_KEY_POWER_RELEASED);
            }else if(keystat == KEY_PRESSED_LONG)
            {
                GUI_StoreKey(GUI_KEY_POWER_LONG);
            }
            break;

        default:
            // nothing pressed, make sure that flags are reset
            keyActive.up_act = 0;
            keyActive.down_act = 0;
            break;
    }
}


static void UIN_keypad_scan(void)
{
    KEYS currentkey;
    static BATTERY_STATE batteryState = GRID_DISPLAY_NO;
    static u8 battery_charge_state_new = BATTERY_CHARGING_INIT;

    if (Keys_HasAnyKeyChanged())    // any key change?
    {   // key change DETECTED
        set_sleep_timer();
        for (currentkey = ADULT_CHILD; currentkey <= KEY_POWER; currentkey++)
        {
            if (Keys_IsPressedClear(currentkey))    // key pressed?
            {   // key has been PRESSED - status has been cleared
                FeedInKeys(currentkey, KEY_PRESSED);
            }

            if (Keys_IsReleasedClear(currentkey))   // key released?
            {   // key has been RELEASED - status has been cleared
                FeedInKeys(currentkey, KEY_RELEASED);
            }

            if (Keys_IsPressedLongClear(currentkey))    // key long-pressed?
            {   // key has been LONG-PRESSED - status has been cleared
                FeedInKeys(currentkey, KEY_PRESSED_LONG);
            }
        }
    }else if((get_setting_timeout() == 0) &&(SYSTEM_SETTING_STATE == system_state)) 
    {
        GUI_StoreKey(GUI_SETTING_TIME_OUT);
    }
#if 1   
    else if((get_sleep_timer() == 0) && (system_state == IDLE_STATE))
    {
        GUI_StoreKey(GUI_SLEEP_TIMER);
    }
#endif     
    else if((get_battery2low_timeout() == 0) &&(BATTERY2LOW_STATE == system_state)) 
    {
        GUI_StoreKey(GUI_BATTERY2LOW_TIME_OUT);
    }
    else
    {
        if(system_state != EXPOSURE_STATE)
        {
            batteryState = update_battery_status();
            battery_charge_state_new = get_battery_charge_state();
            if((currentBatteryState != batteryState) || (battery_charge_state_new != battery_charge_state))
            {
                currentBatteryState = batteryState;
                battery_charge_state = battery_charge_state_new;
                GUI_StoreKey(GUI_BATTERY_UPDATE);
            }
        }
    }

}

void displayExpDly(void)
{
    uint8_t offset_x=220,offset_y=150;
    u8 scanType = 0;
    uint8_t tenth;//十分位
    uint8_t percentile;//百分位
    u16 unit;
    u8 temp;
    u8 last;
    
    scanType = get_scan_type(current_exposoure_para.ScanMode);
    temp = current_expose_time[scanType][tooth_position];
    unit = expose_table[temp];
    temp =unit/1000;
    tenth=(unit%1000)/100;
    percentile=(unit%100)/10;
    last = unit%10;

    drawRec(217,320,145,200,DarkGRAY);

    pictureDisplay(offset_x+0   ,16,offset_y,32,&num0_9_by[numSize_by*temp]);
    pictureDisplay(offset_x+16  ,16,offset_y,31,&num0_9_by[numSize_by*10]);
    pictureDisplay(offset_x+16*2-DOT_OFFSET_1,16,offset_y,32,&num0_9_by[numSize_by*tenth]);
    pictureDisplay(offset_x+16*3-DOT_OFFSET_1,16,offset_y,32,&num0_9_by[numSize_by*percentile]);
    pictureDisplay(offset_x+16*4-DOT_OFFSET_1,16,offset_y,32,&num0_9_by[numSize_by*last]);
    pictureDisplay(offset_x+16*5-DOT_OFFSET_1,16,offset_y,31,gImage_s_by);
}

void displayExpDly_Demo(void)
{
    uint8_t offset_x=220,offset_y=150;
    u8 scanType = 0;
    uint8_t tenth;//十分位
    uint8_t percentile;//百分位
    u16 unit;
    u8 temp;
    u8 last;
    
    scanType = get_scan_type(demo_exposoure_para.ScanMode);
    temp = demo_expose_time[scanType][demo_tooth_position];
    unit = expose_table[temp];
    temp =unit/1000;
    tenth=(unit%1000)/100;
    percentile=(unit%100)/10;
    last = unit%10;
    drawRec(217,320,145,200,DarkGRAY);

    pictureDisplay(offset_x+0   ,16,offset_y,32,&num0_9_by[numSize_by*temp]);
    pictureDisplay(offset_x+16  ,16,offset_y,31,&num0_9_by[numSize_by*10]);
    pictureDisplay(offset_x+16*2-DOT_OFFSET_1,16,offset_y,32,&num0_9_by[numSize_by*tenth]);
    pictureDisplay(offset_x+16*3-DOT_OFFSET_1,16,offset_y,32,&num0_9_by[numSize_by*percentile]);
    pictureDisplay(offset_x+16*4-DOT_OFFSET_1,16,offset_y,32,&num0_9_by[numSize_by*last]);
    pictureDisplay(offset_x+16*5-DOT_OFFSET_1,16,offset_y,31,gImage_s_by);
}


void powerOn_displayFunc(void)
{
    u8 temp[5] = {0,0,0,0,0};
    u16 offset_x;
    u16 offset_y;
#if X_RAY_TYPE
    pictureDisplay(55,209,50,61,gImage_powerON_A);    //显示soredx ver.10.0-1021

#else    
    pictureDisplay(55,209,50,44,gImage_powerON_A);    //显示soredx ver.10.0-1021
#endif
//    pictureDisplay(112,95,193,13,gImage_powerON_B);
//    drawRec(112,95,112+193,95+13,VerWhite);
    //pictureDisplay(VER_X,46,VER_Y,31,gImage_Ver);
    pictureDisplay(VER_X,28,VER_Y,19,gImage_Ver_small);
    offset_x = VER_X+28;
    offset_y = VER_Y;
    
    temp[0] = MAJOR_VER;
    temp[1] = MINOR_VER;
    temp[2] = MINOR_CHN;
    temp[3] = LASTNUM_VER/10;
    temp[4] = LASTNUM_VER%10;
    pictureDisplay(offset_x+VER_NUM_X*0,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*temp[0]]);
    pictureDisplay(offset_x+VER_NUM_X*1,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*10]);    //
    pictureDisplay(offset_x+VER_NUM_X*2-DOT_OFFSET_2,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*temp[1]]);
    pictureDisplay(offset_x+VER_NUM_X*3-DOT_OFFSET_2,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*10]);    //
    pictureDisplay(offset_x+VER_NUM_X*4-2*DOT_OFFSET_2,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*temp[2]]);
    pictureDisplay(offset_x+VER_NUM_X*5-2*DOT_OFFSET_2,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*10]);    //
    pictureDisplay(offset_x+VER_NUM_X*6-3*DOT_OFFSET_2,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*temp[3]]);
    pictureDisplay(offset_x+VER_NUM_X*7-3*DOT_OFFSET_2,VER_NUM_X,offset_y,VER_NUM_Y,&num0_9_wb[numSize_wb*temp[4]]);

    for(u16 i=0;i<3000;i++)
        delay_1_ms();
}

void powerOff_displayFunc(void)
{
    LCD_Clear(0XFFFF);
    pictureDisplay(67,187,82,29,gImage_powerOFF);

}
void MainWindowDisplayFunc(void)
{
    uint16_t offset_x,offset_y;
    u16 expose_time = 0;
    u8 temp_data[5] = {0,0,0,0,0};
    
    LCD_Clear(DarkGRAY);
    //drawRec(0,320,79,82,LightGRAY);
    drawRec(106,217,160,163,LightGRAY);
    drawRec(106,109,79,240,LightGRAY);
    drawRec(214,217,79,240,LightGRAY);
    offset_x=0;
    offset_y=15;

#if X_RAY_TYPE    
    expose_time = get_current_exposuretime();
    expose_time = expose_time*current_exposoure_para.DAP/100;//expose_time*current_exposoure_para.DAP*10/100;
    temp_data[0] = expose_time/10000;
    temp_data[1] = expose_time/1000%10;
    temp_data[2] = expose_time/100%10;
    temp_data[3] = expose_time/10%10;
    temp_data[4] = expose_time%10;
    
    if(temp_data[1] == 0)
    {
        //pictureDisplay(offset_x+16*0,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[1]]);
        pictureDisplay(offset_x+16*1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[2]]);
        pictureDisplay(offset_x+16*2,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[3]]);
        pictureDisplay(offset_x+16*3,16,offset_y,32,&num0_9_bw[numSize_bw*10]);    //
        pictureDisplay(offset_x+16*4-DOT_OFFSET_1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[4]]);

        pictureDisplay(97,MGYCM2_L,MGYCM2_Y,MGYCM2_W,gImage_mGycm2);    
    }else
    {
        //pictureDisplay(offset_x+16*0,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[0]]);
        pictureDisplay(offset_x+16*1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[1]]);
        pictureDisplay(offset_x+16*2,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[2]]);
        pictureDisplay(offset_x+16*3,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[2]]);
        pictureDisplay(offset_x+16*4,16,offset_y,32,&num0_9_bw[numSize_bw*10]);    //
        pictureDisplay(offset_x+16*5-DOT_OFFSET_1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[4]]);
        pictureDisplay(113,MGYCM2_L,MGYCM2_Y,MGYCM2_W,gImage_mGycm2);    

    }
#endif    
    offset_x=10;
    offset_y=95;

    offset_x=217;
    offset_y=19;

    offset_x=114;
    pictureDisplay(offset_x+16*0,16,110,31,&num0_9_bw[numSize_bw*2]);
    pictureDisplay(offset_x+16*1,16,110,32,&num0_9_bw[numSize_bw*10]);
    pictureDisplay(offset_x+16*2-DOT_OFFSET_1,16,110,31,&num0_9_bw[numSize_bw*5]);

    pictureDisplay(162,46,110,37,gImage_mA_bw);
    offset_x=124;
    pictureDisplay(offset_x+16*0,16,190,32,&num0_9_bw[numSize_bw*6]);
    pictureDisplay(offset_x+16*1,16,190,32,&num0_9_bw[numSize_bw*0]);

    pictureDisplay(165,36,190,36,gImage_kV_bw);
    
    if(current_exposoure_para.ScanMode & CHILD_MODE)
    {
        pictureDisplay(CHILD_UP_X,CHILD_UP_L,CHILD_UP_Y,CHILD_UP_W,gImage_childUp);
        drawRec(ADULT_UP_X,ADULT_UP_X+ADULT_UP_L,ADULT_UP_Y,ADULT_UP_Y+ADULT_UP_W,DarkGRAY);
        pictureDisplay(ADULT_DOWN_X,ADULT_DOWN_L,ADULT_DOWN_Y,ADULT_DOWN_W,gImage_adultDown);
    }else
    {
        drawRec(CHILD_UP_X,CHILD_UP_X+CHILD_UP_L,CHILD_UP_Y,CHILD_UP_Y+CHILD_UP_W,DarkGRAY);
        pictureDisplay(CHILD_DOWN_X,CHILD_DOWN_L,CHILD_DOWN_Y,CHILD_DOWN_W,gImage_childDown);
        pictureDisplay(ADULT_UP_X,ADULT_UP_L,ADULT_UP_Y,ADULT_UP_W,gImage_adultUp);
    }

    if(current_exposoure_para.ScanMode & DIGITAL_MODE)
    {
        pictureDisplay(SENSOR_UP_X,SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_W,gImage_sensorUp);
        drawRec(FILM_UP_X,FILM_UP_X+FILM_UP_L,FILM_UP_Y,FILM_UP_Y+FILM_UP_W,DarkGRAY);
        pictureDisplay(FILM_DOWN_X,FILM_DOWN_L,FILM_DOWN_Y,FILM_DOWN_W,gImage_filmDown);
    }else
    {
        drawRec(SENSOR_UP_X,SENSOR_UP_X+SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_Y+SENSOR_UP_W,DarkGRAY);
        pictureDisplay(SENSOR_DOWN_X,SENSOR_DOWN_L,SENSOR_DOWN_Y,SENSOR_DOWN_W,gImage_sensorDown);
        pictureDisplay(FILM_UP_X,FILM_UP_L,FILM_UP_Y,FILM_UP_W,gImage_filmUp);
    }
    display_battery_status(currentBatteryState);
    displayExpDly();
    LED_init(LED_OFF);
    LED_Control(tooth_position+1,LED_ON);
}

void DemoWindowDisplayFunc(void)
{
    uint16_t offset_x;//offset_y;
    LCD_Clear(DarkGRAY);

    drawRec(0,320,79,82,LightGRAY);
    drawRec(0,217,160,163,LightGRAY);
    drawRec(106,109,79,240,LightGRAY);
    drawRec(214,217,0,240,LightGRAY);

    Display(42,23,32,32,fontDemo, GrassGREEN,0);
    Display(74,23,32,32,fontDemo, GrassGREEN,1);
    Display(106,23,32,32,fontDemo, GrassGREEN,2);
    Display(138,23,32,32,fontDemo, GrassGREEN,3);

    offset_x=10;
    //offset_y=95;


    offset_x=217;
    //offset_y=19;

    offset_x=114;
    pictureDisplay(offset_x+16*0,16,110,31,&num0_9_bw[numSize_bw*2]);
    pictureDisplay(offset_x+16*1,16,110,32,&num0_9_bw[numSize_bw*10]);
    pictureDisplay(offset_x+16*2-DOT_OFFSET_1,16,110,31,&num0_9_bw[numSize_bw*5]);

    pictureDisplay(162,46,110,37,gImage_mA_bw);
    offset_x=124;
    pictureDisplay(offset_x+16*0,16,190,32,&num0_9_bw[numSize_bw*6]);
    pictureDisplay(offset_x+16*1,16,190,32,&num0_9_bw[numSize_bw*0]);

    pictureDisplay(165,36,190,36,gImage_kV_bw);
    
    //if(sys_exposoure_para.ScanMode & CHILD_MODE)
    if(demo_exposoure_para.ScanMode & CHILD_MODE)
    {
        pictureDisplay(CHILD_UP_X,CHILD_UP_L,CHILD_UP_Y,CHILD_UP_W,gImage_childUp);
        drawRec(ADULT_UP_X,ADULT_UP_X+ADULT_UP_L,ADULT_UP_Y,ADULT_UP_Y+ADULT_UP_W,DarkGRAY);
        pictureDisplay(ADULT_DOWN_X,ADULT_DOWN_L,ADULT_DOWN_Y,ADULT_DOWN_W,gImage_adultDown);
    }else
    {
        drawRec(CHILD_UP_X,CHILD_UP_X+CHILD_UP_L,CHILD_UP_Y,CHILD_UP_Y+CHILD_UP_W,DarkGRAY);
        pictureDisplay(CHILD_DOWN_X,CHILD_DOWN_L,CHILD_DOWN_Y,CHILD_DOWN_W,gImage_childDown);
        pictureDisplay(ADULT_UP_X,ADULT_UP_L,ADULT_UP_Y,ADULT_UP_W,gImage_adultUp);
    }

    if(demo_exposoure_para.ScanMode & DIGITAL_MODE)
    {
        pictureDisplay(SENSOR_UP_X,SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_W,gImage_sensorUp);
        drawRec(FILM_UP_X,FILM_UP_X+FILM_UP_L,FILM_UP_Y,FILM_UP_Y+FILM_UP_W,DarkGRAY);
        pictureDisplay(FILM_DOWN_X,FILM_DOWN_L,FILM_DOWN_Y,FILM_DOWN_W,gImage_filmDown);
    }else
    {
        drawRec(SENSOR_UP_X,SENSOR_UP_X+SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_Y+SENSOR_UP_W,DarkGRAY);
        pictureDisplay(SENSOR_DOWN_X,SENSOR_DOWN_L,SENSOR_DOWN_Y,SENSOR_DOWN_W,gImage_sensorDown);
        pictureDisplay(FILM_UP_X,FILM_UP_L,FILM_UP_Y,FILM_UP_W,gImage_filmUp);
    }
    display_battery_status(currentBatteryState);
    displayExpDly_Demo();
}

static u8 key_event;
void UserInterface(void)
{
    UIN_keypad_scan();

    if(keyActive.up_act)           // Generate auto-repeat events
    {
        if(repeat_key_time == 0)
        {    
            GUI_StoreKey(GUI_KEY_UP);
            repeat_key_time = REPEAT_COUNT;
        }else
        {
            repeat_key_time--;
        }
        //keyActive.up_act = 0;
    }

    if (keyActive.down_act)         // Generate auto-repeat events
    {
         
        if(repeat_key_time == 0)
        {    
            GUI_StoreKey(GUI_KEY_DOWN);
            repeat_key_time = REPEAT_COUNT;
        }else
        {
            repeat_key_time--;
        }

    }
    switch(system_state)
    {
        case IDLE_STATE:
            idle_state_process();
            break;
        case EXPOSURE_STATE:
            exposure_state_process();
            break;
        case DEMO_EXPOSURE_STATE:
            demo_exposure_state_process();
            break;
        case POWER_KEY_PRESSING_STATE:
            power_key_pressing_state_process();
            break;
        case SLEEP_STATE:
            sleep_state_process();
            break;
        case ERROR_STATE:
            error_state_process();
            break;
        case DEMO_STATE:
            demo_state_process();
            break;
        case SYSTEM_SETTING_STATE:
            system_setting_state_process();
            break;
        case BATTERY2LOW_STATE:
            battery2low_state_process();
            break;
        case CALIBRATION_STATE:
            calibration_state_process();
            break;
        case CALIBRATION_DAP_STATE:
            calibration_DAP_process();
            break;
        case DEMO_ERRO_STATE:
            demo_error_state_process();
            break;
        default:
            break;
    }
}

void Buzzer_Control(BUZZER_STATE state)
{
    switch(state)
    {
        case BUZZER_OFF:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);
            break;
        case BUZZER_ON:
             HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
           break;
    }
}

void Buzzer_Key(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_SET);
    osDelay(200);
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_2, GPIO_PIN_RESET);    
}

void idle_state_process(void)
{
     if(exposure_over_wait)
     {
         if((exposure_interval_timeout == 0) && (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET))
         {
             LED_control(GREEN_LED);
             exposure_over_wait = 0;
         }
     }else if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_SET)
     {
         LED_control(BLUE_LED);
     }else
     {
         LED_control(GREEN_LED);
     }
     if(key_event == GUI_KEY_ANA_DIG_LONG)
     {
        Buzzer_Key();
        demo_exposoure_para = exposoure_para_default;
        DemoWindowDisplayFunc();
        WIN = DemoWindowDisplay;
     }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
     {
        if(WIN == SystemSettingWin)
        {
            change_state(SYSTEM_SETTING_STATE);
            LED_control(BLUE_LED);
            set_setting_timeout();
        }else if(current_exposoure_para.ScanMode & CHILD_MODE)
        {
            current_exposoure_para.ScanMode &= ~CHILD_MODE;
            drawRec(CHILD_UP_X,CHILD_UP_X+CHILD_UP_L,CHILD_UP_Y,CHILD_UP_Y+CHILD_UP_W,DarkGRAY);
            pictureDisplay(CHILD_DOWN_X,CHILD_DOWN_L,CHILD_DOWN_Y,CHILD_DOWN_W,gImage_childDown);
            pictureDisplay(ADULT_UP_X,ADULT_UP_L,ADULT_UP_Y,ADULT_UP_W,gImage_adultUp);
            displayExpDly();
            display_mGycm2();
        }else
        {
            current_exposoure_para.ScanMode |= CHILD_MODE;
            pictureDisplay(CHILD_UP_X,CHILD_UP_L,CHILD_UP_Y,CHILD_UP_W,gImage_childUp);
            drawRec(ADULT_UP_X,ADULT_UP_X+ADULT_UP_L,ADULT_UP_Y,ADULT_UP_Y+ADULT_UP_W,DarkGRAY);
            pictureDisplay(ADULT_DOWN_X,ADULT_DOWN_L,ADULT_DOWN_Y,ADULT_DOWN_W,gImage_adultDown);
            displayExpDly();
            display_mGycm2();
        }
     }else if(key_event == GUI_KEY_ADULT_CHILD_LONG)
     {
         WIN = SystemSettingWin;
         currentFocus = 0;
         Buzzer_Key();
         display_system_setting_win(currentFocus);
         //change_state(SYSTEM_SETTING_STATE);
         
     }else if(key_event == GUI_KEY_ANA_DIG_RELEASED)//GUI_KEY_ANA_DIG)
     {
        if(WIN == DemoWindowDisplay)
        {
            demo_exposoure_para = exposoure_para_default;
            LED_init(LED_OFF);
            LED_Control(TOOTH_UP_1,LED_ON);
            demo_tooth_position = LEFT_UP_1;
            change_state(DEMO_STATE); 
            LED_control(GREEN_LED);
        }else if(current_exposoure_para.ScanMode & DIGITAL_MODE)
        {
            //sys_exposoure_para.ScanMode &= ~DIGITAL_MODE;
            current_exposoure_para.ScanMode &= ~DIGITAL_MODE;
            drawRec(SENSOR_UP_X,SENSOR_UP_X+SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_Y+SENSOR_UP_W,DarkGRAY);
            pictureDisplay(SENSOR_DOWN_X,SENSOR_DOWN_L,SENSOR_DOWN_Y,SENSOR_DOWN_W,gImage_sensorDown);
            pictureDisplay(FILM_UP_X,FILM_UP_L,FILM_UP_Y,FILM_UP_W,gImage_filmUp);
            display_mGycm2();
            displayExpDly();
        }else
        {
            current_exposoure_para.ScanMode |= DIGITAL_MODE;
            pictureDisplay(SENSOR_UP_X,SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_W,gImage_sensorUp);
            drawRec(FILM_UP_X,FILM_UP_X+FILM_UP_L,FILM_UP_Y,FILM_UP_Y+FILM_UP_W,DarkGRAY);
            pictureDisplay(FILM_DOWN_X,FILM_DOWN_L,FILM_DOWN_Y,FILM_DOWN_W,gImage_filmDown);
            display_mGycm2();
            displayExpDly();
        }
        
     }else if(key_event == GUI_KEY_EXPOSE_BUTTON)
     {
         //pwmSet(PWM_KV_REF, PWM_VOLTAGE_VALUE);
         //pwmSet(PWM_MA_REF,PWM_CURRENT_VALUE);
         //Buzzer_Control(BUZZER_ON);
         //xTimerStart(OneShotTimer_Handle,0);
         //change_state(EXPOSURE_STATE);
         //change_exposuring_state(PREHEAT_STATE);
         //osTimerStart(myTimer01Handle, 2000);
         //osTimerStart(myTimer02Handle, 2);
         if((exposure_interval_timeout == 0) && (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET))
         {
             change_state(EXPOSURE_STATE);
             change_exposuring_state(PREHEAT_STATE);
             taskENTER_CRITICAL();
             preheat_timer = PREHEAT_DURATION;
             taskEXIT_CRITICAL();
             EXP_ENABLE;
             //pwmSet(PWM_MA_REF,PWM_CURRENT_VALUE);
             //pid_value_init();//PID INIT
             pwmSet(PWM_MA_REF,pwm_current);
             Buzzer_Control(BUZZER_ON);
             LED_control(ORANGE_LED);
         }
#if DEBUG_ON
         else
         {
             u8 temp[6] = {'w','a','i','t','\r','\n'};
             put_data(temp,6);
         }
#endif         
     }else if(key_event == GUI_KEY_TOOTH_UP1)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_UP_1,LED_ON);
         tooth_position = LEFT_UP_1;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_UP2)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_UP_2,LED_ON);
         tooth_position = LEFT_UP_2;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_UP3)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_UP_3,LED_ON);
         tooth_position = LEFT_UP_3;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_UP4)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_UP_4,LED_ON);
         tooth_position = LEFT_UP_4;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_DOWN1)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_DOWN_1,LED_ON);
         tooth_position = LEFT_DOWN_1;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_DOWN2)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_DOWN_2,LED_ON);
         tooth_position = LEFT_DOWN_2;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_DOWN3)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_DOWN_3,LED_ON);
         tooth_position = LEFT_DOWN_3;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_DOWN4)
     {
         LED_init(LED_OFF);
         LED_Control(TOOTH_DOWN_4,LED_ON);
         tooth_position = LEFT_DOWN_4;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_TOOTH_UP1_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_UP_1, current_expose_time[scanType][LEFT_UP_1]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_UP2_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_UP_2, current_expose_time[scanType][LEFT_UP_2]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_UP3_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_UP_3, current_expose_time[scanType][LEFT_UP_3]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_UP4_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_UP_4, current_expose_time[scanType][LEFT_UP_4]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_DOWN1_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_DOWN_1, current_expose_time[scanType][LEFT_DOWN_1]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_DOWN2_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_DOWN_2, current_expose_time[scanType][LEFT_DOWN_2]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_DOWN3_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_DOWN_3, current_expose_time[scanType][LEFT_DOWN_3]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_TOOTH_DOWN4_LONG)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         taskENTER_CRITICAL();
         set_exposetime(scanType, LEFT_DOWN_4, current_expose_time[scanType][LEFT_DOWN_4]);
         taskEXIT_CRITICAL();
         Buzzer_Key();
     }else if(key_event == GUI_KEY_POWER_OFF)
     {
         //LCD_BK_0;
         //SYSTEM_POWER_OFF;
         system_power_off();
     }else if(key_event == GUI_KEY_POWER)
     {
         change_state(POWER_KEY_PRESSING_STATE);
     }else if(key_event == GUI_KEY_UP)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         if(current_expose_time[scanType][tooth_position] < T2000)
            current_expose_time[scanType][tooth_position]++;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_KEY_DOWN)
     {
         u8 scanType;
         scanType = get_scan_type(current_exposoure_para.ScanMode);
         if(current_expose_time[scanType][tooth_position] > T100)
            current_expose_time[scanType][tooth_position]--;
         displayExpDly();
         display_mGycm2();
     }else if(key_event == GUI_SLEEP_TIMER)
     {
         
         LCD_BK_0;
         LED_init(LED_OFF);
#if X_RAY_TYPE
         HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_RESET);
         HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_RESET);
         HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_RESET);
#else
         
#endif         
        //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);         
         change_state(SLEEP_STATE);
         taskENTER_CRITICAL();
         event_Data_Store(SLEEP_MODE);
         breath_led_timer = BREATH_INTERVAL;
         sleep2off_timer = SLEEP2OFF_DURATION;
         taskEXIT_CRITICAL();
     }
     else if(key_event == GUI_BATTERY_UPDATE)
     {
         display_battery_status(currentBatteryState);
         if((currentBatteryState == ZERO_GRID_DISPLAY) && (battery_charge_state == BATTERY_NO_CHARGE))
         {
             display_battery_2_low();
             change_state(BATTERY2LOW_STATE);
             set_battery2low_timeout();
         }
     }else if(key_event == CALIBRATION_SETTING)
     {
         calibration_focus = 0;
         display_pwm_set();
         change_state(CALIBRATION_STATE);
         WIN = CalibratinWin;
     }else if(key_event == CALIBRATION_DAP_SETTING)
     {
         display_DAP_set();
         change_state(CALIBRATION_DAP_STATE);
     }
#if DEBUG_ON
     if(key_event)     
     {
         put_char(key_event);
     }
#endif
     key_event = 0;
     
}

u16 current_feedback[5] = {0,0,0,0,0};
u8  current_feedback_count = 0;
u16 voltage_feedback[5] = {0,0,0,0,0};
u8  voltage_feedback_count = 0;

void exposure_state_process(void)
{
    switch(exposuring_state)
    {
        case EXPROSURE_IDLE_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {    
                u32 expose_time = 0;
                if(current_exposoure_para.ExposeCount < MAX_EXPOSE_COUNT)
                    current_exposoure_para.ExposeCount++;
                expose_time = get_current_exposuretime();
                expose_time = expose_time*current_exposoure_para.DAP/100;
                current_exposoure_para.DAPAccu += expose_time/10;
                exposure_interval_timeout = get_current_exposuretime()*60;
                //LED_control(BLUE_LED);
                exposure_over_wait = 1;
                osDelay(2000);//wait for battery voltage recovery
                change_state(IDLE_STATE);
                
                
#if DEBUG_ON
            put_string("voltage",7);
            put_char(voltage_feedback_count);

            voltage_feedback[0] = voltage_feedback[0]*66/4095;    
            put_char(voltage_feedback[0]);
            voltage_feedback[1] = voltage_feedback[1]*66/4095;    
            put_char(voltage_feedback[1]);
            voltage_feedback[2] = voltage_feedback[2]*66/4095;    
            put_char(voltage_feedback[2]);
            
            put_string("current",7);
            put_char(current_feedback_count);
            current_feedback[0] = current_feedback[0]*10*33/4095;    
            put_char(current_feedback[0]);
            current_feedback[1] = current_feedback[1]*10*33/4095;
            put_char(current_feedback[1]);
            current_feedback[2] = current_feedback[2]*10*33/4095;
            put_char(current_feedback[2]);
#endif    
            }
            
            key_event = 0;
            break;
        case PREHEAT_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {
                EXP_DISABLE;
                pwmSet(PWM_KV_REF, 0);
                pwmSet(PWM_MA_REF,0);
                Buzzer_Control(BUZZER_OFF);
                
                drawRec(E20_X-20,E20_X+E20_L+20,E20_Y-20,E20_W+E20_Y+20,NOVAGREEN );
                pictureDisplay(E20_X,E20_L,E20_Y,E20_W,gImage_E20_yb);
                taskENTER_CRITICAL();
                event_Data_Store(EXPOSUREDURATION);
                taskEXIT_CRITICAL();
                change_state(ERROR_STATE); 
                LED_control(BLUE_LED);
                WIN = ErrorWindowDisplay;                
            }else if(preheat_timer == 0)
            {
#if 1
                 u16 battery_data = 0;
                 u16 adjust_data = 0;
                 change_exposuring_state(EXPOSURING_STATE);

                 taskENTER_CRITICAL();
                 battery_data = Get_Adc_Average(BATTERY_VOLTAGE,3);
                 if(battery_data >= CHARGE_FULL_MAX) 
                     battery_data = CHARGE_FULL_MAX;
                                 
                 adjust_data = CHARGE_FULL_MAX*100/battery_data;
                 pwm_voltage = adjust_data*pwm_voltage_cali/100;                
                 exposuring_timer = get_current_exposuretime();//EXPOSURING_DURATION;
                 taskEXIT_CRITICAL();
                 pwmSet(PWM_KV_REF, pwm_voltage);
#else
                 change_exposuring_state(EXPROSURE_IDLE_STATE);
                 pwmSet(PWM_KV_REF, 0);
                 pwmSet(PWM_MA_REF,0);
                 EXP_DISABLE;
                 Buzzer_Control(BUZZER_OFF);
#endif                
            }else if(preheat_timer<720)
            {
#if 0
                u8 temp_data[6];
                //current_feedback[current_feedback_count] = Get_Adc_Average(TUBE_CURRENT,3);
                pid.ActualTarget = (float)Get_Adc_Average(TUBE_CURRENT,3);
                
                temp_data[0] = current_feedback_count;
                put_char(temp_data[0]);
                temp_data[0] = ' ';
                put_data(temp_data,1);
                put_char(pid.ActualTarget);
                temp_data[0] = ' ';
                temp_data[1] = ' ';
                temp_data[2] = ' ';
                put_data(temp_data,3);                
                //PID_realize((float)PWM_CURRENT_VALUE);
                put_char(pid.ActualTarget);
                temp_data[0] = '\r';
                temp_data[1] = '\n';
                put_data(temp_data,2);
                
                current_feedback_count++;
                //if(current_feedback_count == 5)
                //    current_feedback_count = 0;                
#endif                
            }
            key_event = 0;
            break;
        case EXPOSURING_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {
                EXP_DISABLE;
                pwmSet(PWM_KV_REF, 0);
                pwmSet(PWM_MA_REF,0);
                Buzzer_Control(BUZZER_OFF);
                //error state E20
                drawRec(E20_X-20,E20_X+E20_L+20,E20_Y-20,E20_W+E20_Y+20,NOVAGREEN );
                pictureDisplay(E20_X,E20_L,E20_Y,E20_W,gImage_E20_yb);
                taskENTER_CRITICAL();
                event_Data_Store(EXPOSUREDURATION);
                taskEXIT_CRITICAL();
                change_state(ERROR_STATE);
                WIN = ErrorWindowDisplay;
                LED_control(BLUE_LED);
            }else if(exposuring_timer == 0)
            {
                 change_exposuring_state(COLLING_STATE);
                 taskENTER_CRITICAL();
                 colling_timer = COLLING_DURATION;
                 taskEXIT_CRITICAL();
                 pwmSet(PWM_KV_REF, 0);
            }
#if 0              
            else if(exposuring_timer < 50)
            {
                voltage_feedback[voltage_feedback_count] = Get_Adc_Average(TUBE_VOLTAGE,3);
                voltage_feedback_count++;
                if(voltage_feedback_count == 5)
                    voltage_feedback_count = 0;
              
                current_feedback[current_feedback_count] = Get_Adc_Average(TUBE_CURRENT,3);
                current_feedback_count++;
                if(current_feedback_count == 5)
                    current_feedback_count = 0;

            }
#if 0
            else if(exposuring_timer < 50)
            {
                current_feedback[current_feedback_count] = Get_Adc_Average(TUBE_CURRENT,3);
                current_feedback_count++;
                if(current_feedback_count == 5)
                    current_feedback_count = 0;
                
            }
#endif            
#endif            
            key_event = 0;
          break;
        case COLLING_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {
                EXP_DISABLE;
                pwmSet(PWM_KV_REF, 0);
                pwmSet(PWM_MA_REF,0);
                Buzzer_Control(BUZZER_OFF);
                drawRec(E20_X-20,E20_X+E20_L+20,E20_Y-20,E20_W+E20_Y+20,NOVAGREEN );
                pictureDisplay(E20_X,E20_L,E20_Y,E20_W,gImage_E20_yb);
                taskENTER_CRITICAL();
                event_Data_Store(EXPOSUREDURATION);
                taskEXIT_CRITICAL();
                change_state(ERROR_STATE);
                WIN = ErrorWindowDisplay;
                LED_control(BLUE_LED);
            }else if(colling_timer == 0)
            {
                 Buzzer_Control(BUZZER_OFF);
                 change_exposuring_state(EXPROSURE_IDLE_STATE);
                 pwmSet(PWM_KV_REF, 0);
                 pwmSet(PWM_MA_REF,0);
                 LED_control(BLUE_LED);
                 EXP_DISABLE;
           }
           key_event = 0;
            break;
        default:
            key_event = 0;
            break;
    }
}

void demo_exposure_state_process(void)
{
    switch(demo_exposuring_state)
    {
        case EXPROSURE_IDLE_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {    
                //u32 expose_time = 0;
                change_state(DEMO_STATE);
#if 0                
                current_exposoure_para.ExposeCount++;
                expose_time = get_current_exposuretime();
                expose_time = expose_time*current_exposoure_para.DAP/100;
                current_exposoure_para.DAPAccu += expose_time;
#endif                
                exposure_interval_timeout = get_demo_exposuretime()*60;
                LED_control(BLUE_LED);
                demo_exposure_over_wait = 1;
            } 
            key_event = 0;
            break;
        case PREHEAT_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {
                EXP_DISABLE;
                Buzzer_Control(BUZZER_OFF);
                
                drawRec(E20_X-20,E20_X+E20_L+20,E20_Y-20,E20_W+E20_Y+20,NOVAGREEN );
                pictureDisplay(E20_X,E20_L,E20_Y,E20_W,gImage_E20_yb);
                change_state(DEMO_ERRO_STATE);
                LED_control(BLUE_LED);                
            }else if(preheat_timer == 0)
            {

                 change_demo_exposuring_state(EXPOSURING_STATE);

                 taskENTER_CRITICAL();
                 exposuring_timer = get_demo_exposuretime();//EXPOSURING_DURATION;
                 taskEXIT_CRITICAL();
            }
            key_event = 0;
            break;
        case EXPOSURING_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {
                EXP_DISABLE;
                Buzzer_Control(BUZZER_OFF);
                //error state E20
                drawRec(E20_X-20,E20_X+E20_L+20,E20_Y-20,E20_W+E20_Y+20,NOVAGREEN );
                pictureDisplay(E20_X,E20_L,E20_Y,E20_W,gImage_E20_yb);
                change_state(DEMO_ERRO_STATE);
                LED_control(BLUE_LED);
            }else if(exposuring_timer == 0)
            {
                 change_demo_exposuring_state(COLLING_STATE);
                 taskENTER_CRITICAL();
                 colling_timer = COLLING_DURATION;
                 taskEXIT_CRITICAL();
            }
            key_event = 0;
            break;
        case COLLING_STATE:
            if(key_event == GUI_KEY_EXPOSE_BUTTON_RELEASED)
            {
                EXP_DISABLE;
                Buzzer_Control(BUZZER_OFF);
                drawRec(E20_X-20,E20_X+E20_L+20,E20_Y-20,E20_W+E20_Y+20,NOVAGREEN );
                pictureDisplay(E20_X,E20_L,E20_Y,E20_W,gImage_E20_yb);
                change_state(DEMO_ERRO_STATE);
                LED_control(BLUE_LED);
            }else if(colling_timer == 0)
            {
                 Buzzer_Control(BUZZER_OFF);
                 change_demo_exposuring_state(EXPROSURE_IDLE_STATE);
                 LED_control(BLUE_LED);
                 EXP_DISABLE;
           }
           key_event = 0;
            break;
        default:
            key_event = 0;
            break;
    }
}

void sleep_state_process(void)
{
    if(key_event == GUI_KEY_POWER_OFF)
    {
        //SYSTEM_POWER_OFF;
        system_power_off();
    }else if(key_event != 0)//(key_event == GUI_KEY_POWER)
    {
//        change_state(IDLE_STATE);
        MainWindowDisplayFunc();
        change_state(IDLE_STATE);
        WIN = MainWindowDisplay;
        power_led_state = 0;
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);//GPIO_PIN_RESET);
#if 0
        if(exposure_over_wait)
        {
             if((exposure_interval_timeout == 0) && (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0) == GPIO_PIN_RESET))
             {
                 LED_control(GREEN_LED);
                 exposure_over_wait = 0;
             }else
                 LED_control(BLUE_LED);
        }
#endif        
        LED_control(GREEN_LED);
        LCD_BK_1;
    }else if(breath_led_timer == 0)
    {
        taskENTER_CRITICAL();
        breath_led_timer = BREATH_INTERVAL;
        taskEXIT_CRITICAL();
#if X_RAY_TYPE        
        if(power_led_state == 0)
        {
            power_led_state = 1;
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
        }else
        {
            power_led_state = 0;
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
        }
#else
        if(power_led_state == 0)
        {
            power_led_state = 1;
            //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);
            LED_control(GREEN_LED);
        }else
        {
            power_led_state = 0;
            //HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_RESET);

        }    
#endif
    }else if(sleep2off_timer == 0)
    {
            LCD_BK_1;
            powerOff_displayFunc();
            osDelay(3000);
            para_data_store();
            system_power_off();
    }
    
    key_event = 0;
    
}

void error_state_process(void)
{
   // if(key_event == GUI_KEY_ADULT_CHILD)
   // {
   //     MainWindowDisplayFunc();
   //     WIN = MainWindowDisplay;
        
   // }else 
    if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
    {
        change_state(IDLE_STATE);
#if 0        
        if(WIN != MainWindowDisplay)
        {
            WIN = MainWindowDisplay;
            MainWindowDisplayFunc();
        }
#endif  
        WIN = MainWindowDisplay;
        MainWindowDisplayFunc();
        LED_control(GREEN_LED);
       
    }
    key_event = 0;
}

void demo_error_state_process(void)
{
    if(key_event == GUI_KEY_ADULT_CHILD)
    {
        //MainWindowDisplayFunc();
        DemoWindowDisplayFunc();

    }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
    {
        change_state(DEMO_STATE);
        WIN = DemoWindowDisplay;
        LED_control(GREEN_LED);
    }
    key_event = 0;
}

void demo_state_process(void)
{
     if(demo_exposure_over_wait)
     {
         if(exposure_interval_timeout == 0)
         {
             LED_control(GREEN_LED);
             demo_exposure_over_wait = 0;
         }
     }
    
    if(key_event == GUI_KEY_ANA_DIG_LONG)
    {
        Buzzer_Key();
        MainWindowDisplayFunc();
        WIN = MainWindowDisplay;
    }else if(key_event == GUI_KEY_ADULT_CHILD)
    {
        if(demo_exposoure_para.ScanMode & CHILD_MODE)
        {
            demo_exposoure_para.ScanMode &= ~CHILD_MODE;
            drawRec(CHILD_UP_X,CHILD_UP_X+CHILD_UP_L,CHILD_UP_Y,CHILD_UP_Y+CHILD_UP_W,DarkGRAY);
            pictureDisplay(CHILD_DOWN_X,CHILD_DOWN_L,CHILD_DOWN_Y,CHILD_DOWN_W,gImage_childDown);
            pictureDisplay(ADULT_UP_X,ADULT_UP_L,ADULT_UP_Y,ADULT_UP_W,gImage_adultUp);
        }else
        {
            demo_exposoure_para.ScanMode |= CHILD_MODE;
            pictureDisplay(CHILD_UP_X,CHILD_UP_L,CHILD_UP_Y,CHILD_UP_W,gImage_childUp);
            drawRec(ADULT_UP_X,ADULT_UP_X+ADULT_UP_L,ADULT_UP_Y,ADULT_UP_Y+ADULT_UP_W,DarkGRAY);
            pictureDisplay(ADULT_DOWN_X,ADULT_DOWN_L,ADULT_DOWN_Y,ADULT_DOWN_W,gImage_adultDown);
        }
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_ANA_DIG_RELEASED)
    {
        if(WIN == MainWindowDisplay)
        {
            change_state(IDLE_STATE); 
            if(demo_exposure_over_wait)
                demo_exposure_over_wait = 0;
            if(exposure_interval_timeout != 0)
            {
                taskENTER_CRITICAL();
                exposure_interval_timeout = 0;
                taskEXIT_CRITICAL();
            }
            LED_control(GREEN_LED);
        }else if(demo_exposoure_para.ScanMode & DIGITAL_MODE)
        {
            demo_exposoure_para.ScanMode &= ~DIGITAL_MODE;
            drawRec(SENSOR_UP_X,SENSOR_UP_X+SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_Y+SENSOR_UP_W,DarkGRAY);
            pictureDisplay(SENSOR_DOWN_X,SENSOR_DOWN_L,SENSOR_DOWN_Y,SENSOR_DOWN_W,gImage_sensorDown);
            pictureDisplay(FILM_UP_X,FILM_UP_L,FILM_UP_Y,FILM_UP_W,gImage_filmUp);
            displayExpDly_Demo();
        }else
        {
            demo_exposoure_para.ScanMode |= DIGITAL_MODE;
            pictureDisplay(SENSOR_UP_X,SENSOR_UP_L,SENSOR_UP_Y,SENSOR_UP_W,gImage_sensorUp);
            drawRec(FILM_UP_X,FILM_UP_X+FILM_UP_L,FILM_UP_Y,FILM_UP_Y+FILM_UP_W,DarkGRAY);
            pictureDisplay(FILM_DOWN_X,FILM_DOWN_L,FILM_DOWN_Y,FILM_DOWN_W,gImage_filmDown);
            displayExpDly_Demo();
        }
        //displayExpDly();

    }else if(key_event == GUI_KEY_EXPOSE_BUTTON)
    {
         if(exposure_interval_timeout == 0)
         {
             change_state(DEMO_EXPOSURE_STATE);
             change_demo_exposuring_state(PREHEAT_STATE);
             taskENTER_CRITICAL();
             preheat_timer = PREHEAT_DURATION;
             taskEXIT_CRITICAL();
             //EXP_ENABLE;
             Buzzer_Control(BUZZER_ON);
             LED_control(ORANGE_LED);
         }else
         {
             u8 temp[6] = {'w','a','i','t','\r','\n'};
             put_data(temp,6);
         }
    }else if(key_event == GUI_KEY_TOOTH_UP1)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_UP_1,LED_ON);
        demo_tooth_position = LEFT_UP_1;
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_UP2)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_UP_2,LED_ON);
        demo_tooth_position = LEFT_UP_2;
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_UP3)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_UP_3,LED_ON);
        demo_tooth_position = LEFT_UP_3;
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_UP4)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_UP_4,LED_ON);
        demo_tooth_position = LEFT_UP_4;
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_DOWN1)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_DOWN_1,LED_ON);
        demo_tooth_position = LEFT_DOWN_1;
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_DOWN2)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_DOWN_2,LED_ON);
        demo_tooth_position = LEFT_DOWN_2;
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_DOWN3)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_DOWN_3,LED_ON);
        demo_tooth_position = LEFT_DOWN_3;
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_DOWN4)
    {
        LED_init(LED_OFF);
        LED_Control(TOOTH_DOWN_4,LED_ON);
        demo_tooth_position = LEFT_DOWN_4;
        //displayExpDly();
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_TOOTH_UP1_LONG)
    {
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_UP2_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_UP_2, current_expose_time[scanType][LEFT_UP_2]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_UP3_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_UP_3, current_expose_time[scanType][LEFT_UP_3]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_UP4_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_UP_4, current_expose_time[scanType][LEFT_UP_4]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_DOWN1_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_DOWN_1, current_expose_time[scanType][LEFT_DOWN_1]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_DOWN2_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_DOWN_2, current_expose_time[scanType][LEFT_DOWN_2]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_DOWN3_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_DOWN_3, current_expose_time[scanType][LEFT_DOWN_3]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }else if(key_event == GUI_KEY_TOOTH_DOWN4_LONG)
    {
        //u8 scanType;
        //scanType = get_scan_type(current_exposoure_para.ScanMode);
        //taskENTER_CRITICAL();
        //set_exposetime(scanType, LEFT_DOWN_4, current_expose_time[scanType][LEFT_DOWN_4]);
        //taskEXIT_CRITICAL();
        Buzzer_Key();
    }
    //else if(key_event == GUI_KEY_POWER_OFF)
    //{
    //    LCD_BK_0;
    //    SYSTEM_POWER_OFF;
    //}else if(key_event == GUI_KEY_POWER)
    //{
    //change_state(POWER_KEY_PRESSING_STATE);
    //}
    else if(key_event == GUI_KEY_UP)
    {
        u8 scanType;
        scanType = get_scan_type(demo_exposoure_para.ScanMode);
        if(demo_expose_time[scanType][demo_tooth_position] < T2000)
            demo_expose_time[scanType][demo_tooth_position]++;
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_DOWN)
    {
        u8 scanType;
        scanType = get_scan_type(demo_exposoure_para.ScanMode);
        if(demo_expose_time[scanType][demo_tooth_position] > T100)
            demo_expose_time[scanType][demo_tooth_position]--;
        displayExpDly_Demo();
    }else if(key_event == GUI_KEY_POWER_LONG)
    {
            powerOff_displayFunc();
            osDelay(3000);
            para_data_store();
            system_power_off();
            //poweroff_state = 1;
    }
    //else if(key_event == GUI_SLEEP_TIMER)
    //{
    //LCD_BK_0;
    //change_state(SLEEP_STATE);
    //}
    key_event = 0;
}

static u8 poweroff_state = 0;
void power_key_pressing_state_process(void)
{
    if(poweroff_state == 0)
    {
        if(power_key_press_duration > POWER_OFF_DURATION)
        {
            //LCD_BK_0;
            //SYSTEM_POWER_OFF;
            powerOff_displayFunc();
            osDelay(3000);
            para_data_store();
            system_power_off();
            poweroff_state = 1;
             
        }else if(key_event == GUI_KEY_POWER_RELEASED)
        {
            LCD_BK_0;
            LED_init(LED_OFF);
#if X_RAY_TYPE
             HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_RESET);
             HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_RESET);
             HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_RESET);
#else
         
#endif 
            change_state(SLEEP_STATE);
            sleep2off_timer = SLEEP2OFF_DURATION;
        }
        key_event = 0;
    }
}

static void system_infor_change_win(u8 focus)
{
    switch(focus)
    {
        case 0:
            display_system_infor_win();
            WIN = SystemInforWin;
            break;
        case 2:
            system_para_focus = 0;
#if X_RAY_TYPE        
            display_system_para_setting(system_para_focus);
#else
            display_system_para_setting_maxRay(system_para_focus);        
#endif        
            WIN = ParaSettingWin;
            break;
        case 1:
            event_record_page = 0;
            display_event_record();
            WIN = EventInforWin;
            break;
        case 3:
           system_reset_focus = 0;
           display_system_reset_win(system_reset_focus);
           WIN = SystemResetWin;
            break;
        default:
            break;                            
    }
}

static uint8_t jump_flag = 0;
void system_setting_state_process(void)
{
    
    switch(WIN)
    {
        case SystemSettingWin:
            if(key_event == GUI_KEY_UP)
            {
                if(currentFocus == 0)
                    currentFocus = 3;
                else
                    currentFocus--;
                display_system_setting_win(currentFocus);
                set_setting_timeout();
            }else if(key_event == GUI_KEY_DOWN)
            {
                if(currentFocus == 3)
                    currentFocus = 0;
                else
                    currentFocus++;
                display_system_setting_win(currentFocus);
                set_setting_timeout();
            }else if(key_event == GUI_KEY_ADULT_CHILD_LONG)
            {
                Buzzer_Key();
                MainWindowDisplayFunc();
                adult_child_long_pressed = 1;
                set_setting_timeout();
            }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
            {
                if(adult_child_long_pressed)
                {    
                     change_state(IDLE_STATE);
                     WIN = MainWindowDisplay;
                     adult_child_long_pressed = 0;
                    //LED_init(LED_OFF);
                    //LED_Control(tooth_position,LED_ON);

                     
                }else
                {
                     system_infor_change_win(currentFocus);
                }
            }else if(key_event == GUI_SETTING_TIME_OUT)
            {
                     MainWindowDisplayFunc();
                     change_state(IDLE_STATE);
                     WIN = MainWindowDisplay;
            }else if(key_event == GUI_KEY_POWER_LONG)
            {
                    powerOff_displayFunc();
                    osDelay(3000);
                    para_data_store();
                    system_power_off();
                    //poweroff_state = 1;
            }
            key_event = 0;
            break;
        case SystemInforWin:
            if(key_event == GUI_KEY_ADULT_CHILD)
            {
                display_system_setting_win(currentFocus);
                
            }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
            {
                WIN = SystemSettingWin;
            }else if(key_event == GUI_SETTING_TIME_OUT)
            {
                MainWindowDisplayFunc();
                change_state(IDLE_STATE);
                WIN = MainWindowDisplay;
            }else if(key_event == GUI_KEY_POWER_LONG)
            {
                    powerOff_displayFunc();
                    osDelay(3000);
                    para_data_store();
                    system_power_off();
                    //poweroff_state = 1;
            }
            key_event = 0;
            break;
        case EventInforWin:
            if(key_event == GUI_KEY_UP)
            {
                if(event_record_page == 0)
                {
                    event_record_page = (record_page_total-1);
                }
                else
                {
                    event_record_page--;
                }
                display_event_record();
                set_setting_timeout();

            }else if(key_event == GUI_KEY_DOWN)
            {
                if(event_record_page == (record_page_total-1))
                {    
                    event_record_page = 0;
                }
                else
                {    
                    event_record_page++;
                }
                display_event_record();
                set_setting_timeout();

            }else if(key_event == GUI_KEY_ADULT_CHILD)
            {
                display_system_setting_win(currentFocus);
                set_setting_timeout();
            }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
            {
                WIN = SystemSettingWin;
                set_setting_timeout();
            }else if(key_event == GUI_SETTING_TIME_OUT)
            {
                 MainWindowDisplayFunc();
                 change_state(IDLE_STATE);
                 WIN = MainWindowDisplay;
            }else if(key_event == GUI_KEY_POWER_LONG)
            {
                    powerOff_displayFunc();
                    osDelay(3000);
                    para_data_store();
                    system_power_off();
                    //poweroff_state = 1;
            }
            key_event = 0;
            break;
        case ParaSettingWin:
            if(key_event == GUI_KEY_UP)
            {
#if X_RAY_TYPE
                if(para_setting_state == PARA_SETTING_ALLOWED)
                {
                    set_system_para(system_para_focus);
                }else
                {
                    if(system_para_focus == 0)
                        system_para_focus = 2;
                    else
                        //system_para_focus--;
                        system_para_focus -= 2;
                    display_system_para_setting(system_para_focus);
                }
#else
                if(para_setting_state == PARA_SETTING_ALLOWED)
                {
                    set_system_para_maxRay(system_para_focus);
                }else
                {
                    if(system_para_focus == 0)
                        system_para_focus = 1;
                    else
                        system_para_focus--;
                    display_system_para_setting_maxRay(system_para_focus);
                }
#endif                
                set_setting_timeout();
            }else if(key_event == GUI_KEY_DOWN)
            {
#if X_RAY_TYPE
                if(para_setting_state == PARA_SETTING_ALLOWED)
                {
                    set_system_para(system_para_focus);
                }else 
                {
                    if(system_para_focus == 2)
                        system_para_focus = 0;
                    else
                        //system_para_focus++;
                    system_para_focus += 2;
                    display_system_para_setting(system_para_focus);
                }
#else
                if(para_setting_state == PARA_SETTING_ALLOWED)
                {
                    set_system_para_maxRay(system_para_focus);
                }else 
                {
                    if(system_para_focus == 1)
                        system_para_focus = 0;
                    else
                        system_para_focus++;
                    display_system_para_setting_maxRay(system_para_focus);
                }
#endif                
                set_setting_timeout();
            }else if(key_event == GUI_KEY_ADULT_CHILD_LONG)
            {
                 jump_flag = 1;
                 display_system_setting_win(currentFocus);
            }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
            {
                if(para_setting_state == PARA_SETTING_ALLOWED)
                {
                    if( jump_flag == 1)
                    {
                        WIN = SystemSettingWin;
                        jump_flag = 0;
                    }else
                    {
    #if X_RAY_TYPE
                        //display_system_para_setting(system_para_focus);
                        display_system_para_setting_arrow_dispear(system_para_focus);
    #else
                        display_system_para_setting_arrow_maxRay_dispear(system_para_focus);
    #endif
                    }
                    para_setting_state = PARA_SETTING_NO_ALLOWED;
                    if(sys_exposoure_para.SleepTime != current_exposoure_para.SleepTime)
                    {
                        sys_exposoure_para.SleepTime = current_exposoure_para.SleepTime;
                        taskENTER_CRITICAL();
                        set_sys_exposoure_para(SLEEP_TIME,sys_exposoure_para.SleepTime);
                        taskEXIT_CRITICAL();
                    }
#if 0                    
//                    if(sys_exposoure_para.DAP != current_exposoure_para.DAP)
//                    {
//                        sys_exposoure_para.DAP = current_exposoure_para.DAP;
//                        taskENTER_CRITICAL();
//                        set_sys_exposoure_para(DAP,sys_exposoure_para.DAP);
//                        taskEXIT_CRITICAL();
//                    }
#endif                    
                    if(sys_exposoure_para.ScanMode != current_exposoure_para.ScanMode)
                    {
                        sys_exposoure_para.ScanMode = current_exposoure_para.ScanMode;
                        taskENTER_CRITICAL();
                        set_sys_exposoure_para(SCAN_MODE,sys_exposoure_para.ScanMode);
                        taskEXIT_CRITICAL();
                    }
                }else
                {
                    if( jump_flag == 1)
                    {
                        WIN = SystemSettingWin;
                        jump_flag = 0;
                    }else
                    {
                        para_setting_state = PARA_SETTING_ALLOWED;
    #if X_RAY_TYPE                    
                        //if(system_para_focus == 1)//DAP is not allowed to be midified by the user
                        //{
                        //    para_setting_state = PARA_SETTING_NO_ALLOWED;
                        //}else
                        {
                            display_system_para_setting_arrow(system_para_focus);
                        }
    #else
                        display_system_para_setting_arrow_maxRay(system_para_focus);
    #endif                    
                    }
                }
                set_setting_timeout();
            }
            else if(key_event == GUI_SETTING_TIME_OUT)
            {
                 MainWindowDisplayFunc();
                 change_state(IDLE_STATE);
                 WIN = MainWindowDisplay;
            }else if(key_event == GUI_KEY_POWER_LONG)
            {
                powerOff_displayFunc();
                osDelay(3000);
                para_data_store();
                system_power_off();
                //poweroff_state = 1;
            }
            key_event = 0;
            break;
        case SystemResetWin:
            if(key_event == GUI_KEY_UP)
            {
                if(system_reset_focus == 0)
                    system_reset_focus = 1;
                else
                    system_reset_focus--;
                display_system_reset_win(system_reset_focus);
                set_setting_timeout();
            }else if(key_event == GUI_KEY_DOWN)
            {
                if(system_reset_focus == 1)
                    system_reset_focus = 0;
                else
                    system_reset_focus++;
                display_system_reset_win(system_reset_focus);
                set_setting_timeout();
            }else if(key_event == GUI_KEY_ADULT_CHILD)
            {
                if(system_reset_focus == 0)//yes
                {
                    taskENTER_CRITICAL();
                     __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
                    fagctory_default_store();
                    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
                    system_para_restore();
                    __HAL_IWDG_RELOAD_COUNTER(&hiwdg);
                    taskEXIT_CRITICAL();
                    Buzzer_Key();
                    tooth_position = LEFT_UP_1;

                }
                display_system_setting_win(currentFocus);
                set_setting_timeout();
            }else if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
            {
                WIN = SystemSettingWin;
                set_setting_timeout();
            }else if(key_event == GUI_SETTING_TIME_OUT)
            {
                 MainWindowDisplayFunc();
                 change_state(IDLE_STATE);
                 WIN = MainWindowDisplay;
            }else if(key_event == GUI_KEY_POWER_LONG)
            {
                powerOff_displayFunc();
                osDelay(3000);
                para_data_store();
                system_power_off();
                //poweroff_state = 1;
            }
            key_event = 0;            
            break;
        default:
            break;
    }
}

void display_system_setting_win(u8 item)
{
    LCD_Clear(DarkGRAY);
 
    switch(item)
    {
        case 0:
            drawRec(FOCUS_X,FOCUS_X+FOCUS_L,FOCUS_Y-60,FOCUS_Y+FOCUS_W-60,NOVAGREEN);    
            pictureDisplay(ARROW_X,ARROW_L,ARROW_Y-60,ARROW_W,gImage_arrow);
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y,SYSTEM_MENU_W_GREEN,gImage_systemInfor_yb);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+60,SYSTEM_MENU_W_GREEN-1,gImage_Event );
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+60,LINE_Y+LINE_W+60,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+120,SYSTEM_MENU_W,gImage_systemSettingWhite);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+120,LINE_Y+LINE_W+120,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+180,SYSTEM_MENU_W,gImage_systemReset);
            
            break;
        case 1:        
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y,SYSTEM_MENU_W_GREEN,gImage_systemInfor_bw);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);
            drawRec(FOCUS_X,FOCUS_X+FOCUS_L,FOCUS_Y-10,FOCUS_Y+FOCUS_W,NOVAGREEN);    
            pictureDisplay(ARROW_X,ARROW_L,ARROW_Y,ARROW_W,gImage_arrow);
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+60,SYSTEM_MENU_W_GREEN,gImage_EventGreen);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+60,LINE_Y+LINE_W+60,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+120,SYSTEM_MENU_W,gImage_systemSettingWhite);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+120,LINE_Y+LINE_W+120,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+180,SYSTEM_MENU_W,gImage_systemReset);
            break;
        case 2:
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y,SYSTEM_MENU_W_GREEN,gImage_systemInfor_bw);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);

            drawRec(FOCUS_X,FOCUS_X+FOCUS_L,FOCUS_Y+50,FOCUS_Y+FOCUS_W+60,NOVAGREEN);    
            pictureDisplay(ARROW_X,ARROW_L,ARROW_Y+60,ARROW_W,gImage_arrow);
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+60,SYSTEM_MENU_W-1,gImage_Event );
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+60,LINE_Y+LINE_W+60,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+120,SYSTEM_MENU_W_GREEN,gImage_systemSettingGreen);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+120,LINE_Y+LINE_W+120,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+180,SYSTEM_MENU_W,gImage_systemReset);
            break;
        case 3:
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y,SYSTEM_MENU_W_GREEN,gImage_systemInfor_bw);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);

            drawRec(FOCUS_X,FOCUS_X+FOCUS_L,FOCUS_Y+110,FOCUS_Y+FOCUS_W+120,NOVAGREEN);    
            pictureDisplay(ARROW_X,ARROW_L,ARROW_Y+120,ARROW_W,gImage_arrow);
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+60,SYSTEM_MENU_W-1,gImage_Event);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+60,LINE_Y+LINE_W+60,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+120,SYSTEM_MENU_W,gImage_systemSettingWhite);
            drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+120,LINE_Y+LINE_W+120,LINECOLOR);    
            pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y+180,SYSTEM_MENU_W_GREEN,gImage_systemResetGreen);
            break;
        default:
            break;
        
    }
}

void display_system_infor_win(void)
{
    u8 pa[5] = {0,0,0,0,0};
    LCD_Clear(DarkGRAY);
    pictureDisplay(SYSTEM_MENU_X,SYSTEM_MENU_L,SYSTEM_MENU_Y,SYSTEM_MENU_W,gImage_systemInfor_by);
    drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);   
#if X_RAY_TYPE
    pictureDisplay(EXPOSURE_CI_X,EXPOSURE_CI_L,EXPOSURE_CI_Y,EXPOSURE_CI_W,gImage_exposuretimes_by);
    pa[4] = current_exposoure_para.ExposeCount/10000;
    pa[3] = current_exposoure_para.ExposeCount/1000%10;
    pa[2] = current_exposoure_para.ExposeCount/100%10;
    pa[1] = current_exposoure_para.ExposeCount/10%10;
    pa[0] = current_exposoure_para.ExposeCount%10;
    
    
    pictureDisplay(SYSINFO_NUM_X,SYSINFO_NUM_L,EXPOSURE_CI_Y+45,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[4]]);
    pictureDisplay(SYSINFO_NUM_X+16*1,SYSINFO_NUM_L,EXPOSURE_CI_Y+45,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[3]]);
    pictureDisplay(SYSINFO_NUM_X+16*2,SYSINFO_NUM_L,EXPOSURE_CI_Y+45,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[2]]);
    pictureDisplay(SYSINFO_NUM_X+16*3,SYSINFO_NUM_L,EXPOSURE_CI_Y+45,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[1]]);
    pictureDisplay(SYSINFO_NUM_X+16*4,SYSINFO_NUM_L,EXPOSURE_CI_Y+45,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[0]]);

    pictureDisplay(SYSINFO_NUM_X+16*6,31,EXPOSURE_CI_Y+45,SYSINFO_NUM_W,gImage_ci_by);
    drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+90,LINE_Y+LINE_W+90,LINECOLOR);   

    pictureDisplay(EXPOSURE_CI_X,123,EXPOSURE_CI_Y+90,35,gImage_DAPAcc_by);

    pa[4] = current_exposoure_para.DAPAccu/10000;
    pa[3] = current_exposoure_para.DAPAccu/1000%10;
    pa[2] = current_exposoure_para.DAPAccu/100%10;
    pa[1] = current_exposoure_para.DAPAccu/10%10;
    pa[0] = current_exposoure_para.DAPAccu%10;
    pictureDisplay(DAPACC_NUM_X,DAPACC_NUM_L,DAPACC_NUM_Y,DAPACC_NUM_W,&num0_9_by[numSize_by*pa[4]]);
    pictureDisplay(DAPACC_NUM_X+16*1,DAPACC_NUM_L,DAPACC_NUM_Y,DAPACC_NUM_W,&num0_9_by[numSize_by*pa[3]]);
    pictureDisplay(DAPACC_NUM_X+16*2,DAPACC_NUM_L,DAPACC_NUM_Y,DAPACC_NUM_W,&num0_9_by[numSize_by*pa[2]]);
    pictureDisplay(DAPACC_NUM_X+16*3,DAPACC_NUM_L,DAPACC_NUM_Y,DAPACC_NUM_W,&num0_9_by[numSize_by*pa[1]]);
    pictureDisplay(DAPACC_NUM_X+16*4,DAPACC_NUM_L,DAPACC_NUM_Y,DAPACC_NUM_W,&num0_9_by[numSize_by*pa[0]]);
    pictureDisplay(DAPACC_NUM_X+16*7,124,DAPACC_NUM_Y,37,gImage_mGycm2_by);
#else
    pictureDisplay(EXPOSURE_CI_X,EXPOSURE_CI_L,EXPOSURE_CI_Y+66,EXPOSURE_CI_W,gImage_exposuretimes_by);
    pa[4] = current_exposoure_para.ExposeCount/10000;
    pa[3] = current_exposoure_para.ExposeCount/1000%10;
    pa[2] = current_exposoure_para.ExposeCount/100%10;
    pa[1] = current_exposoure_para.ExposeCount/10%10;
    pa[0] = current_exposoure_para.ExposeCount%10;
    
    
    pictureDisplay(SYSINFO_NUM_X,SYSINFO_NUM_L,EXPOSURE_CI_Y+66,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[4]]);
    pictureDisplay(SYSINFO_NUM_X+16*1,SYSINFO_NUM_L,EXPOSURE_CI_Y+66,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[3]]);
    pictureDisplay(SYSINFO_NUM_X+16*2,SYSINFO_NUM_L,EXPOSURE_CI_Y+66,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[2]]);
    pictureDisplay(SYSINFO_NUM_X+16*3,SYSINFO_NUM_L,EXPOSURE_CI_Y+66,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[1]]);
    pictureDisplay(SYSINFO_NUM_X+16*4,SYSINFO_NUM_L,EXPOSURE_CI_Y+66,SYSINFO_NUM_W,&num0_9_by[numSize_by*pa[0]]);

    pictureDisplay(SYSINFO_NUM_X+16*6,31,EXPOSURE_CI_Y+66,SYSINFO_NUM_W,gImage_ci_by);
#endif


}

void display_system_para_setting(u8 focus)
{
    u8 para[3] = {0,0,0};
    LCD_Clear(DarkGRAY);
    
    pictureDisplay(SYSTEM_PARA_SETTING_X,SYSTEM_PARA_SETTING_L,SYSTEM_PARA_SETTING_Y,SYSTEM_PARA_SETTING_W,gImage_systemSetting_by);
    drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y,S_LINE_Y+S_LINE_W,LINECOLOR); 
    switch(focus)
    {
        case 0:
            drawRec(SYSPARA_FOCUS_X,SYSPARA_FOCUS_X+SYSPARA_FOCUS_L,SYSPARA_FOCUS_Y,SYSPARA_FOCUS_Y+SYSPARA_FOCUS_W,NOVAGREEN);    
            //pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y,SYSPARA_ARROW_W,gImage_arrow);
            pictureDisplay(SLEEP_TIME_SETTING_X,SLEEP_TIME_SETTING_L,SLEEP_TIME_SETTING_Y-4,SLEEP_TIME_SETTING_W,gImage_sleeptime_yb);
            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;

            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,gImage_s_yb);
        
            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET,LINECOLOR); 
            
            para[2] = current_exposoure_para.DAP /100;
            para[1] = current_exposoure_para.DAP/10%10;
            para[0] = current_exposoure_para.DAP%10;

            pictureDisplay(SLEEP_TIME_SETTING_X,DAPPA_SETTING_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET-4,DAPPA_SETTING_W,gImage_DAPPa_bw);
        
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_bw[numSize_bw*10]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[0]]);
        
        
            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET*2,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET*2,LINECOLOR); 
            pictureDisplay(DIGITAL_MODE_X,DIGITAL_MODE_L,DIGITAL_MODE_Y,DIGITAL_MODE_W,gImage_digitalmode_bw);
            if((current_exposoure_para.ScanMode & (1<<2)) == 0)
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_by);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_bw);
            }else
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_bw);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_by);
            }

            break;
        case 1:
            drawRec(SYSPARA_FOCUS_X,SYSPARA_FOCUS_X+SYSPARA_FOCUS_L,SYSPARA_FOCUS_Y+SYSTEM_OFFSET,SYSPARA_FOCUS_Y+SYSPARA_FOCUS_W+SYSTEM_OFFSET,NOVAGREEN);    
            //pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET,SYSPARA_ARROW_W,gImage_arrow);
            pictureDisplay(SLEEP_TIME_SETTING_X,SLEEP_TIME_SETTING_L,SLEEP_TIME_SETTING_Y-4,SLEEP_TIME_SETTING_W,gImage_sleeptime_bw);

            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;

            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,gImage_s_bw);
        
            para[2] = current_exposoure_para.DAP /100;
            para[1] = current_exposoure_para.DAP/10%10;
            para[0] = current_exposoure_para.DAP%10;

            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET,LINECOLOR); 
            pictureDisplay(SLEEP_TIME_SETTING_X,DAPPA_SETTING_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET-4,DAPPA_SETTING_W,gImage_DAPPa_yb);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*10]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);

            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET*2,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET*2,LINECOLOR); 
            pictureDisplay(DIGITAL_MODE_X,DIGITAL_MODE_L,DIGITAL_MODE_Y,DIGITAL_MODE_W,gImage_digitalmode_bw);
            if((current_exposoure_para.ScanMode & (1<<2)) == 0)
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_by);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_bw);
            }else
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_bw);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_by);
            }

            break;
        case 2:
            drawRec(SYSPARA_FOCUS_X,SYSPARA_FOCUS_X+SYSPARA_FOCUS_L,SYSPARA_FOCUS_Y+SYSTEM_OFFSET*2,SYSPARA_FOCUS_Y+SYSPARA_FOCUS_W*2+SYSTEM_OFFSET*2,NOVAGREEN);    
            //pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET,SYSPARA_ARROW_W,gImage_arrow);
            pictureDisplay(SLEEP_TIME_SETTING_X,SLEEP_TIME_SETTING_L,SLEEP_TIME_SETTING_Y-4,SLEEP_TIME_SETTING_W,gImage_sleeptime_bw);

            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;

            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W-1,gImage_s_bw);
        
            para[2] = current_exposoure_para.DAP /100;
            para[1] = current_exposoure_para.DAP/10%10;
            para[0] = current_exposoure_para.DAP%10;

            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET,LINECOLOR); 
            pictureDisplay(SLEEP_TIME_SETTING_X,DAPPA_SETTING_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET-4,DAPPA_SETTING_W,gImage_DAPPa_bw);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_bw[numSize_bw*10]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[0]]);


            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET*2,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET*2,LINECOLOR); 
            pictureDisplay(DIGITAL_MODE_X,DIGITAL_MODE_L,DIGITAL_MODE_Y,DIGITAL_MODE_W,gImage_digitalmode_yb);
            if((current_exposoure_para.ScanMode & (1<<2)) == 0)
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yb);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yw);
            }else
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yw);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yb);
            }
        
            break;
        default:
            break;
    }
    
}

void display_system_para_setting_maxRay(u8 focus)
{
    u8 para[3] = {0,0,0};
    LCD_Clear(DarkGRAY);
    
    pictureDisplay(SYSTEM_PARA_SETTING_X,SYSTEM_PARA_SETTING_L,SYSTEM_PARA_SETTING_Y,SYSTEM_PARA_SETTING_W,gImage_systemSetting_by);
    drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y,S_LINE_Y+S_LINE_W,LINECOLOR); 
    switch(focus)
    {
        case 0:
            drawRec(SYSPARA_FOCUS_X,SYSPARA_FOCUS_X+SYSPARA_FOCUS_L,SYSPARA_FOCUS_Y,SYSPARA_FOCUS_Y+SYSPARA_FOCUS_W*2,NOVAGREEN);    
            //pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y,SYSPARA_ARROW_W,gImage_arrow);
            pictureDisplay(SLEEP_TIME_SETTING_X,SLEEP_TIME_SETTING_L,SLEEP_TIME_SETTING_Y+20,SLEEP_TIME_SETTING_W,gImage_sleeptime_yb);
            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;

            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,gImage_s_yb);
        
            pictureDisplay(DIGITAL_MODE_X,DIGITAL_MODE_L,DIGITAL_MODE_Y,DIGITAL_MODE_W,gImage_digitalmode_bw);
            if((current_exposoure_para.ScanMode & (1<<2)) == 0)
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_by);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_bw);
            }else
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_bw);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_by);
            }

            break;

        case 1:
            drawRec(SYSPARA_FOCUS_X,SYSPARA_FOCUS_X+SYSPARA_FOCUS_L,SYSPARA_FOCUS_Y+SYSTEM_OFFSET*2,SYSPARA_FOCUS_Y+SYSPARA_FOCUS_W*2+SYSTEM_OFFSET*2,NOVAGREEN);    
            //pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET,SYSPARA_ARROW_W,gImage_arrow);
            pictureDisplay(SLEEP_TIME_SETTING_X,SLEEP_TIME_SETTING_L,SLEEP_TIME_SETTING_Y+20,SLEEP_TIME_SETTING_W,gImage_sleeptime_bw);

            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;

            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W-1,gImage_s_bw);
        

            drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y+SYSTEM_OFFSET*2,S_LINE_Y+S_LINE_W+SYSTEM_OFFSET*2,LINECOLOR); 
            pictureDisplay(DIGITAL_MODE_X,DIGITAL_MODE_L,DIGITAL_MODE_Y,DIGITAL_MODE_W,gImage_digitalmode_yb);
            if((current_exposoure_para.ScanMode & (1<<2)) == 0)
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yb);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yw);
            }else
            {
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yw);
                pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yb);
            }
        
            break;
        default:
            break;
    }
    
}


void display_system_para_setting_arrow(u8 focus)
{
    switch(focus)
    {
        case 0:
            pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y,SYSPARA_ARROW_W,gImage_arrow);
            break;
        case 1:
            pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET,SYSPARA_ARROW_W,gImage_arrow);
            break;
        case 2:
            pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET*3-24,SYSPARA_ARROW_W,gImage_arrow);
            break;
        default:
            break;
    }    
}

void display_system_para_setting_arrow_dispear(u8 focus)
{
    switch(focus)
    {
        case 0:
            drawRec(SYSPARA_ARROW_X,SYSPARA_ARROW_X+SYSPARA_ARROW_L,SYSPARA_ARROW_Y,SYSPARA_ARROW_Y+SYSPARA_ARROW_W,NOVAGREEN);
            break;
        case 1:
            drawRec(SYSPARA_ARROW_X,SYSPARA_ARROW_X+SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET,SYSPARA_ARROW_Y+SYSTEM_OFFSET+SYSPARA_ARROW_W,NOVAGREEN);
            break;
        case 2:
            drawRec(SYSPARA_ARROW_X,SYSPARA_ARROW_X+SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET*3-24,SYSPARA_ARROW_Y+SYSTEM_OFFSET*3-24+SYSPARA_ARROW_W,NOVAGREEN);
            break;
        default:
            break;
    }    
}

void display_system_para_setting_arrow_maxRay(u8 focus)
{
    switch(focus)
    {
        case 0:
            pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+24,SYSPARA_ARROW_W,gImage_arrow);
            break;
        case 1:
            pictureDisplay(SYSPARA_ARROW_X,SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET*3-24,SYSPARA_ARROW_W,gImage_arrow);
            break;
        default:
            break;
    }    
}

void display_system_para_setting_arrow_maxRay_dispear(u8 focus)
{
    switch(focus)
    {
        case 0:
            drawRec(SYSPARA_ARROW_X,SYSPARA_ARROW_X+SYSPARA_ARROW_L,SYSPARA_ARROW_Y+24,SYSPARA_ARROW_Y+24+SYSPARA_ARROW_W,NOVAGREEN);
            break;
        case 1:
            drawRec(SYSPARA_ARROW_X,SYSPARA_ARROW_X+SYSPARA_ARROW_L,SYSPARA_ARROW_Y+SYSTEM_OFFSET*3-24,SYSPARA_ARROW_Y+SYSTEM_OFFSET*3-24+SYSPARA_ARROW_W,NOVAGREEN);
            break;
        default:
            break;
    }    
}

void set_system_para(u8 focus)
{
    u8 para[3] = {0,0,0};
    switch(focus)
    {
        case 0:
            if(key_event == GUI_KEY_UP)
            {
                if(current_exposoure_para.SleepTime < MAX_SLEEP_DURATION)
                    current_exposoure_para.SleepTime +=5;
            }else if(key_event == GUI_KEY_DOWN)
            {
                if(current_exposoure_para.SleepTime > MIN_SLEEP_DUARTION)
                {
                    current_exposoure_para.SleepTime -=5;
                }
            }
            
            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;
            
            drawRec(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L+SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SLEEP_TIME_SETTING_Y-4,SLEEP_TIME_SETTING_Y-4+SYSINFO_NUM_W, NOVAGREEN);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4,SYSINFO_NUM_W,gImage_s_yb);

            break;
        case 1:
            if(key_event == GUI_KEY_UP)
            {
                if(current_exposoure_para.DAP < MAX_DAP)
                    current_exposoure_para.DAP ++;
            }else if(key_event == GUI_KEY_DOWN)
            {
                if(current_exposoure_para.DAP > MIN_DAP)
                {
                    current_exposoure_para.DAP --;
                }
            }
            
            para[2] = current_exposoure_para.DAP /100;
            para[1] = current_exposoure_para.DAP/10%10;
            para[0] = current_exposoure_para.DAP%10;
            
            drawRec(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3+SYSINFO_NUM_L*4,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET+SYSINFO_NUM_W, NOVAGREEN);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*10]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
            break;
        case 2:
            if((key_event == GUI_KEY_UP) || (key_event == GUI_KEY_DOWN))
            {
                if(current_exposoure_para.ScanMode & (1<<2))//sensor
                {
                    current_exposoure_para.ScanMode &= ~(1<<2);

                    drawRec(YAPIANBAO_X,YAPIANBAO_X+YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2+YAPIANBAO_W*2+17,NOVAGREEN);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yb);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yw); 

                }
                else
                {
                    current_exposoure_para.ScanMode |= (1<<2);
                    drawRec(YAPIANBAO_X,YAPIANBAO_X+YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2+YAPIANBAO_W*2+17,NOVAGREEN);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yw);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yb); 
                }
            }
            break;
        default:
            break;
    }    
}

void set_system_para_maxRay(u8 focus)
{
    u8 para[3] = {0,0,0};
    switch(focus)
    {
        case 0:
            if(key_event == GUI_KEY_UP)
            {
                if(current_exposoure_para.SleepTime < MAX_SLEEP_DURATION)
                    current_exposoure_para.SleepTime +=5;
            }else if(key_event == GUI_KEY_DOWN)
            {
                if(current_exposoure_para.SleepTime > MIN_SLEEP_DUARTION)
                {
                    current_exposoure_para.SleepTime -=5;
                }
            }
            
            para[2] = current_exposoure_para.SleepTime/100;
            para[1] = current_exposoure_para.SleepTime/10%10;
            para[0] = current_exposoure_para.SleepTime%10;
            
            drawRec(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L+SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SLEEP_TIME_SETTING_Y+20,SLEEP_TIME_SETTING_Y+20+SYSINFO_NUM_W, NOVAGREEN);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*2,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
            pictureDisplay(SLEEP_TIME_SETTING_X+SLEEP_TIME_SETTING_L+16*5,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y+20,SYSINFO_NUM_W,gImage_s_yb);

            break;

        case 1:
            if((key_event == GUI_KEY_UP) || (key_event == GUI_KEY_DOWN))
            {
                if(current_exposoure_para.ScanMode & (1<<2))//sensor
                {
                    current_exposoure_para.ScanMode &= ~(1<<2);

                    drawRec(YAPIANBAO_X,YAPIANBAO_X+YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2+YAPIANBAO_W*2+17,NOVAGREEN);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yb);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yw); 

                }
                else
                {
                    current_exposoure_para.ScanMode |= (1<<2);
                    drawRec(YAPIANBAO_X,YAPIANBAO_X+YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2+YAPIANBAO_W*2+17,NOVAGREEN);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*2,YAPIANBAO_W,gImage_yapianbao_yw);
                    pictureDisplay(YAPIANBAO_X,YAPIANBAO_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET*3,YAPIANBAO_W,gImage_chuanganqi_yb); 
                }
            }
            break;
        default:
            break;
    }    
}


void display_event_record()
{
    //read record
    u8 record[20] = {0};
    if(eventData.EventCount<EVENT_NUMBER)
    {
        for(u8 i = 0;i<eventData.EventCount;i++)
            record[i] = eventData.data[eventData.EventCount-i-1];
    }else
    {
        for(u8 i = 0;i<EVENT_NUMBER;i++)
        {
            if(i< eventData.NextToUpdate)
                record[i] = eventData.data[eventData.NextToUpdate-i-1];
            else
                record[i] = eventData.data[EVENT_NUMBER+eventData.NextToUpdate-i-1];
        }
    }
    LCD_Clear(DarkGRAY);
    pictureDisplay(EVENT_RECORD_X,EVENT_RECORD_L,EVENT_RECORD_Y,EVENT_RECORD_W,gImage_eventRecord_by);
    drawRec(S_LINE_X,S_LINE_X+S_LINE_L,S_LINE_Y,S_LINE_Y+S_LINE_W,LINECOLOR); 
    switch(event_record_page)
    {
        case 0:
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            display_event_record_each(EVENT_X,EVENT_Y,record[0]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*2]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*1,record[1]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*3]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*2,record[2]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*4]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*3,record[3]);
            break;
        case 1:
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*5]);
            display_event_record_each(EVENT_X,EVENT_Y,record[4]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*6]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*1,record[5]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*7]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*2,record[6]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*8]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*3,record[7]);            
            break;
        case 2:
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*9]);
            display_event_record_each(EVENT_X,EVENT_Y,record[8]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*0]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*1,record[9]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*2,record[10]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*2]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*3,record[11]);            
           break;
        case 3:
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*3]);
            display_event_record_each(EVENT_X,EVENT_Y,record[12]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*4]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*1,record[13]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*5]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*2,record[14]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*6]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*3,record[15]);            
           break;
        case 4:
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*7]);
            display_event_record_each(EVENT_X,EVENT_Y,record[16]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*8]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*1,record[17]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*1]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*2,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*9]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*2,record[18]);
            pictureDisplay(EVENT_NUMBER_X,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*2]);
            pictureDisplay(EVENT_NUMBER_X+EVENT_NUMBER_L,EVENT_NUMBER_L,EVENT_NUMBER_Y+SYSTEM_OFFSET*3,EVENT_NUMBER_W,&num0_9_bw[numSize_bw*0]);
            display_event_record_each(EVENT_X,EVENT_Y+SYSTEM_OFFSET*3,record[19]);            
        default:
            break;
    }
}

void display_event_record_each(u16 x, u16 y, u8 event)
{
    switch(event)
    {
        case POWER_ON:
            pictureDisplay(x,EVENT_L,y,EVENT_W,gImage_kai_bw);
            pictureDisplay(x+EVENT_L,EVENT_L,y,EVENT_W,gImage_ji_bw);
            break;
        case POWER_OFF:
            pictureDisplay(x,EVENT_L,y,EVENT_W,gImage_guan_bw);
            pictureDisplay(x+EVENT_L,EVENT_L,y,EVENT_W,gImage_ji_bw);
            break;
        case SLEEP_MODE:
            pictureDisplay(x,EVENT_L_SLEEP,y,EVENT_W_SLEEP,gImage_xiumian_bw);
            break;
        case EXPOSUREDURATION:
            pictureDisplay(x,16,y,EVENT_W,gImage_E_bw);
            pictureDisplay(x+16,16,y,EVENT_W,&num0_9_bw[2*numSize_bw]);
            pictureDisplay(x+32,16,y,EVENT_W,&num0_9_bw[0*numSize_bw]);
            pictureDisplay(x+48,EVENT_L,y,EVENT_W,gImage_bao_by);
            pictureDisplay(x+48+EVENT_L,EVENT_L,y,EVENT_W,gImage_guang_bw);
            pictureDisplay(x+48+EVENT_L*2,EVENT_L,y,EVENT_W,gImage_bao_2_bw);
            pictureDisplay(x+48+EVENT_L*3,EVENT_L,y,EVENT_W,gImage_cuo_bw);
            break;
        case ERRORINFO:
            pictureDisplay(x,EVENT_L,y,EVENT_W,gImage_bao_2_bw);
            pictureDisplay(x+EVENT_L,EVENT_L,y,EVENT_W,gImage_cuo_bw);
            break;
        case BATTERY_TOO_LOW:
            pictureDisplay(x,16,y,EVENT_W,gImage_E_bw);
            pictureDisplay(x+16,16,y,EVENT_W,&num0_9_bw[1*numSize_bw]);
            pictureDisplay(x+32,16,y,EVENT_W,&num0_9_bw[0*numSize_bw]);
            pictureDisplay(x+48,EVENT_L,y,EVENT_W,gImage_bao_2_bw);
            pictureDisplay(x+48+EVENT_L,EVENT_L,y,EVENT_W,gImage_cuo_bw);
        default:
            break;
    }
}


void display_system_reset_win(u8 focus)
{
    LCD_Clear(DarkGRAY);
    pictureDisplay(SYSTEM_RESET_X,SYSTEM_RESET_L,SYSTEM_RESET_Y,SYSTEM_RESET_W,gImage_systemreset_by);
    if(focus == 0)
    {
        drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);  
        drawRec(YES_FOCUS_X,YES_FOCUS_X+YES_FOCUS_L,YES_FOCUS_Y,YES_FOCUS_Y+YES_FOCUS_W,NOVAGREEN);    
        pictureDisplay(SYSRESET_ARROW_X,SYSRESET_ARROW_L,SYSRESET_ARROW_Y,SYSRESET_ARROW_W,gImage_arrow);
        pictureDisplay(YES_X,YES_L,YES_Y,YES_W_by,gImage_yes_by);
        drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+90,LINE_Y+LINE_W+90,LINECOLOR);   
        pictureDisplay(NO_X,NO_L,NO_Y,NO_W_bw,gImage_No_bw);
    }else if(focus == 1)
    {
        drawRec(LINE_X,LINE_X+LINE_L,LINE_Y,LINE_Y+LINE_W,LINECOLOR);  
        drawRec(YES_FOCUS_X,YES_FOCUS_Y+YES_FOCUS_L,YES_FOCUS_Y+90,YES_FOCUS_Y+YES_FOCUS_W+90,NOVAGREEN);    
        pictureDisplay(SYSRESET_ARROW_X,SYSRESET_ARROW_L,SYSRESET_ARROW_Y+90,SYSRESET_ARROW_W,gImage_arrow);
        pictureDisplay(YES_X,YES_L,YES_Y,YES_W_bw,gImage_yes_bw);
        drawRec(LINE_X,LINE_X+LINE_L,LINE_Y+90,LINE_Y+LINE_W+90,LINECOLOR);   
        pictureDisplay(NO_X,NO_L,NO_Y,NO_W_by,gImage_No_by);
    }
}

void change_state(SYSTEM_STATE state)
{
    system_state = state;
    return;
}

void change_exposuring_state(EXPOSURE_PROCESS state)
{
    exposuring_state = state;
}

void change_demo_exposuring_state(EXPOSURE_PROCESS state)
{
    demo_exposuring_state = state;
}
void LED_init(u8 state)
{
    if(state == LED_ON)
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);//led 3
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);//led 4
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);//led 5
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);//led 6
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);//led 3
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);//led 4
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);//led 5
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);//led 6
    }else
    {
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);//led 3
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);//led 4
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);//led 5
        HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);//led 6
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);//led 3
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);//led 4
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);//led 5
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);//led 6
    }        
}


void LED_Control(u8 led, u8 state)
{
    switch(led)
    {
        case TOOTH_UP_1:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);//led 3
            }else
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);//led 3
            }
            break;
        case TOOTH_UP_2:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);//led 3
            }else
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);//led 3
            }
            break;
        case TOOTH_UP_3:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_RESET);//led 3
            }else
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_10, GPIO_PIN_SET);//led 3
            }
            
            break;
        case TOOTH_UP_4:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);//led 3
            }else
            {
                HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);//led 3
            }
            break;
        case TOOTH_DOWN_1:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
            }else
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
            }
            break;
        case TOOTH_DOWN_2:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
            }else
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
            }
            break;
        case TOOTH_DOWN_3:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
            }else
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
            }
            break;
        case TOOTH_DOWN_4:
            if(state == LED_OFF)
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);
            }else
            {
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);
            }
            break;
        default:
            break;
    }
}

void set_sleep_timer(void)
{
    taskENTER_CRITICAL();
    sleep_timer = current_exposoure_para.SleepTime*1000;
    taskEXIT_CRITICAL();
}

u32 get_sleep_timer(void)
{
    return sleep_timer;
}

//static u16 test_data = 0;
static u8 get_battery_charge_state(void)
{
    u16 data = 0;
    u16 data2 = 0;
    data = Get_Adc_Average(ADAPTER_VOLTAGE,3);
    data2 = Get_Adc_Average(BATTERY_VOLTAGE,3);
    //test_data = Get_Adc_Average(ADAPTER_VOLTAGE,3);
    switch(battery_charge_state)
    {
        case BATTERY_NO_CHARGE:
            if(data >= ADAPTER_PLUG)
            {
                return BATTERY_CHARGING;
            }else
                return BATTERY_NO_CHARGE;
            break;
        case BATTERY_CHARGING:
            if(data2 >= CHARGE_FULL)
                return BATTERY_CHARGING_FULL;
            else if(data >= ADAPTER_PLUG)
                return BATTERY_CHARGING;
            else
                return BATTERY_NO_CHARGE;
            break;
        case BATTERY_CHARGING_FULL:
            if(data >= ADAPTER_PLUG)
                return BATTERY_CHARGING_FULL;
            else
                return BATTERY_NO_CHARGE;
            break;
    }
}
BATTERY_STATE update_battery_status(void)
{
    u16 data = 0;
    data = Get_Adc_Average(BATTERY_VOLTAGE,3);
    switch(battery_charge_state)
    {
        case BATTERY_NO_CHARGE:
            switch(currentBatteryState)    
            {
                case FOUR_GRID_DISPLAY:
                    if(data < THREE_GRID)
                        return THREE_GRID_DISPLAY;
                    else
                        return FOUR_GRID_DISPLAY;
                    break;
                case THREE_GRID_DISPLAY:
                    if(data < TWO_GRID)
                        return TWO_GRID_DISPLAY;
                    else
                        return THREE_GRID_DISPLAY;
                    break;
                case TWO_GRID_DISPLAY:
                    if(data < ONE_GRID)
                        return ONE_GRID_DISPLAY;
                    else
                        return TWO_GRID_DISPLAY;
                    break;
                case ONE_GRID_DISPLAY:
                    if(data < ZERO_GRID)
                        return ZERO_GRID_DISPLAY;
                    else 
                        return ONE_GRID_DISPLAY;
                    break;
                case ZERO_GRID_DISPLAY:
                    return ZERO_GRID_DISPLAY;
                    break;
                case GRID_DISPLAY_NO:
                    if(data >= THREE_GRID)
                    {
                        return FOUR_GRID_DISPLAY;
                    }else if(data >= TWO_GRID)
                    {
                        return THREE_GRID_DISPLAY;
                    }else if(data >= ONE_GRID)
                    {
                        return TWO_GRID_DISPLAY;
                    }else if(data >= ZERO_GRID)
                    {
                        return ONE_GRID_DISPLAY;
                    }else
                    {
                        return ZERO_GRID_DISPLAY;
                    }
                    break;
                default:
                    break;
            }
            break;
        case BATTERY_CHARGING:
        case BATTERY_CHARGING_FULL:
            switch(currentBatteryState)    
            {
                case FOUR_GRID_DISPLAY:
                    return FOUR_GRID_DISPLAY;
                    break;
                case THREE_GRID_DISPLAY:
                    if(data >= THREE_GRID_CHARGE)
                        return FOUR_GRID_DISPLAY;
                    else
                        return THREE_GRID_DISPLAY;
                    break;
                case TWO_GRID_DISPLAY:
                    if(data >= TWO_GRID_CHARGE)
                        return THREE_GRID_DISPLAY;
                    else
                        return TWO_GRID_DISPLAY;
                    break;
                case ONE_GRID_DISPLAY:
                    if(data >= ONE_GRID_CHARGE)
                        return TWO_GRID_DISPLAY;
                    else
                        return ONE_GRID_DISPLAY;
                    break;
                case ZERO_GRID_DISPLAY:
                    if(data>=ZERO_GRID_CHARGE)
                        return ONE_GRID_DISPLAY;
                    else
                        return ZERO_GRID_DISPLAY;
                    break;
                case GRID_DISPLAY_NO:
                    if(data >= THREE_GRID_CHARGE)
                    {
                        return FOUR_GRID_DISPLAY;
                    }else if(data >= TWO_GRID_CHARGE)
                    {
                        return THREE_GRID_DISPLAY;
                    }else if(data >= ONE_GRID_CHARGE)
                    {
                        return TWO_GRID_DISPLAY;
                    }else if(data >= ZERO_GRID_CHARGE)
                    {
                        return ONE_GRID_DISPLAY;
                    }else
                    {
                        return ZERO_GRID_DISPLAY;
                    }                    
                    break;
                default:
                    break;
            }
            break;
    }
 }    
void battery2low_state_process(void)
{
    if(battery_charge_state != BATTERY_NO_CHARGE)
    {
        MainWindowDisplayFunc();
        change_state(IDLE_STATE);
        WIN = MainWindowDisplay;
    }else if(key_event == GUI_KEY_POWER)
    {
         change_state(POWER_KEY_PRESSING_STATE);
    }else if(key_event == GUI_BATTERY2LOW_TIME_OUT)
    {
        powerOff_displayFunc();
        osDelay(3000);
        para_data_store();
        taskENTER_CRITICAL();
        event_Data_Store(BATTERY_TOO_LOW);
        taskEXIT_CRITICAL();
        
        system_power_off();
        poweroff_state = 1;
    }
    key_event = 0;
}

void display_battery_2_low(void)
{
    drawRec(BACKGROUND_WARNING_X,BACKGROUND_WARNING_X+BACKGROUND_WARNING_L,BACKGROUND_WARNING_Y,BACKGROUND_WARNING_Y+BACKGROUND_WARNING_W,NOVAGREEN); 
    pictureDisplay(BATTERY2LOW_X,BATTERY2LOW_L,BATTERY2LOW_Y,BATTERY2LOW_W ,gImage_battery_toolow);
    pictureDisplay(NEED2CHARGE_X,NEED2CHARGE_L,NEED2CHARGE_Y,NEED2CHARGE_W ,gImage_need_charge);
}

void display_battery_status(u8 grid)
{
    //read voltage
    switch(grid)
    {
        case ZERO_GRID_DISPLAY:
            pictureDisplay(BATTERY_X,BATTERY_L,BATTERY_Y,BATTERY_W ,battery0);
            break;
        case ONE_GRID_DISPLAY:
            pictureDisplay(BATTERY_X,BATTERY_L,BATTERY_Y,BATTERY_W ,battery1);
            break;
        case TWO_GRID_DISPLAY:
            pictureDisplay(BATTERY_X,BATTERY_L,BATTERY_Y,BATTERY_W ,battery2);
            break;
        case THREE_GRID_DISPLAY:
            pictureDisplay(BATTERY_X,BATTERY_L,BATTERY_Y,BATTERY_W ,battery3);
            break;
        case FOUR_GRID_DISPLAY:
            pictureDisplay(BATTERY_X,BATTERY_L,BATTERY_Y,BATTERY_W ,battery4);
            break;
        default:
            break;
    }
    if(battery_charge_state == BATTERY_CHARGING)
    {
        pictureDisplay(CHARGING_ICON_X,CHARGING_ICON_L,CHARGING_ICON_Y,CHARGING_ICON_W ,gImage_charging_icon);
    }else
    {
        drawRec(CHARGING_ICON_X,CHARGING_ICON_X+CHARGING_ICON_L,CHARGING_ICON_Y,CHARGING_ICON_Y+CHARGING_ICON_W,DarkGRAY);    
    }
}
void system_power_off(void)
{
    taskENTER_CRITICAL();
    event_Data_Store(POWER_OFF);
    taskEXIT_CRITICAL();
    LCD_BK_0;
    SYSTEM_POWER_OFF;
}

void LED_control(LED_COLOR color)
{
    switch(color)
    {
        case ORANGE_LED:
#if 1
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);//GPIO_PIN_RESET);//GPIO_PIN_SET);
#else 
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_RESET);//GPIO_PIN_SET);
        
#endif        
            break;
        case GREEN_LED:
#if 1
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);//GPIO_PIN_RESET);
#else
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_RESET);
#endif
            break;
        case BLUE_LED:
#if 1
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);//GPIO_PIN_RESET);
#else
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);//GPIO_PIN_SET);//GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);//GPIO_PIN_RESET);
        
#endif        
            break;
        default:
            break;
    }
}

void set_setting_timeout(void)
{
    taskENTER_CRITICAL();
    setting_timeout = SETTING_TIMEOUT_DUARTION;
    taskEXIT_CRITICAL();
}

u16 get_setting_timeout(void)
{
    u16 temp;
    taskENTER_CRITICAL();
    temp = setting_timeout;
    taskEXIT_CRITICAL();
   
    return temp;
}

void set_battery2low_timeout(void)
{
    taskENTER_CRITICAL();
    battery2low_timeout = BATTERY2LOW_POWEROFF;
    taskEXIT_CRITICAL();
}

u16 get_battery2low_timeout(void)
{
    u16 temp;
    taskENTER_CRITICAL();
    temp = battery2low_timeout;
    taskEXIT_CRITICAL();
    return temp;
}

u16 get_current_exposuretime(void)
{
    u8 temp;
    u8 scanType;
    
    scanType = get_scan_type(current_exposoure_para.ScanMode);
    temp = current_expose_time[scanType][tooth_position];
    return (expose_table[temp]);
}

u16 get_demo_exposuretime(void)
{
    u8 temp;
    u8 scanType;
 
#if 0    
    scanType = get_scan_type(current_exposoure_para.ScanMode);
    temp = current_expose_time[scanType][tooth_position];
#else
    
    scanType = get_scan_type(demo_exposoure_para.ScanMode);
    temp = demo_expose_time[scanType][demo_tooth_position];
#endif    
    return (expose_table[temp]);
}

void display_pwm_set(void)
{
    uint16_t offset_x;//offset_y;
    u16 temp[4] = {0,0,0,0};
    //u16 Full_Empty = 0;
    LCD_Clear(DarkGRAY);

    offset_x=80;

    temp[0] = pwm_current/1000;
    temp[1] = pwm_current/100%10;
    temp[2] = pwm_current/10%10;
    temp[3] = pwm_current%10;

    pictureDisplay(offset_x+16*0,16,60,31,&num0_9_bw[numSize_bw*temp[0]]);
    pictureDisplay(offset_x+16*1,16,60,31,&num0_9_bw[numSize_bw*temp[1]]);
    pictureDisplay(offset_x+16*2,16,60,31,&num0_9_bw[numSize_bw*temp[2]]);
    pictureDisplay(offset_x+16*3,16,60,31,&num0_9_bw[numSize_bw*temp[3]]);

    pictureDisplay(offset_x+16*4,46,60,37,gImage_mA_bw);
    
    temp[0] = pwm_current*1000/PWM_PERIOD_VALUE;
    temp[1] = temp[0]/100;
    temp[2] = temp[0]/10%10;
    temp[3] = temp[0]%10;

    pictureDisplay(offset_x+16*8,16,60,31,&num0_9_bw[numSize_bw*temp[1]]);
    pictureDisplay(offset_x+16*9,16,60,31,&num0_9_bw[numSize_bw*temp[2]]);
    pictureDisplay(offset_x+16*10,16,60,31,&num0_9_bw[numSize_bw*10]);
    pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,60,31,&num0_9_bw[numSize_bw*temp[3]]);

    
    
    if(calibration_focus == 0)
    {    
        pictureDisplay(40,SYSPARA_ARROW_L,62,SYSPARA_ARROW_W,gImage_arrow);
        drawRec(40,40+SYSPARA_ARROW_L,132,132+SYSPARA_ARROW_W,DarkGRAY);
    }
    else
    {
        pictureDisplay(40,SYSPARA_ARROW_L,132,SYSPARA_ARROW_W,gImage_arrow);
        drawRec(40,40+SYSPARA_ARROW_L,62,62+SYSPARA_ARROW_W,DarkGRAY);
   }
#if 0
    temp[0] = pwm_voltage/1000;
    temp[1] = pwm_voltage/100%10;
    temp[2] = pwm_voltage/10%10;
    temp[3] = pwm_voltage%10;
#else
    temp[0] = pwm_voltage_cali/1000;
    temp[1] = pwm_voltage_cali/100%10;
    temp[2] = pwm_voltage_cali/10%10;
    temp[3] = pwm_voltage_cali%10;
   
#endif
    //offset_x=124;
    //pictureDisplay(offset_x+16*0,16,190,32,&num0_9_bw[numSize_bw*6]);
    //pictureDisplay(offset_x+16*1,16,190,32,&num0_9_bw[numSize_bw*0]);
    pictureDisplay(offset_x+16*0,16,130,31,&num0_9_bw[numSize_bw*temp[0]]);
    pictureDisplay(offset_x+16*1,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
    pictureDisplay(offset_x+16*2,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
    pictureDisplay(offset_x+16*3,16,130,31,&num0_9_bw[numSize_bw*temp[3]]);

    pictureDisplay(offset_x+16*4,36,130,36,gImage_kV_bw);

    temp[0] = pwm_voltage_cali*1000/PWM_PERIOD_VALUE;
    temp[1] = temp[0]/100;
    temp[2] = temp[0]/10%10;
    temp[3] = temp[0]%10;

    pictureDisplay(offset_x+16*8,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
    pictureDisplay(offset_x+16*9,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
    pictureDisplay(offset_x+16*10,16,130,31,&num0_9_bw[numSize_bw*10]);
    pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,130,31,&num0_9_bw[numSize_bw*temp[3]]);

   
}

void calibration_state_process(void)
{
    u16 temp[4] = {0,0,0,0};
    uint16_t offset_x;//offset_y;

    if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
    {
        
        if(WIN == MainWindowDisplay)
        {
            change_state(IDLE_STATE);
        }else
        {
            if(calibration_focus == 0)
                calibration_focus = 1;
            else
                calibration_focus = 0;
            display_pwm_set();
        }
    }else if(key_event == GUI_KEY_UP)
    {
        if(calibration_focus == 0)
        {
            pwm_current += 5;//pwm_current++;

            offset_x=80;
            
            temp[0] = pwm_current/1000;
            temp[1] = pwm_current/100%10;
            temp[2] = pwm_current/10%10;
            temp[3] = pwm_current%10;

            pictureDisplay(offset_x+16*0,16,60,31,&num0_9_bw[numSize_bw*temp[0]]);
            pictureDisplay(offset_x+16*1,16,60,31,&num0_9_bw[numSize_bw*temp[1]]);
            pictureDisplay(offset_x+16*2,16,60,31,&num0_9_bw[numSize_bw*temp[2]]);
            pictureDisplay(offset_x+16*3,16,60,31,&num0_9_bw[numSize_bw*temp[3]]);   

#if 1            
            temp[0] = pwm_current*1000/PWM_PERIOD_VALUE;
            temp[1] = temp[0]/100;
            temp[2] = temp[0]/10%10;
            temp[3] = temp[0]%10;

            pictureDisplay(offset_x+16*8,16,60,31,&num0_9_bw[numSize_bw*temp[1]]);
            pictureDisplay(offset_x+16*9,16,60,31,&num0_9_bw[numSize_bw*temp[2]]);
            pictureDisplay(offset_x+16*10,16,60,31,&num0_9_bw[numSize_bw*10]);
            pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,60,31,&num0_9_bw[numSize_bw*temp[3]]);            
#endif            
            
        }else
        {
#if 0
            pwm_voltage++;
            offset_x=80;
            temp[0] = pwm_voltage/1000;
            temp[1] = pwm_voltage/100%10;
            temp[2] = pwm_voltage/10%10;
            temp[3] = pwm_voltage%10;
#else
            pwm_voltage_cali += 5;//pwm_voltage_cali++;
            offset_x=80;
            temp[0] = pwm_voltage_cali/1000;
            temp[1] = pwm_voltage_cali/100%10;
            temp[2] = pwm_voltage_cali/10%10;
            temp[3] = pwm_voltage_cali%10;
            
#endif
            pictureDisplay(offset_x+16*0,16,130,31,&num0_9_bw[numSize_bw*temp[0]]);
            pictureDisplay(offset_x+16*1,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
            pictureDisplay(offset_x+16*2,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
            pictureDisplay(offset_x+16*3,16,130,31,&num0_9_bw[numSize_bw*temp[3]]);  

#if 1            
            temp[0] = pwm_voltage_cali*1000/PWM_PERIOD_VALUE;
            temp[1] = temp[0]/100;
            temp[2] = temp[0]/10%10;
            temp[3] = temp[0]%10;

            pictureDisplay(offset_x+16*8,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
            pictureDisplay(offset_x+16*9,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
            pictureDisplay(offset_x+16*10,16,130,31,&num0_9_bw[numSize_bw*10]);
            pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,130,31,&num0_9_bw[numSize_bw*temp[3]]);
#endif            
        }
    }else if(key_event == GUI_KEY_DOWN)
    {
        if(calibration_focus == 0)
        {
            if(pwm_current >= 5)//if(pwm_current >0)
            {
                pwm_current -= 5;//pwm_current--;

                offset_x=80;
                
                temp[0] = pwm_current/1000;
                temp[1] = pwm_current/100%10;
                temp[2] = pwm_current/10%10;
                temp[3] = pwm_current%10;

                pictureDisplay(offset_x+16*0,16,60,31,&num0_9_bw[numSize_bw*temp[0]]);
                pictureDisplay(offset_x+16*1,16,60,31,&num0_9_bw[numSize_bw*temp[1]]);
                pictureDisplay(offset_x+16*2,16,60,31,&num0_9_bw[numSize_bw*temp[2]]);
                pictureDisplay(offset_x+16*3,16,60,31,&num0_9_bw[numSize_bw*temp[3]]);           

#if 1                
                temp[0] = pwm_current*1000/PWM_PERIOD_VALUE;
                temp[1] = temp[0]/100;
                temp[2] = temp[0]/10%10;
                temp[3] = temp[0]%10;

                pictureDisplay(offset_x+16*8,16,60,31,&num0_9_bw[numSize_bw*temp[1]]);
                pictureDisplay(offset_x+16*9,16,60,31,&num0_9_bw[numSize_bw*temp[2]]);
                pictureDisplay(offset_x+16*10,16,60,31,&num0_9_bw[numSize_bw*10]);
                pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,60,31,&num0_9_bw[numSize_bw*temp[3]]);            
#endif
            }            
            
        }else
        {
#if 0
            if(pwm_voltage > 0)
            {
                pwm_voltage--;
                offset_x=80;
                temp[0] = pwm_voltage/1000;
                temp[1] = pwm_voltage/100%10;
                temp[2] = pwm_voltage/10%10;
                temp[3] = pwm_voltage%10;

                pictureDisplay(offset_x+16*0,16,130,31,&num0_9_bw[numSize_bw*temp[0]]);
                pictureDisplay(offset_x+16*1,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
                pictureDisplay(offset_x+16*2,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
                pictureDisplay(offset_x+16*3,16,130,31,&num0_9_bw[numSize_bw*temp[3]]); 

#if 1                
                temp[0] = pwm_voltage*1000/PWM_PERIOD_VALUE;
                temp[1] = temp[0]/100;
                temp[2] = temp[0]/10%10;
                temp[3] = temp[0]%10;

                pictureDisplay(offset_x+16*8,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
                pictureDisplay(offset_x+16*9,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
                pictureDisplay(offset_x+16*10,16,130,31,&num0_9_bw[numSize_bw*10]);
                pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,130,31,&num0_9_bw[numSize_bw*temp[3]]);
#endif
                
            }
#else
            if(pwm_voltage_cali >= 5)//if(pwm_voltage_cali > 0)
            {
                pwm_voltage_cali -= 5;//pwm_voltage_cali--;
                offset_x=80;
                temp[0] = pwm_voltage_cali/1000;
                temp[1] = pwm_voltage_cali/100%10;
                temp[2] = pwm_voltage_cali/10%10;
                temp[3] = pwm_voltage_cali%10;

                pictureDisplay(offset_x+16*0,16,130,31,&num0_9_bw[numSize_bw*temp[0]]);
                pictureDisplay(offset_x+16*1,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
                pictureDisplay(offset_x+16*2,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
                pictureDisplay(offset_x+16*3,16,130,31,&num0_9_bw[numSize_bw*temp[3]]); 

#if 1                
                temp[0] = pwm_voltage_cali*1000/PWM_PERIOD_VALUE;
                temp[1] = temp[0]/100;
                temp[2] = temp[0]/10%10;
                temp[3] = temp[0]%10;

                pictureDisplay(offset_x+16*8,16,130,31,&num0_9_bw[numSize_bw*temp[1]]);
                pictureDisplay(offset_x+16*9,16,130,31,&num0_9_bw[numSize_bw*temp[2]]);
                pictureDisplay(offset_x+16*10,16,130,31,&num0_9_bw[numSize_bw*10]);
                pictureDisplay(offset_x+16*11-DOT_OFFSET_1,16,130,31,&num0_9_bw[numSize_bw*temp[3]]);
#endif
                
            }
            
#endif            
        }
    }else if(key_event == GUI_KEY_ADULT_CHILD_LONG)
    {
        MainWindowDisplayFunc();
        WIN = MainWindowDisplay;
        taskENTER_CRITICAL();
        //set_voltage_current(pwm_voltage,pwm_current);
        set_voltage_current(pwm_voltage_cali,pwm_current);
        taskEXIT_CRITICAL();
    }
    key_event = 0;

}

void display_DAP_set(void)
{
    u8 para[3] = {0,0,0};
    LCD_Clear(DarkGRAY);
    
    para[2] = current_exposoure_para.DAP /100;
    para[1] = current_exposoure_para.DAP/10%10;
    para[0] = current_exposoure_para.DAP%10;

    pictureDisplay(SLEEP_TIME_SETTING_X,DAPPA_SETTING_L,SLEEP_TIME_SETTING_Y+SYSTEM_OFFSET-4,DAPPA_SETTING_W,gImage_DAPPa_bw);

    pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[2]]);
    pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_bw[numSize_bw*10]);
    pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[1]]);
    pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W-1,&num0_9_bw[numSize_bw*para[0]]);
}
void calibration_DAP_process(void)
{
    
    u8 para[3] = {0,0,0};

    if(key_event == GUI_KEY_ADULT_CHILD_RELEASED)
    {
        
        if(WIN == MainWindowDisplay)
        {
            change_state(IDLE_STATE);
        }
    }else if(key_event == GUI_KEY_UP)
    {
        if(current_exposoure_para.DAP < MAX_DAP)
            current_exposoure_para.DAP ++;

        para[2] = current_exposoure_para.DAP /100;
        para[1] = current_exposoure_para.DAP/10%10;
        para[0] = current_exposoure_para.DAP%10;
            
        drawRec(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3+SYSINFO_NUM_L*4,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET+SYSINFO_NUM_W, NOVAGREEN);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*10]);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
             
    }else if(key_event == GUI_KEY_DOWN)
    {
        if(current_exposoure_para.DAP > MIN_DAP)
        {
            current_exposoure_para.DAP --;
        }
        para[2] = current_exposoure_para.DAP /100;
        para[1] = current_exposoure_para.DAP/10%10;
        para[0] = current_exposoure_para.DAP%10;
            
        drawRec(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3+SYSINFO_NUM_L*4,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET+SYSINFO_NUM_W, NOVAGREEN);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*3,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[2]]);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*4,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*10]);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*5-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[1]]);
        pictureDisplay(SLEEP_TIME_SETTING_X+DAPPA_SETTING_L+16*6-DOT_OFFSET_1,SYSINFO_NUM_L,SLEEP_TIME_SETTING_Y-4+SYSTEM_OFFSET,SYSINFO_NUM_W,&num0_9_yb[numSize_yb*para[0]]);
             

    }else if(key_event == GUI_KEY_ADULT_CHILD_LONG)
    {
        MainWindowDisplayFunc();
        WIN = MainWindowDisplay;
        taskENTER_CRITICAL();
        set_sys_exposoure_para(DAP,current_exposoure_para.DAP);
        taskEXIT_CRITICAL();
    }
    key_event = 0;

}


void display_mGycm2(void)
{
    uint16_t offset_x,offset_y;
    u16 expose_time = 0;
    u8 temp_data[5] = {0,0,0,0,0};
    
    offset_x=0;
    offset_y=15;

#if X_RAY_TYPE    

    expose_time = get_current_exposuretime();
    expose_time = expose_time*current_exposoure_para.DAP/100;
    temp_data[0] = expose_time/10000;
    temp_data[1] = expose_time/1000%10;
    temp_data[2] = expose_time/100%10;
    temp_data[3] = expose_time/10%10;
    temp_data[4] = expose_time%10;
    drawRec(0,245,0,4+49,DarkGRAY);

    if(temp_data[1] == 0)
    {
        //pictureDisplay(offset_x+16*0,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[1]]);
        pictureDisplay(offset_x+16*1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[2]]);
        pictureDisplay(offset_x+16*2,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[3]]);
        pictureDisplay(offset_x+16*3,16,offset_y,32,&num0_9_bw[numSize_bw*10]);    //
        pictureDisplay(offset_x+16*4-DOT_OFFSET_1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[4]]);

        pictureDisplay(97,MGYCM2_L,MGYCM2_Y,MGYCM2_W,gImage_mGycm2);    
    }else
    {
        //pictureDisplay(offset_x+16*0,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[0]]);
        pictureDisplay(offset_x+16*1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[1]]);
        pictureDisplay(offset_x+16*2,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[2]]);
        pictureDisplay(offset_x+16*3,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[3]]);
        pictureDisplay(offset_x+16*4,16,offset_y,32,&num0_9_bw[numSize_bw*10]);    //
        pictureDisplay(offset_x+16*5-DOT_OFFSET_1,16,offset_y,31,&num0_9_bw[numSize_bw*temp_data[4]]);
        
        pictureDisplay(113,MGYCM2_L,MGYCM2_Y,MGYCM2_W,gImage_mGycm2);    

    }
#endif  
}

GPIO_PinState GetTempretureState(void)
{
    return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_0);
}






