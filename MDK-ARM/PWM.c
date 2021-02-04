/*
 =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         PWM.c
    Description:    PA1, PA2 is set as pwm output using TIM2
                    PA1-------CHANNEL 2  kv
                    PA2-------CHANNEL 3  current
                    GPIO_AF1_TIM2
 =====================================================================================
*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_tim.h"
#include "types.h"
#include "keys.h"
#include "delay.h"
#include "PWM.h"



#define INCREMENT   (5)

struct PwmControl {
    uint16_t target;
    uint16_t current;
    uint32_t channel;
    TIM_OC_InitTypeDef  sConfig;
} PwmControl;

struct PwmControl sCtrls[PWM_NUM_OF_CHANNELS];
static TIM_HandleTypeDef sGeneratorTimHandle;

uint16_t sInput2Period(float input)
{
    uint16_t pwmVal = 0;
    if (0.f < input){
        if (1.f > input){
            pwmVal = (uint16_t)(input * PWM_PERIOD_VALUE);
        }else{
            pwmVal = PWM_PERIOD_VALUE;
        }
    }
    return pwmVal;
}

float sPeriod2Input(uint16_t pwmVal)
{
    float input = (float)pwmVal / (float)PWM_PERIOD_VALUE;
    return input;
}

void sUpdateChannelVal(struct PwmControl *ctrl)
{
    int err = (int)ctrl->target - (int)ctrl->current;
    if (INCREMENT < err){
        ctrl->current = (uint16_t)(ctrl->current + INCREMENT);
    }else if(-INCREMENT > err){
        ctrl->current = (uint16_t)(ctrl->current - INCREMENT);
    }else{
        ctrl->current = (uint16_t)(ctrl->current + err);
    }        
}

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    /*Configure GPIO pins : PA1 PA2 */
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    __TIM2_CLK_ENABLE();
        /* Set TIMx instances */
    sGeneratorTimHandle.Instance = TIM2;
    sGeneratorTimHandle.Init.Period = (uint16_t)PWM_PERIOD_VALUE;
    sGeneratorTimHandle.Init.Prescaler = (uint16_t)0;
    sGeneratorTimHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    sGeneratorTimHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
    HAL_TIM_PWM_Init(&sGeneratorTimHandle);
    
    sCtrls[0].channel = TIM_CHANNEL_2; // kV
    sCtrls[1].channel = TIM_CHANNEL_3; // mA
    
    sCtrls[1].sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;
    sCtrls[0].sConfig.OCPolarity = TIM_OCPOLARITY_HIGH;//TIM_OCPOLARITY_LOW;
    
    for (int i = 0; i < PWM_NUM_OF_CHANNELS; i++) {
        sCtrls[i].target = 0;
        sCtrls[i].current = 0;
        sCtrls[i].sConfig.OCMode = TIM_OCMODE_PWM1;
        sCtrls[i].sConfig.Pulse = 0;
        sCtrls[i].sConfig.OCFastMode = TIM_OCFAST_DISABLE;
                    
        HAL_TIM_PWM_ConfigChannel(&sGeneratorTimHandle,&sCtrls[i].sConfig,sCtrls[i].channel);
    }
    
    for (int i = 0; i < PWM_NUM_OF_CHANNELS; i++)
    {
        HAL_TIM_PWM_Start(&sGeneratorTimHandle, sCtrls[i].channel); 
    }

}

void pwmSet(uint32_t channel, float output)
{
    uint16_t pwmVal = output;//sInput2Period(output);
    pwmTargetSet(channel, output);

    switch(channel)
    {
    case PWM_KV_REF:    
        //eHalTimSetCompare(&sGeneratorTimHandle,
        //                  sCtrls[PWM_KV_REF].channel, pwmVal);
        __HAL_TIM_SetCompare(&sGeneratorTimHandle,
                          sCtrls[PWM_KV_REF].channel, pwmVal);
        sCtrls[PWM_KV_REF].current = pwmVal;
        break;
    case PWM_MA_REF:   
        __HAL_TIM_SetCompare(&sGeneratorTimHandle,
                          sCtrls[PWM_MA_REF].channel, pwmVal);
        sCtrls[PWM_MA_REF].current = pwmVal;
        break;            
    default:
        break;
    }
}

void pwmUpdate(void)
{
    for(int i = 0; i < PWM_NUM_OF_CHANNELS; i++){
        sUpdateChannelVal(&sCtrls[i]);
    }

    for(int i = 0; i < PWM_NUM_OF_CHANNELS; i++){
        __HAL_TIM_SetCompare(&sGeneratorTimHandle,
                          sCtrls[i].channel, sCtrls[i].current);
    }
}

void pwmTargetSet(uint32_t channel, float target)
{
    uint16_t pwmVal = target;//sInput2Period(target);
    if (PWM_NUM_OF_CHANNELS > channel){
        sCtrls[channel].target = pwmVal;
    }
}

float pwmGet(uint32_t channel)
{
    uint16_t pwmVal = 0;
    if (PWM_NUM_OF_CHANNELS > channel){
        pwmVal = sCtrls[channel].current;
    }
    return sPeriod2Input(pwmVal);
}


