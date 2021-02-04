/*
    =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         userinterface.c
    Description:    This header contains global types and definitions
    =====================================================================================
*/
#include "pwm_timer.h"
#include "userinterface.h"
#include "24cxx.h"
#include "keys.h"

u32 sleep_timer = 0;
u32 preheat_timer = 0;
u32 exposuring_timer = 0;
u32 colling_timer = 0;
u16 breath_led_timer = 0;
u32 sleep2off_timer = 0;


#if 0
osTimerId_t myTimer01Handle;
osStaticTimerDef_t myTimer01ControlBlock;
const osTimerAttr_t myTimer01_attributes = {
  .name = "myTimer01",
  .cb_mem = &myTimer01ControlBlock,
  .cb_size = sizeof(myTimer01ControlBlock),
};
#endif

/* Definitions for myTimer02 */
osTimerId_t myTimer02Handle;
osStaticTimerDef_t myTimer02ControlBlock;
const osTimerAttr_t myTimer02_attributes = {
  .name = "myTimer02",
  .cb_mem = &myTimer02ControlBlock,
  .cb_size = sizeof(myTimer02ControlBlock),
};

#if 0
void Callback01(void *argument)
{
  /* USER CODE BEGIN Callback01 */
    Buzzer_Control(BUZZER_OFF);
  /* USER CODE END Callback01 */
}
#endif


void Callback02(void *argument)
{
 
    /* USER CODE BEGIN Callback02 */
 
#if 0   //test period prcision
  static u8 test_flag = 0;
  if(test_flag)
  {
      SDA_1;
      test_flag = 0;
  }else
  {
      SDA_0;
      test_flag = 1;
  }
#endif 
    if(preheat_timer > 0)
      preheat_timer--;
  
    if((sleep_timer > 0) && (exposure_over_wait == 0))
      sleep_timer--;

    if(exposuring_timer > 0)
      exposuring_timer--;
    
    if(colling_timer > 0)
      colling_timer--;
    
    if(power_key_is_pressing == KEY_PRESSED)
    {
        power_key_press_duration++;
        //if(power_key_press_duration > POWER_OFF_DURATION)
        //    SYSTEM_POWER_OFF;
    }
    if(setting_timeout > 0)
        setting_timeout--;
    
    if(battery2low_timeout > 0)
        battery2low_timeout--;
    
    if(exposure_interval_timeout > 0)
        exposure_interval_timeout--;
    
    if(breath_led_timer > 0)
        breath_led_timer--;
    
    if(sleep2off_timer > 0)
        sleep2off_timer--;
  
  /* USER CODE END Callback02 */
}
