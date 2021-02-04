/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         userinterface.h
    Description:    This header contains global types and definitions
    =====================================================================================
*/
#ifndef __USERINTERFACE_H
#define __USERINTERFACE_H

#ifdef __cplusplus
 extern "C" {
#endif
     
#define DEBUG_ON 0     
     
#define GUI_KEY_ADULT_CHILD       8         
#define GUI_KEY_ANA_DIG           9         
#define GUI_KEY_UP                10
#define GUI_KEY_DOWN              13        
#define GUI_KEY_TOOTH_UP1         16
#define GUI_KEY_TOOTH_UP2         17
#define GUI_KEY_TOOTH_UP3         18
#define GUI_KEY_TOOTH_UP4         19
#define GUI_KEY_TOOTH_DOWN1       23
#define GUI_KEY_TOOTH_DOWN2       24
#define GUI_KEY_TOOTH_DOWN3       25
#define GUI_KEY_TOOTH_DOWN4       26
#define GUI_KEY_POWER             27        
#define GUI_KEY_EXPOSE_BUTTON     29
#define GUI_KEY_MULTIKEY1         30
#define GUI_KEY_MULTIKEY2         32
#define GUI_KEY_MULTIKEY3         33
#define GUI_KEY_MULTIKEY4         34
#define GUI_KEY_ADULT_CHILD_LONG  35
#define GUI_KEY_ANA_DIG_LONG      36
#define GUI_KEY_EXPOSE_BUTTON_RELEASED 37
#define GUI_KEY_POWER_OFF         38        
#define GUI_KEY_POWER_RELEASED    39
#define GUI_KEY_TOOTH_UP1_LONG    40
#define GUI_KEY_TOOTH_UP2_LONG    41
#define GUI_KEY_TOOTH_UP3_LONG    42
#define GUI_KEY_TOOTH_UP4_LONG    43
#define GUI_KEY_TOOTH_DOWN1_LONG  44
#define GUI_KEY_TOOTH_DOWN2_LONG  45
#define GUI_KEY_TOOTH_DOWN3_LONG  46
#define GUI_KEY_TOOTH_DOWN4_LONG  47
#define GUI_SLEEP_TIMER           48
#define GUI_KEY_ADULT_CHILD_RELEASED 49         
#define GUI_KEY_ANA_DIG_RELEASED     50
#define GUI_BATTERY_UPDATE           51
#define GUI_SETTING_TIME_OUT         52
#define GUI_BATTERY2LOW_TIME_OUT     53
#define GUI_KEY_UP_LONG              54
#define CALIBRATION_SETTING          55
#define CALIBRATION_DAP_SETTING      56
#define GUI_KEY_POWER_LONG           57  
//LED name
#define TOOTH_UP_1      1
#define TOOTH_UP_2      2
#define TOOTH_UP_3      3
#define TOOTH_UP_4      4
#define TOOTH_DOWN_1    5
#define TOOTH_DOWN_2    6
#define TOOTH_DOWN_3    7
#define TOOTH_DOWN_4    8
//LED state
#define LED_ON   1
#define LED_OFF  0

#define MGYCM2_X 116
#define MGYCM2_Y 15
#define MGYCM2_L 114
#define MGYCM2_W 36

#define DarkGRAY       0x10A2      //Éî»Ò
#define BackGRAY       0x528A      //ºÚ»Ò
#define LightGRAY      0x5AAA      //0xBDF7      //COLOR 3//0xDEDB      //color 2//0xF79E//color 1      //0x8c51      //0x8c51      //Ç³»Ò
#define GrassGREEN     0x84C0      //²ÝÂÌ
#define VerWhite       0xFFFF
#define NOVAGREEN      0xBE63 //0xDF30
#define LINECOLOR      0x5AAA
#define numSize_bw     992
#define numSize_by     1024
#define numSize_yb     1024
#define numSize_wb     380
#define CHILD_UP_X     10
#define CHILD_UP_Y     100
#define CHILD_UP_L     23
#define CHILD_UP_W     50

#define CHILD_DOWN_X   10
#define CHILD_DOWN_Y   100
#define CHILD_DOWN_L   23
#define CHILD_DOWN_W   30

#define ADULT_UP_X     55
#define ADULT_UP_Y     90
#define ADULT_UP_L     35
#define ADULT_UP_W     64

#define ADULT_DOWN_X   55
#define ADULT_DOWN_Y   90
#define ADULT_DOWN_L   35
#define ADULT_DOWN_W   45

#define FILM_UP_X     5//55
#define FILM_UP_Y     170
#define FILM_UP_L     33
#define FILM_UP_W     56

#define FILM_DOWN_X   5//55
#define FILM_DOWN_Y   170
#define FILM_DOWN_L   33
#define FILM_DOWN_W   37

#define SENSOR_UP_X     55//5
#define SENSOR_UP_Y     170
#define SENSOR_UP_L     37
#define SENSOR_UP_W     56

#define SENSOR_DOWN_X     55//5
#define SENSOR_DOWN_Y     170
#define SENSOR_DOWN_L     37
#define SENSOR_DOWN_W     37

#define BATTERY_X     244//240
#define BATTERY_Y     15
#define BATTERY_L     56
#define BATTERY_W     29

#define SYSTEM_MENU_X  100
#define SYSTEM_MENU_Y  15
#define SYSTEM_MENU_L  121
#define SYSTEM_MENU_W  31
#define SYSTEM_MENU_W_GREEN  32

#define EXPOSURE_CI_X  25
#define EXPOSURE_CI_Y  67
#define EXPOSURE_CI_L  121
#define EXPOSURE_CI_W  31

#define SYSINFO_NUM_X  180
#define SYSINFO_NUM_Y  132
#define SYSINFO_NUM_L  16
#define SYSINFO_NUM_W  32

#define DAPACC_NUM_X  75
#define DAPACC_NUM_Y  202
#define DAPACC_NUM_L  16
#define DAPACC_NUM_W  32

#define ARROW_X  70
#define ARROW_Y  79
#define ARROW_L  13
#define ARROW_W  22

#define FOCUS_X  0
#define FOCUS_Y  65
#define FOCUS_L  320
#define FOCUS_W  51

#define LINE_X  0
#define LINE_Y  55
#define LINE_L  320
#define LINE_W  2

#define SYSTEM_RESET_X  100
#define SYSTEM_RESET_Y  15
#define SYSTEM_RESET_L  121
#define SYSTEM_RESET_W  32

#define YES_X     144
#define YES_Y     89
#define YES_L     31
#define YES_W_by  32
#define YES_W_bw  31

#define NO_X     144
#define NO_Y     179
#define NO_L     31
#define NO_W_by  32
#define NO_W_bw  31

#define YES_FOCUS_X 0
#define YES_FOCUS_Y 59
#define YES_FOCUS_L 320
#define YES_FOCUS_W 88

#define SYSRESET_ARROW_X  114
#define SYSRESET_ARROW_Y  91
#define SYSRESET_ARROW_L  13
#define SYSRESET_ARROW_W  22

#define SYSTEM_PARA_SETTING_X  100
#define SYSTEM_PARA_SETTING_Y  10
#define SYSTEM_PARA_SETTING_L  121
#define SYSTEM_PARA_SETTING_W  31

#define SLEEP_TIME_SETTING_X  38
#define SLEEP_TIME_SETTING_Y  62
#define SLEEP_TIME_SETTING_L  121
#define SLEEP_TIME_SETTING_W  32
#define SYSTEM_OFFSET         48

#define DAPPA_SETTING_L       106
#define DAPPA_SETTING_W       31
#define DAPPA_SETTING_W_      32


#define DIGITAL_MODE_X        38
#define DIGITAL_MODE_Y        177
#define DIGITAL_MODE_L        181
#define DIGITAL_MODE_W        31

#define YAPIANBAO_X           223
#define YAPIANBAO_L           91
#define YAPIANBAO_W           31
#define YAPIANBAO_W_          32

#define S_LINE_X  0
#define S_LINE_Y  46
#define S_LINE_L  320
#define S_LINE_W  2

#define SYSPARA_ARROW_X  17
#define SYSPARA_ARROW_Y  62
#define SYSPARA_ARROW_L  13
#define SYSPARA_ARROW_W  22

#define SYSPARA_FOCUS_X 0
#define SYSPARA_FOCUS_Y 48
#define SYSPARA_FOCUS_L 320
#define SYSPARA_FOCUS_W 48

#define EVENT_RECORD_X  100
#define EVENT_RECORD_Y  10
#define EVENT_RECORD_L  121
#define EVENT_RECORD_W  32

#define EVENT_NUMBER_X  30
#define EVENT_NUMBER_Y  58
#define EVENT_NUMBER_L  16
#define EVENT_NUMBER_W  31

#define EVENT_X         72
#define EVENT_Y         58
#define EVENT_L         31
#define EVENT_W         31
#define EVENT_L_SLEEP   61
#define EVENT_W_SLEEP   32

#define CHARGING_ICON_X 305//301  
#define CHARGING_ICON_Y 15
#define CHARGING_ICON_L 14
#define CHARGING_ICON_W 29

//µç³ØµçÑ¹¼à²â
//³äµç
#define  ZERO_GRID_CHARGE      3266 //15v    0xCC2
#define  ONE_GRID_CHARGE       3375 //15.5v  0xD2F
#define  TWO_GRID_CHARGE       3484 //16.0v  0xD9C
#define  THREE_GRID_CHARGE     3593 //16.5V  0xE09
#define  CHARGE_FULL           3635 //16.7V3657 //16.8   0xE49
#define  CHARGE_FULL_MAX       3701 //17V 3668 //16.85V
#define  CHARGE_FULL_MAX_17    3701 //17V


//·Åµç
#define  ZERO_GRID             3222 //14.8v  0xC96  
#define  ONE_GRID              3330 //15.3v  0xD02
#define  TWO_GRID              3439 //15.8v  0xD6F
#define  THREE_GRID            3548 //16.3V  0xDDC

#define  ADAPTER_PLUG          0xD00

//charge state & normal state
#define BATTERY_NO_CHARGE          0
#define BATTERY_CHARGING           1
#define BATTERY_CHARGING_FULL      2
#define BATTERY_CHARGING_INIT      3

//battery warning
#define BACKGROUND_WARNING_X 33
#define BACKGROUND_WARNING_Y 55
#define BACKGROUND_WARNING_L 255
#define BACKGROUND_WARNING_W 150

#define BATTERY2LOW_X 45
#define BATTERY2LOW_Y 80
#define BATTERY2LOW_L 240
#define BATTERY2LOW_W 23

#define NEED2CHARGE_X 70
#define NEED2CHARGE_Y 138
#define NEED2CHARGE_L 211
#define NEED2CHARGE_W 35

#define E20_X         40
#define E20_Y         108
#define E20_L         240
#define E20_W         23

#define VER_X         105
#define VER_Y         193

#define VER_NUM_X     10
#define VER_NUM_Y     19
#define DOT_OFFSET_1  9
#define DOT_OFFSET_2  5


typedef enum
{
    MainWindowDisplay,
    SettingWindowDisplay,
    SleepWindowDisplay,
    DemoWindowDisplay,
    ErrorWindowDisplay,
    SystemSettingWin,
    SystemInforWin,
    EventInforWin,
    ParaSettingWin,
    SystemResetWin,
    CalibratinWin,
    WINDOWS_NO
} WINDOWS;

typedef enum
{
    IDLE_STATE,
    EXPOSURE_STATE,
    POWER_KEY_PRESSING_STATE,
    SLEEP_STATE,
    ERROR_STATE,
    DEMO_STATE,
    SYSTEM_SETTING_STATE,
    BATTERY2LOW_STATE,
    CALIBRATION_STATE,
    DEMO_EXPOSURE_STATE,
    DEMO_ERRO_STATE,
    CALIBRATION_DAP_STATE,
    
}SYSTEM_STATE;

typedef enum
{
    EXPROSURE_IDLE_STATE,
    PREHEAT_STATE,
    EXPOSURING_STATE,
    COLLING_STATE
}EXPOSURE_PROCESS;

typedef enum
{
    BUZZER_OFF = 0,
    BUZZER_ON,
}BUZZER_STATE;

typedef enum
{
    ZERO_GRID_DISPLAY = 0,
    ONE_GRID_DISPLAY,
    TWO_GRID_DISPLAY,
    THREE_GRID_DISPLAY,
    FOUR_GRID_DISPLAY,
    GRID_DISPLAY_NO
}BATTERY_STATE;

typedef enum
{
    BLUE_LED,
    GREEN_LED,
    ORANGE_LED
}LED_COLOR;
extern WINDOWS WIN;
extern u16 power_key_press_duration ;
extern u8 power_key_is_pressing;
extern u16 setting_timeout;
extern u16 battery2low_timeout;
extern u16 pwm_voltage_cali;
extern u16 pwm_voltage;
extern u16 pwm_current;
extern u32 exposure_interval_timeout;
extern u8 exposure_over_wait;

#define SYSTEM_POWER_OFF HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET)
#define POWER_OFF_DURATION    2000
#define SETTING_TIMEOUT_DUARTION 30000 //30s
#define BATTERY2LOW_POWEROFF     5000  //5s

#define MAX_EXPOSE_COUNT 99999

extern SYSTEM_STATE system_state;
extern EXPOSURE_PROCESS exposuring_state;
void UserInterface(void);
void powerOn_displayFunc(void);
void MainWindowDisplayFunc(void);
void Buzzer_Control(BUZZER_STATE state);
void idle_state_process(void);
void change_state(SYSTEM_STATE state);
void exposure_state_process(void);
void change_exposuring_state(EXPOSURE_PROCESS state);
void LED_Control(u8 led, u8 state);
void LED_init(u8 state);
void sleep_state_process(void);
void power_key_pressing_state_process(void);
void displayExpDly(void);
void Buzzer_Key(void);
void set_sleep_timer(void);
u32 get_sleep_timer(void);
void demo_state_process(void);
void system_setting_state_process(void);
void display_battery_status(u8 grid);
void display_system_setting_win(u8 item);
void display_system_infor_win(void);
void display_system_reset_win(u8 focus);
void display_system_para_setting(u8 focus);
void display_system_para_setting_maxRay(u8 focus);
void display_event_record(void);
void system_power_off(void);
void display_event_record_each(u16 x, u16 y, u8 event);
BATTERY_STATE update_battery_status(void);
void display_battery_2_low(void);
void battery2low_state_process(void);
void LED_control(LED_COLOR color);
void displayExpDly_Demo(void);
void Demo_Para_Init(void);
void powerOff_displayFunc(void);
void display_system_para_setting_arrow(u8 focus);
void display_system_para_setting_arrow_maxRay(u8 focus);
void set_system_para(u8 focus);
void set_system_para_maxRay(u8 focus);
void set_setting_timeout(void);
u16 get_setting_timeout(void);
void set_battery2low_timeout(void);
u16 get_battery2low_timeout(void);
void error_state_process(void);
u16 get_current_exposuretime(void);
void display_pwm_set(void);
void calibration_state_process(void);
void display_mGycm2(void);
void demo_exposure_state_process(void);
u16 get_demo_exposuretime(void);
void change_demo_exposuring_state(EXPOSURE_PROCESS state);
void demo_error_state_process(void);
void calibration_DAP_process(void);
void display_DAP_set(void);
void display_system_para_setting_arrow_dispear(u8 focus);
void display_system_para_setting_arrow_maxRay_dispear(u8 focus);
#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_FLASH_H */
