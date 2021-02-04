/*
 =====================================================================================
    Processor:      STM32F405VG
    Author:         Leo Liu
    Date:           04/2020
    Module:         keys.c
    Description:    the keys module reads and deglitches the input of the keys and
                    returns the status of each key.
                    State machine is called periodically in a 10ms cycle
 =====================================================================================
*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "types.h"
#include "keys.h"
#include "delay.h"

typedef enum
{
    INIT,
    IDLE,
    ON,
    OFF
}   KEY_SM_STATES;

KEYS_STRUCT keys;

/*
    =====================================================================================
    Name:           Keys_HasAnyKeyChanged
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      -
    Return value:   BOOL (True == Any key status has changed
    Description:    This function checks if any key change flag is set.
    =====================================================================================
*/
BOOL Keys_HasAnyKeyChanged(void)
{
    KEYS i;
    BOOL change = FALSE;

    for (i = ADULT_CHILD; i <= KEY_POWER; i++)
    {
        if (keys.key[i].changed == TRUE)
        {
            change = TRUE;
        }
    }

    return change;
}

/*
    =====================================================================================
    Name:           Keys_IsPressed
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if specified key was pressed.
    =====================================================================================
*/
BOOL Keys_IsPressed(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].pressed == TRUE)
    {
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsPressedClear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if specified key was pressed. According flag
                    is cleard.
    =====================================================================================
*/
BOOL Keys_IsPressedClear(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].pressed == TRUE)
    {
        keys.key[key].pressed = FALSE;
        keys.key[key].changed = FALSE;
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsReleased
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if specified key was released.
    =====================================================================================
*/
BOOL Keys_IsReleased(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].released == TRUE)
    {
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsReleasedCLear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if specified key was released. Flag is reset
                    after read.
    =====================================================================================
*/
BOOL Keys_IsReleasedClear(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].released == TRUE)
    {
        keys.key[key].released = FALSE;
        keys.key[key].changed = FALSE;
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsOn
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if the specified key is on.
    =====================================================================================
*/
BOOL Keys_IsOn(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].on == TRUE)
    {
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsOnClear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if the specified key is on. Flag is cleared
                    after read.
    =====================================================================================
*/
BOOL Keys_IsOnClear(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].on == TRUE)
    {
        keys.key[key].on = FALSE;
        keys.key[key].changed = FALSE;
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsChanged
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if the specified key has changed flags
    =====================================================================================
*/
BOOL Keys_IsChanged(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].changed == TRUE)
    {
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsChangedClear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if the specified key has changed flags. Flag
                    is cleard
    =====================================================================================
*/
BOOL Keys_IsChangedClear(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].changed == TRUE)
    {
        keys.key[key].changed = FALSE;
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsPressedLong
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if specified key is pressed long.
    =====================================================================================
*/
BOOL Keys_IsPressedLong(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].pressed_long == TRUE)
    {
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_IsPressedLongClear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key // which key is to be tested
    Return value:   BOOL // True == key status is true
    Description:    This function checks if specified key is pressed long. Flag is cleard
                    after read.
    =====================================================================================
*/
BOOL Keys_IsPressedLongClear(KEYS key)
{
    BOOL result = FALSE;

    if (keys.key[key].pressed_long == TRUE)
    {
        keys.key[key].pressed_long = FALSE;
        keys.key[key].changed = FALSE;
        result = TRUE;
    }

    return result;
}

/*
    =====================================================================================
    Name:           Keys_Init
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      -
    Return value:   -
    Description:    This function inits IOs, variables an flags for module
                    This function is calls once while system initialization.
    =====================================================================================
*/
void Keys_Init(void)
{
    KEYS i;
    Keys_Portinit();

    for (i = ADULT_CHILD; i <= KEY_POWER; i++)
    {
        keys.key[i].dglCnt = 0;
        keys.key[i].longCnt = 0;
        keys.key[i].changed = FALSE;
        keys.key[i].pressed = FALSE;
        keys.key[i].released = FALSE;
        keys.key[i].on = FALSE;
        keys.key[i].pressed_long = FALSE;
        keys.key[i].smState = INIT;
    }
}

/*
    =====================================================================================
    Name:           Keys_Scan
    Author:         Johannes Sauter
    Date:           08/2012
    Parameter:      -
    Return value:   -
    Description:    This function reads the input pins of the key and calls the
                    state machine to check the transisions and set the according flags.
                    This function is called cyclic from a higher sysmte level
    =====================================================================================
*/
void Keys_Scan(void)
{
    KEYS i;
    Keys_GetPins();

//    for (i = ADULT_CHILD; i <= EXPOSE_BUTTON; i++)
    for (i = ADULT_CHILD; i <= KEY_POWER; i++)
    {
        Keys_Statemachine(i);
    }
}
//u8 leo_test = 0;
void Keys_GetPins(void)
{
    u8 i;

    for(i=0;i<4;i++)
    {
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);//OUTPUT LOW
        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9>>i, GPIO_PIN_SET);//OUTPUT High
        
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)) //K2 ADULT_CHILD
        {
            keys.key[ADULT_CHILD+i].pin = TRUE;
        }else
        {
            keys.key[ADULT_CHILD+i].pin = FALSE;
        }

        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_9)) //K6 TOOTH_UP1
        {
            keys.key[TOOTH_UP1+i].pin = TRUE;
        }else
        {
            keys.key[TOOTH_UP1+i].pin = FALSE;
        }

        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)) //K10 TOOTH_DOWN1
        {
            keys.key[TOOTH_DOWN1+i].pin = TRUE;
        }else
        {
            keys.key[TOOTH_DOWN1+i].pin = FALSE;
        }    
    }
    //read Expose key
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3)) //K10 TOOTH_DOWN1
    {
        keys.key[EXPOSE_BUTTON].pin = FALSE;
        //leo_test = 5;
    }else
    {
        keys.key[EXPOSE_BUTTON].pin = TRUE;
        //leo_test = 4;
    }    
    //read Power Key
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_11)) //K10 TOOTH_DOWN1
    {
        keys.key[KEY_POWER].pin = FALSE;
    }else
    {
        keys.key[KEY_POWER].pin = TRUE;
    }           
}

/*
    =====================================================================================
    Name:           Keys_Statemachine
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key
    Return value:   -
    Description:    This function scans the inputs of the actual key.
                    Depending on the status change of the key different flags for each
                    action will be set
                    Key flags: CHANGE, PRESSED, RELEASED, IS_PRESSED, LONG_PRESSED
    =====================================================================================
*/
void Keys_Statemachine(KEYS actualKey)
{
    switch (keys.key[actualKey].smState)
    {
        default:
        case INIT:
            keys.key[actualKey].smState = IDLE;
            break;

        case IDLE:
            if (keys.key[actualKey].pin == TRUE)
            {
                keys.key[actualKey].dglCnt++;    // Key pressed
            }
            else
            {
                keys.key[actualKey].dglCnt = 0;
            }

            if (keys.key[actualKey].dglCnt > DGL_CNT)
            {
                keys.key[actualKey].changed = TRUE;
                keys.key[actualKey].pressed = TRUE;
                keys.key[actualKey].on = TRUE;
                keys.key[actualKey].longCnt = 0;
                keys.key[actualKey].dglCnt = 0;
                keys.key[actualKey].smState = ON;
            }

            break;

        case ON:
            if (keys.key[actualKey].longCnt < TIME_LONGKEY)
            {
                keys.key[actualKey].longCnt++;

                if (keys.key[actualKey].longCnt == TIME_LONGKEY)
                {
                    keys.key[actualKey].pressed_long = TRUE;
                    keys.key[actualKey].changed = TRUE;
                }
            }

            if (keys.key[actualKey].pin == FALSE)
            {
                keys.key[actualKey].dglCnt++;    // Key released
            }
            else
            {
                keys.key[actualKey].dglCnt = 0;
            }

            if (keys.key[actualKey].dglCnt > DGL_CNT)
            {
                keys.key[actualKey].changed = TRUE;
                keys.key[actualKey].released = TRUE;
                keys.key[actualKey].on = FALSE;
                keys.key[actualKey].longCnt = 0;
                keys.key[actualKey].dglCnt = 0;
                keys.key[actualKey].smState = IDLE;
            }

            break;
    }
}

/*
    =====================================================================================
    Name:           Keys_Clear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      
    Return value:   -
    Description:    This function clears the status flag of all keys
    =====================================================================================
*/
void Keys_ClearAll(void)
{
    KEYS i;

    for (i = ADULT_CHILD; i <= EXPOSE_BUTTON; i++)
    {
        keys.key[i].changed = FALSE;
        keys.key[i].pressed = FALSE;
        keys.key[i].released = FALSE;
        keys.key[i].on = FALSE;
        keys.key[i].pressed_long = FALSE;
    }
}

/*
    =====================================================================================
    Name:           Keys_Clear
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      key
    Return value:   -
    Description:    This function clears the status flag of all keys
    =====================================================================================
*/
void Keys_ClearKey(KEYS i)
{
    keys.key[i].changed = FALSE;
    keys.key[i].pressed = FALSE;
    keys.key[i].released = FALSE;
    keys.key[i].on = FALSE;
    keys.key[i].pressed_long = FALSE;
}

/*
    =====================================================================================
    Name:           Keys_Portinit
    Author:         Leo Liu
    Date:           04/2020
    Parameter:      -
    Return value:   -
    Description:    This function sets the direction of IOs used in this module
    =====================================================================================
*/
void Keys_Portinit(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
   //key matrix£ºROW
    /*Configure GPIO pins : PC6 PC7 PC8 PC9 */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);//OUTPUT LOW

    //key matrix: column
    /*Configure GPIO pins : PA8 PA9 PA10 */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
     /*Configure GPIO pins : PA3(Expose Key) PA11(Power Key) */
    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;//GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
}


